"""Signed fixed-point residue values."""

from __future__ import annotations

from math import prod
from typing import TextIO
import sys

from .errors import critical_error
from .mrn import MRN
from .modtable import RNSNumberSystem
from .ppm import _SUPPORTED_RADICES, _format_unsigned, PPM
from .sppm import NEGATIVE, POSITIVE, SIGN_VALID, SPPM


def _parse_signed_decimal_fixed_text(value: str) -> tuple[int, int, int]:
    """Return ``(sign, numerator, denominator)`` for decimal fixed-point text."""

    if not isinstance(value, str):
        raise TypeError("decimal fixed-point value must be a string")
    text = value.strip().replace("_", "")
    if not text:
        raise ValueError("decimal fixed-point assignment requires at least one digit")

    sign = 1
    if text[0] == "+":
        text = text[1:]
    elif text[0] == "-":
        sign = -1
        text = text[1:]
    if not text:
        raise ValueError("decimal fixed-point assignment requires at least one digit")

    if text.count(".") > 1:
        raise ValueError("decimal fixed-point assignment allows at most one decimal point")
    whole_text, _, fractional_text = text.partition(".")
    digits_text = f"{whole_text}{fractional_text}"
    if not digits_text:
        raise ValueError("decimal fixed-point assignment requires at least one digit")

    numerator = 0
    for character in digits_text:
        if not "0" <= character <= "9":
            raise ValueError(f"invalid decimal digit {character!r}")
        numerator = numerator * 10 + (ord(character) - ord("0"))
    denominator = 10 ** len(fractional_text)
    return sign, numerator, denominator


def _external_integer(value: int | str) -> int:
    if type(value) is int:
        return value
    if isinstance(value, str):
        text = value.strip().replace("_", "")
        if not text:
            raise ValueError("integer text requires at least one digit")
        sign = 1
        if text[0] == "+":
            text = text[1:]
        elif text[0] == "-":
            sign = -1
            text = text[1:]
        if text.lower().startswith("0x") or text.lower().startswith("0b"):
            return sign * int(text, 0)
        result = 0
        for character in text:
            if not "0" <= character <= "9":
                raise ValueError(f"invalid decimal digit {character!r}")
            result = result * 10 + (ord(character) - ord("0"))
        return sign * result
    raise TypeError("value must be an int or numeric string")


class SPMF(SPPM):
    """Signed fixed-point interpretation layered on top of ``SPPM``.

    This first slice establishes the class format only. Fractional arithmetic,
    decimal assignment, ratio assignment, rounding, and fixed-point printing
    remain future conversion slices.
    """

    def __init__(
        self,
        value: "SPMF | SPPM | PPM | int | str" = 0,
        *,
        system: RNSNumberSystem | None = None,
    ) -> None:
        if isinstance(value, PPM) and system is None:
            system = value.system
        if not isinstance(system, RNSNumberSystem):
            raise TypeError("system must be an RNSNumberSystem")
        self.num_fract_digits = system.fractional_digits
        self.normal_fract_digits = system.fractional_digits
        self._ensure_fractional_geometry(system)
        super().__init__(value, system=system)

    @staticmethod
    def _ensure_fractional_geometry(system: RNSNumberSystem) -> None:
        if system.fractional_digits <= 0:
            raise ValueError("SPMF requires at least one fractional value digit")
        if system.fractional_digits > len(system.value_indices):
            raise ValueError("fractional digit count cannot exceed value digit count")
        expected = tuple(range(system.fractional_digits))
        if system.fractional_indices != expected:
            raise ValueError(
                "SPMF fractional digits must occupy the least-significant value indexes"
            )

    @property
    def fractional_indices(self) -> tuple[int, ...]:
        """RNS digit indices belonging to the current fractional range."""

        return self.system.value_indices[: self.num_fract_digits]

    @property
    def whole_indices(self) -> tuple[int, ...]:
        """RNS digit indices belonging to the current whole/integer range."""

        return self.system.value_indices[self.num_fract_digits :]

    @property
    def normal_fractional_indices(self) -> tuple[int, ...]:
        """RNS digit indices belonging to the normalized fractional range."""

        return self.system.fractional_indices

    @property
    def normal_whole_indices(self) -> tuple[int, ...]:
        """RNS digit indices belonging to the normalized whole/integer range."""

        return self.system.whole_indices

    @property
    def unit_range(self) -> int:
        """Product of effective moduli in the current fractional range."""

        return prod(self._current_digit_modulus(index) for index in self.fractional_indices)

    @property
    def normal_unit_range(self) -> int:
        """Product of full moduli in the normalized fractional range."""

        return self.system.unit_range

    @property
    def fractional_moduli(self) -> tuple[int, ...]:
        """Effective moduli in the current fractional range."""

        return tuple(self._current_digit_modulus(index) for index in self.fractional_indices)

    @property
    def whole_moduli(self) -> tuple[int, ...]:
        """Effective moduli in the current whole/integer range."""

        return tuple(self._current_digit_modulus(index) for index in self.whole_indices)

    def _current_digit_modulus(self, index: int) -> int:
        digit = self.rn[index]
        if digit.skip:
            critical_error(
                "SPMF cannot use skipped digit in fixed-point range metadata",
                digit_index=index,
                current_fraction_digits=self.num_fract_digits,
                normal_fraction_digits=self.normal_fract_digits,
                value_native=self.format_native(),
            )
        return digit.current_modulus

    @property
    def sliding_point_active(self) -> bool:
        """Whether the current fraction point differs from the normal position."""

        return self.num_fract_digits != self.normal_fract_digits

    def _validate_fraction_position(self, fractional_digits: int) -> None:
        if type(fractional_digits) is not int:
            raise TypeError("fractional digit count must be an integer")
        if not 0 <= fractional_digits <= len(self.system.value_indices):
            raise ValueError(
                "fractional digit count must be between zero and the number of value digits"
            )

    def set_fraction_position(self, fractional_digits: int) -> None:
        """Set the current sliding-point position without scaling residues.

        This is first-pass plumbing for internal algorithms. It changes the
        interpretation metadata only; future scaling routines must perform the
        corresponding RNS operations when converting between fixed-point
        positions.
        """

        self._validate_fraction_position(fractional_digits)
        self.num_fract_digits = fractional_digits

    def reset_fraction_position(self) -> None:
        """Restore the current fraction point to the normalized position."""

        self.num_fract_digits = self.normal_fract_digits

    def has_same_fraction_position(self, other: object) -> bool:
        """Return whether another SPMF has the same current fraction position."""

        return (
            isinstance(other, SPMF)
            and self.system.is_compatible(other.system)
            and self.num_fract_digits == other.num_fract_digits
            and self.normal_fract_digits == other.normal_fract_digits
        )

    def ensure_fraction_position_compatible(self, other: "SPMF") -> None:
        """Raise a critical error if two fixed-point values are not aligned."""

        if not isinstance(other, SPMF):
            raise TypeError("fraction-position compatibility requires an SPMF value")
        if not self.has_same_fraction_position(other):
            critical_error(
                "SPMF fraction positions are incompatible",
                left_current_fraction_digits=self.num_fract_digits,
                left_normal_fraction_digits=self.normal_fract_digits,
                right_current_fraction_digits=other.num_fract_digits,
                right_normal_fraction_digits=other.normal_fract_digits,
                left_system=self.system.name,
                right_system=other.system.name,
            )

    def ensure_normal_fraction_position(self) -> None:
        """Raise a critical error if the value is not in normal fixed-point format."""

        if self.sliding_point_active:
            critical_error(
                "SPMF value is not in normal fraction position",
                current_fraction_digits=self.num_fract_digits,
                normal_fraction_digits=self.normal_fract_digits,
                system=self.system.name,
            )

    def assign(self, value: "SPMF | SPPM | PPM | int | str") -> None:
        super().assign(value)
        if isinstance(value, SPMF):
            self.num_fract_digits = value.num_fract_digits
            self.normal_fract_digits = value.normal_fract_digits
        else:
            self.reset_fraction_position()

    def _assign_scaled_integer_preserving_fraction_position(self, scaled_integer: int) -> None:
        SPPM.assign(self, scaled_integer)

    def assign_scaled_integer(self, value: int | str) -> None:
        """Assign an already-scaled fixed-point integer in normal format."""

        self.reset_fraction_position()
        self._assign_scaled_integer_preserving_fraction_position(_external_integer(value))

    def assign_whole(self, value: int | str) -> None:
        """Assign a whole integer by scaling it by the normal unit range."""

        self.reset_fraction_position()
        self._assign_scaled_integer_preserving_fraction_position(
            _external_integer(value) * self.unit_range
        )

    def assign_ratio(self, numerator: int | str, denominator: int | str) -> None:
        """Assign ``numerator / denominator`` using preliminary trap-door I/O."""

        self.reset_fraction_position()
        numerator_value = _external_integer(numerator)
        denominator_value = _external_integer(denominator)
        if denominator_value == 0:
            raise ZeroDivisionError("fixed-point ratio denominator cannot be zero")

        sign = -1 if numerator_value * denominator_value < 0 else 1
        scaled_abs, remainder = divmod(
            abs(numerator_value) * self.unit_range,
            abs(denominator_value),
        )
        if remainder * 2 > abs(denominator_value):
            scaled_abs += 1
        self._assign_scaled_integer_preserving_fraction_position(sign * scaled_abs)

    def assign_decimal(self, value: str) -> None:
        """Assign decimal text using preliminary trap-door I/O."""

        sign, numerator, denominator = _parse_signed_decimal_fixed_text(value)
        self.assign_ratio(sign * numerator, denominator)

    def unit_ppm(self) -> PPM:
        """Return a PPM representing ``1.0`` for the current fraction position."""

        unit = PPM(0, system=self.system)
        unit.assign_pm(1, format_source=self)
        for index in self.fractional_indices:
            unit.mult(self._current_digit_modulus(index))
        return unit

    def unit_value(self) -> "SPMF":
        """Return an SPMF representing ``1.0`` for the current fraction position."""

        unit = SPMF(0, system=self.system)
        unit.assign(self)
        SPPM.assign(unit, self.unit_ppm())
        unit.num_fract_digits = self.num_fract_digits
        unit.normal_fract_digits = self.normal_fract_digits
        return unit

    def assign_unit(self) -> None:
        """Assign the fixed-point value ``1.0`` for the current fraction position."""

        SPPM.assign(self, self.unit_ppm())

    def copy(self) -> "SPMF":
        result = SPMF(0, system=self.system)
        result.assign(self)
        return result

    def _aligned_zero(self) -> "SPMF":
        result = SPMF(0, system=self.system)
        PPM.assign_pm(result, 0, format_source=self)
        result.num_fract_digits = self.num_fract_digits
        result.normal_fract_digits = self.normal_fract_digits
        result.sign_flag = POSITIVE
        result.sign_valid = SIGN_VALID
        return result

    def _assign_signed_integer_in_aligned_format(self, target: "SPMF", value: int) -> None:
        if value < 0:
            PPM.assign_pm(target, -value, format_source=self)
            PPM.complement(target)
            target.sign_flag = NEGATIVE
        else:
            PPM.assign_pm(target, value, format_source=self)
            target.sign_flag = POSITIVE
        target.sign_valid = SIGN_VALID
        target._zero_clean()

    def _coerce_integer_operand(self, other: object) -> "SPMF":
        """Return an integer operand in this value's effective RNS format."""

        if isinstance(other, SPMF):
            raise TypeError("integer SPMF operand must not already be fractional")
        result = self._aligned_zero()
        if isinstance(other, SPPM):
            self._ensure_format_compatible(other)
            PPM.assign(result, other)
            result.sign_flag = other.sign_flag
            result.sign_valid = other.sign_valid
            result._zero_clean()
            return result
        if type(other) is int or isinstance(other, str):
            self._assign_signed_integer_in_aligned_format(result, _external_integer(other))
            return result
        raise TypeError("SPMF integer operand must be int, numeric string, or SPPM")

    def _coerce_same_fraction_operand(self, other: object) -> "SPMF":
        if not isinstance(other, SPMF):
            raise TypeError("SPMF arithmetic currently requires another SPMF operand")
        self._ensure_format_compatible(other)
        self.ensure_fraction_position_compatible(other)
        return other

    def add_whole(self, other: object) -> None:
        """Add a whole integer to this fixed-point value.

        This mirrors C++ ``SPMF::Add1``: form an aligned helper equal to
        ``other * unit_range`` and then use ordinary same-format fractional
        addition.
        """

        whole = self._coerce_integer_operand(other)
        PPM.mult(whole, self.unit_ppm())
        self.add(whole)

    def sub_whole(self, other: object) -> None:
        """Subtract a whole integer from this fixed-point value."""

        whole = self._coerce_integer_operand(other)
        PPM.mult(whole, self.unit_ppm())
        self.sub(whole)

    def add1(self, other: object) -> None:
        """Compatibility alias for C++ ``SPMF::Add1``."""

        self.add_whole(other)

    def sub1(self, other: object) -> None:
        """Compatibility alias for C++ ``SPMF::Sub1``."""

        self.sub_whole(other)

    def add(self, other: object) -> None:
        """Add another SPMF, or add an integer as whole units."""

        if isinstance(other, SPMF):
            other = self._coerce_same_fraction_operand(other)
            SPPM.add(self, other)
            return
        self.add_whole(other)

    def sub(self, other: object) -> None:
        """Subtract another SPMF, or subtract an integer as whole units."""

        if isinstance(other, SPMF):
            other = self._coerce_same_fraction_operand(other)
            SPPM.sub(self, other)
            return
        self.sub_whole(other)

    def compare(self, other: object) -> int:
        """Compare another same-format SPMF by delegating to SPPM comparison."""

        other = self._coerce_same_fraction_operand(other)
        return SPPM.compare(self, other)

    def _fractional_half_range(self) -> PPM:
        if self.unit_range % 2:
            critical_error(
                "SPMF fractional multiply rounding requires an even fractional range",
                unit_range=self.unit_range,
                fractional_moduli=self.fractional_moduli,
                system=self.system.name,
            )
        half_range = PPM(0, system=self.system)
        half_range.assign_pm(1, format_source=self)
        for index in self.fractional_indices:
            half_range.mult(self._current_digit_modulus(index))
        half_range.mod_div(2)
        half_range.extend_to_current_power()
        return half_range

    def _whole_range_correction_constant(self) -> PPM:
        correction = PPM(0, system=self.system)
        correction.assign_pm(1, format_source=self)
        for index in self.whole_indices:
            correction.mult(self._current_digit_modulus(index))
        return correction

    def _assign_fractional_mr_shift_right(self, value: PPM) -> None:
        shifted = MRN.from_ppm(value).shifted_right(self.num_fract_digits)
        reconstructed = value._reconstruct_mixed_radix_to_current_power(shifted)
        PPM.assign(self, reconstructed)

    def mult_std(self, other: object) -> None:
        """Multiply by another same-format SPMF using the C++ ``Mult4b`` path.

        The method performs the raw product in residue form, scales by
        discarding the fractional mixed-radix positions, applies the C++
        round-up comparison, and then applies the signed correction constant
        when the complement-encoded intermediate is negative.
        """

        other = self._coerce_same_fraction_operand(other)
        if self.system.auxiliary_indices:
            critical_error(
                "SPMF.mult_std does not yet support auxiliary digit systems",
                auxiliary_indices=self.system.auxiliary_indices,
            )

        PPM.mult(self, other)
        intermediate = PPM(self)
        sign = self.calc_sign()
        correction = self._whole_range_correction_constant()
        round_up_value = self._fractional_half_range()

        self._assign_fractional_mr_shift_right(intermediate)

        if intermediate.compare_part(round_up_value, self.num_fract_digits):
            PPM.add(self, 1)
        elif intermediate.is_equal_part(round_up_value, self.num_fract_digits) and sign == POSITIVE:
            PPM.add(self, 1)

        if sign == NEGATIVE:
            PPM.sub(self, correction)
            self.sign_flag = NEGATIVE
        else:
            self.sign_flag = POSITIVE
        self.sign_valid = SIGN_VALID
        self._zero_clean()

    def mult(self, other: object) -> None:
        """Multiply in fixed-point format.

        Same-format ``SPMF`` operands use ``mult_std()``. Integer-style
        operands multiply the underlying scaled fixed-point integer directly,
        preserving the fraction point.
        """

        if isinstance(other, SPMF):
            self.mult_std(other)
            return
        self.mult_integer(other)

    def mult_integer(self, other: object) -> None:
        """Multiply this fixed-point value by an integer operand.

        This mirrors C++ ``SPMF::Mult(__int64)`` and ``SPMF::Mult(SPPM*)``.
        The fractional scale is already embedded in this value's scaled
        integer representation, so no fractional renormalization is required.
        """

        integer = self._coerce_integer_operand(other)
        SPPM.mult(self, integer)

    def to_scaled_integer(self) -> int:
        """Return the signed scaled integer for I/O and diagnostics."""

        magnitude = self._signed_magnitude_copy()
        scaled = magnitude.to_int()
        return -scaled if self.calc_sign() else scaled

    def fixed_parts(self) -> dict[str, int]:
        """Return sign, whole part, fractional numerator, and unit range."""

        scaled_integer = self.to_scaled_integer()
        sign = -1 if scaled_integer < 0 else 1
        whole, fractional_numerator = divmod(abs(scaled_integer), self.unit_range)
        return {
            "sign": sign,
            "whole": whole,
            "fractional_numerator": fractional_numerator,
            "unit_range": self.unit_range,
            "scaled_integer": scaled_integer,
        }

    def format_scaled_integer(self, radix: int = 10, *, prefix: bool = False) -> str:
        """Format the underlying signed scaled integer for diagnostics."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        scaled = self.to_scaled_integer()
        sign = "-" if scaled < 0 else ""
        text = _format_unsigned(abs(scaled), radix)
        if prefix and radix == 2:
            text = f"0b{text}"
        elif prefix and radix == 16:
            text = f"0x{text}"
        return f"{sign}{text}"

    def format_ratio(self) -> str:
        """Return exact diagnostic text as ``scaled_integer/unit_range``."""

        return f"{self.to_scaled_integer()}/{self.unit_range}"

    def format_decimal(self, *, max_fraction_digits: int = 12) -> str:
        """Return a truncated decimal diagnostic string for the fixed-point value."""

        if type(max_fraction_digits) is not int or max_fraction_digits < 0:
            raise ValueError("max_fraction_digits must be a non-negative integer")

        parts = self.fixed_parts()
        sign = "-" if parts["sign"] < 0 and parts["scaled_integer"] != 0 else ""
        whole = _format_unsigned(parts["whole"], 10)
        remainder = parts["fractional_numerator"]
        if remainder == 0 or max_fraction_digits == 0:
            return f"{sign}{whole}."

        digits: list[str] = []
        for _ in range(max_fraction_digits):
            remainder *= 10
            digit, remainder = divmod(remainder, parts["unit_range"])
            digits.append(str(digit))
            if remainder == 0:
                break
        fractional = "".join(digits).rstrip("0")
        if not fractional:
            return f"{sign}{whole}."
        return f"{sign}{whole}.{fractional}"

    def format_fixed_native(self, radix: int = 10) -> str:
        """Return raw residues with a fixed-point marker after fractional digits."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        fields: list[str] = []
        for offset, index in enumerate(self.system.value_indices):
            if offset == self.num_fract_digits:
                fields.append(".")
            digit = self.rn[index]
            if digit.skip:
                fields.append("*")
            else:
                value = _format_unsigned(digit.digit, radix)
                fields.append(f"X|{value}" if digit.power_valid != digit.power else value)
        if self.num_fract_digits == len(self.system.value_indices):
            fields.append(".")
        auxiliary = [
            "*" if self.rn[index].skip else _format_unsigned(self.rn[index].digit, radix)
            for index in self.system.auxiliary_indices
        ]
        if auxiliary:
            fields.append(f"({', '.join(auxiliary)})")
        return " ".join(fields)

    def print_fixed_native(self, radix: int = 10, *, file: TextIO | None = None) -> None:
        print(self.format_fixed_native(radix), file=file or sys.stdout)

    def print_decimal(
        self,
        *,
        max_fraction_digits: int = 12,
        file: TextIO | None = None,
    ) -> None:
        print(self.format_decimal(max_fraction_digits=max_fraction_digits), file=file or sys.stdout)

    def print_ratio(self, *, file: TextIO | None = None) -> None:
        print(self.format_ratio(), file=file or sys.stdout)

    def to_dict(self, *, include_auxiliary: bool = True) -> dict[str, object]:
        """Return a plain-Python snapshot including fixed-point metadata."""

        record = super().to_dict(include_auxiliary=include_auxiliary)
        record.update(
            {
                "class": type(self).__name__,
                "num_fract_digits": self.num_fract_digits,
                "normal_fract_digits": self.normal_fract_digits,
                "fractional_indices": self.fractional_indices,
                "whole_indices": self.whole_indices,
                "normal_fractional_indices": self.normal_fractional_indices,
                "normal_whole_indices": self.normal_whole_indices,
                "fractional_moduli": self.fractional_moduli,
                "whole_moduli": self.whole_moduli,
                "unit_range": self.unit_range,
                "normal_unit_range": self.normal_unit_range,
                "sliding_point_active": self.sliding_point_active,
                "scaled_integer": self.to_scaled_integer(),
                "fixed_parts": self.fixed_parts(),
            }
        )
        return record

    def __repr__(self) -> str:
        system_name = self.system.name or "unnamed"
        return (
            f"SPMF({self.format_native()}, "
            f"fractional_digits={self.num_fract_digits}, system={system_name!r})"
        )
