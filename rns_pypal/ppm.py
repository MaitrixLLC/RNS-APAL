"""Unsigned power-based residue integers."""

from __future__ import annotations

from collections.abc import Iterator
from typing import TextIO
import sys

from .digit import PPMDigit
from .errors import critical_error
from .modtable import RNSNumberSystem


_SUPPORTED_RADICES = (2, 10, 16)


def _parse_unsigned_text(value: str) -> tuple[int, list[int]]:
    """Parse an unsigned binary, decimal, or hexadecimal string into digits."""

    if not isinstance(value, str):
        raise TypeError("value must be a string")

    text = value.strip().replace("_", "")
    if not text:
        raise ValueError("PPM string assignment requires at least one digit")
    if text[0] == "+":
        text = text[1:]
    if not text:
        raise ValueError("PPM string assignment requires at least one digit")
    if text[0] == "-":
        raise ValueError("PPM is unsigned; assignment requires a non-negative string")

    radix = 10
    if text.lower().startswith("0x"):
        radix = 16
        text = text[2:]
    elif text.lower().startswith("0b"):
        radix = 2
        text = text[2:]

    if not text:
        raise ValueError("PPM string assignment requires at least one digit")

    digits: list[int] = []
    for character in text:
        if "0" <= character <= "9":
            digit = ord(character) - ord("0")
        elif "a" <= character.lower() <= "f":
            digit = ord(character.lower()) - ord("a") + 10
        else:
            raise ValueError(f"invalid base-{radix} digit {character!r}")
        if digit >= radix:
            raise ValueError(f"invalid base-{radix} digit {character!r}")
        digits.append(digit)
    return radix, digits


def _format_decimal_arbitrary(value: int) -> str:
    """Format an integer as decimal without relying on Python's int->str limit."""

    if value == 0:
        return "0"

    sign = ""
    if value < 0:
        sign = "-"
        value = -value

    chunk_base = 1_000_000_000
    chunks: list[int] = []
    while value:
        value, chunk = divmod(value, chunk_base)
        chunks.append(chunk)

    leading = str(chunks[-1])
    trailing = "".join(f"{chunk:09d}" for chunk in reversed(chunks[:-1]))
    return f"{sign}{leading}{trailing}"


def _format_unsigned(value: int, radix: int) -> str:
    if radix == 2:
        return format(value, "b")
    if radix == 10:
        return _format_decimal_arbitrary(value)
    if radix == 16:
        return format(value, "x")
    raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")


class PPM:
    """A mutable unsigned integer encoded as independent residue digits.

    This first implementation slice supports normalized values only.  Each
    arithmetic method mutates the object, matching the original C++ class.
    """

    def __init__(self, value: "PPM | int | str" = 0, *, system: RNSNumberSystem | None = None) -> None:
        if isinstance(value, PPM) and system is None:
            system = value.system
        if not isinstance(system, RNSNumberSystem):
            raise TypeError("system must be an RNSNumberSystem")
        self.system = system
        self.rn: list[PPMDigit] = []
        self.assign(value)

    @property
    def num_digits(self) -> int:
        return len(self.rn)

    @property
    def power_based(self) -> bool:
        return self.system.power_based

    def __iter__(self) -> Iterator[PPMDigit]:
        return iter(self.rn)

    def _assign_integer(self, value: int) -> None:
        if type(value) is not int:
            raise TypeError("PPM integer values must be int instances")
        if value < 0:
            raise ValueError("PPM is unsigned; assignment requires a non-negative integer")
        self.rn = [
            PPMDigit(index, value, modulus, power)
            for index, (modulus, power) in enumerate(zip(self.system.moduli, self.system.powers))
        ]

    def _assign_text(self, value: str) -> None:
        radix, digits = _parse_unsigned_text(value)
        self._assign_integer(0)
        for digit in digits:
            self.mult(radix)
            self.add(digit)

    def assign(self, value: "PPM | int | str") -> None:
        if isinstance(value, PPM):
            self._ensure_system_compatible(value)
            self.rn = [digit.copy() for digit in value.rn]
            return
        if isinstance(value, str):
            self._assign_text(value)
            return
        self._assign_integer(value)

    def _promote_valid_powers_to_current_format(self) -> None:
        for digit in self.rn:
            if digit.skip:
                continue
            digit.power = digit.power_valid

    def _apply_current_format_from(self, source: "PPM") -> None:
        self._ensure_system_compatible(source)
        self.rn = [
            PPMDigit(
                index=digit.index,
                digit=0,
                modulus=digit.modulus,
                power=digit.power_valid if not digit.skip else digit.power,
                power_valid=digit.power_valid,
                skip=digit.skip,
                normal_power=digit.normal_power,
            )
            for digit in source.rn
        ]

    def _assign_integer_in_current_format(self, value: int) -> None:
        if type(value) is not int:
            raise TypeError("PPM integer values must be int instances")
        if value < 0:
            raise ValueError("PPM is unsigned; assignment requires a non-negative integer")
        for digit in self.rn:
            digit.assign(value)

    def _assign_text_in_current_format(self, value: str) -> None:
        radix, digits = _parse_unsigned_text(value)
        self._assign_integer_in_current_format(0)
        for digit in digits:
            self.mult(radix)
            self.add(digit)

    def assign_pm(self, value: "PPM | int | str", *, format_source: "PPM | None" = None) -> None:
        """Assign while preserving or creating a derived partial-power format.

        ``assign()`` resets integer and string inputs to the normalized system,
        matching the C++ ``Assign`` behavior. ``assign_pm()`` corresponds to the
        C++ ``AssignPM`` family: integer and string inputs are assigned into the
        current effective format, while a PPM source can promote its current
        ``power_valid`` structure into this object's working ``power`` format.
        """

        if format_source is not None:
            self._apply_current_format_from(format_source)
            if isinstance(value, PPM):
                self._ensure_format_compatible(value)
                self.rn = [digit.copy() for digit in value.rn]
            elif isinstance(value, str):
                self._assign_text_in_current_format(value)
            else:
                self._assign_integer_in_current_format(value)
            return

        if isinstance(value, PPM):
            self.assign(value)
            self._promote_valid_powers_to_current_format()
            return
        if isinstance(value, str):
            self._assign_text_in_current_format(value)
            return
        self._assign_integer_in_current_format(value)

    def copy(self) -> "PPM":
        result = PPM(0, system=self.system)
        result.assign(self)
        return result

    def derived_format_copy(self) -> "PPM":
        """Return a copy whose active powers become its current derived powers.

        This mirrors the C++ pattern used by derived partial-power values:
        ``PowerValid`` is promoted to ``Power`` for active digits, while
        ``NormalPower`` remains unchanged so ``normalize()`` can still restore
        the original full system.
        """

        result = self.copy()
        for digit in result.rn:
            if digit.skip:
                continue
            digit.power = digit.power_valid
        return result

    def _ensure_system_compatible(self, other: "PPM") -> None:
        if not self.system.is_compatible(other.system):
            raise ValueError("incompatible normalized RNS systems")

    def _ensure_format_compatible(self, other: "PPM") -> None:
        self._ensure_system_compatible(other)
        for left, right in zip(self.rn, other.rn):
            if (left.power_valid, left.skip) != (right.power_valid, right.skip):
                raise ValueError(f"incompatible effective RNS formats at digit {left.index}")

    def _zero_with_current_power_format(self) -> "PPM":
        result = PPM(0, system=self.system)
        result.rn = [
            PPMDigit(
                index=digit.index,
                digit=0,
                modulus=digit.modulus,
                power=digit.power,
                power_valid=digit.power,
                skip=False,
                normal_power=digit.normal_power,
            )
            for digit in self.rn
        ]
        return result

    def _reconstruct_mixed_radix_to_current_power(self, mixed_radix: object) -> "PPM":
        result = self._zero_with_current_power_format()
        seeded = False
        for digit in reversed(mixed_radix.digits):
            if digit.skip:
                continue
            if not seeded:
                result._assign_integer_in_current_format(digit.digit)
                seeded = True
            else:
                result.mult(digit.radix)
                result.add(digit.digit)
        return result

    def add(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for index in self.system.arithmetic_indices:
                self.rn[index].add(other.rn[index].digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for index in self.system.arithmetic_indices:
            self.rn[index].add(other)

    def sub(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for index in self.system.arithmetic_indices:
                self.rn[index].sub(other.rn[index].digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for index in self.system.arithmetic_indices:
            self.rn[index].sub(other)

    def mult(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for index in self.system.arithmetic_indices:
                self.rn[index].mult(other.rn[index].digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for index in self.system.arithmetic_indices:
            self.rn[index].mult(other)

    def increment(self) -> None:
        """Increment this value by one in its current effective format."""

        self.add(1)

    def decrement(self) -> None:
        """Decrement this value by one in its current effective format."""

        self.sub(1)

    def mod_div(self, divisor: int) -> None:
        """Divide by ``divisor`` in the residue domain.

        For a digit whose base modulus is consumed by ``divisor``, this reduces
        the digit's valid power and may skip the digit. Other active digits use
        inverse modular multiplication. This is a format-changing low-level RNS
        operation, not a convert-to-integer division shortcut.
        """

        if type(divisor) is not int or divisor <= 0:
            raise ValueError("divisor must be a positive integer")
        for digit in self.rn:
            if not digit.can_reduce_by_base_power(divisor):
                critical_error(
                    "matching base-power digit is not divisible by requested divisor",
                    digit_index=digit.index,
                    digit=digit.digit,
                    base_modulus=digit.modulus,
                    divisor=divisor,
                    power=digit.power,
                    power_valid=digit.power_valid,
                    current_modulus=digit.current_modulus,
                    ppm_native=self.format_native(),
                    ppm_system=self.system.name,
                )
        for digit in self.rn:
            digit.mod_div(divisor)

    def normalize(self) -> None:
        """Restore this value to the full normalized RNS format.

        The value is decomposed through mixed-radix conversion using its current
        effective format, then reconstructed into the parent system's full
        normalized format through RNS multiply/add operations.
        """

        from .mrn import MRN

        normalized = MRN.from_ppm(self).to_ppm(self.system)
        self.assign(normalized)

    def normalized_copy(self) -> "PPM":
        """Return a normalized copy of this value."""

        result = self.copy()
        result.normalize()
        return result

    def extend_to_current_power(self) -> None:
        """Restore skipped/partial valid powers to this value's current ``Power``.

        This is the Python counterpart to the C++ ``ExtendPart2Norm`` behavior:
        the value is decomposed through mixed-radix conversion using the current
        effective format, then reconstructed to each digit's current derived
        ``Power``. Unlike ``normalize()``, this does not restore ``Power`` to
        ``NormalPower``.
        """

        from .mrn import MRN

        extended = self._reconstruct_mixed_radix_to_current_power(MRN.from_ppm(self))
        self.assign(extended)

    def extended_to_current_power_copy(self) -> "PPM":
        """Return a copy extended to its current derived powers."""

        result = self.copy()
        result.extend_to_current_power()
        return result

    def is_zero(self) -> bool:
        return all(
            self.rn[index].skip or self.rn[index].digit == 0
            for index in self.system.comparison_indices
        )

    def is_one(self) -> bool:
        return all(
            self.rn[index].skip or self.rn[index].digit == 1
            for index in self.system.comparison_indices
        )

    def is_equal(self, other: "PPM") -> bool:
        self._ensure_format_compatible(other)
        return all(
            self.rn[index].skip or self.rn[index].digit == other.rn[index].digit
            for index in self.system.comparison_indices
        )

    def compare(self, other: "PPM") -> int:
        """Return ``1`` if this unsigned value is greater than ``other``.

        This follows the C++ comparison strategy: stream mixed-radix digits from
        least-significant to most-significant and let later unequal digits
        override the comparison state.
        """

        self._ensure_format_compatible(other)

        from .mixed_radix import MixedRadixDecomposer

        left = MixedRadixDecomposer(self)
        right = MixedRadixDecomposer(other)
        compare_flag = 0

        while True:
            try:
                left_digit = left.step()
            except StopIteration:
                left_done = True
                left_digit = None
            else:
                left_done = left.is_complete

            try:
                right_digit = right.step()
            except StopIteration:
                right_done = True
                right_digit = None
            else:
                right_done = right.is_complete

            if left_digit is not None and right_digit is not None:
                if left_digit.digit > right_digit.digit:
                    compare_flag = 1
                elif left_digit.digit != right_digit.digit:
                    compare_flag = 0

            if left_done or right_done:
                break

        if not left.working.is_zero() and right.working.is_zero():
            return 1
        if left.working.is_zero() and not right.working.is_zero():
            return 0
        return compare_flag

    def any_part_skips(self) -> bool:
        return any(digit.power_valid != digit.power for digit in self.rn)

    def _has_divisible_power_factor_before(self, end_index: int) -> bool:
        for digit in self.rn[:end_index]:
            if digit.zero_power() is not None:
                return True
        return False

    def _dec_by_next_factor(
        self,
        factor_source: "PPM",
        start_index: int,
        end_index: int,
    ) -> tuple[bool, int, int, int]:
        """Subtract enough to make this value divisible by factor_source's next factor."""

        for index in range(start_index, end_index):
            power = factor_source.rn[index].zero_power()
            if power is not None:
                offset = self.rn[index].pow_offset(power)
                if offset:
                    self.sub(offset)
                return True, offset, index, power
        return False, 0, start_index, 0

    def _index_of_base_modulus(self, base: int) -> int | None:
        for digit in self.rn:
            if digit.modulus == base:
                return digit.index
        return None

    def _division_check(self, dividend: "PPM", divisor: "PPM", quotient: "PPM", remainder: "PPM") -> bool:
        if not divisor.compare(remainder):
            return False
        reconstructed = divisor.copy()
        reconstructed.mult(quotient)
        reconstructed.add(remainder)
        return dividend.is_equal(reconstructed)

    def div_std(self, divisor: "PPM", *, max_iterations: int = 1_000_000) -> "PPM":
        """Unsigned arbitrary integer division in RNS.

        This ports the current C++ container path ``DivStd`` -> ``DivPM7``. The
        calling value is replaced by the quotient, and the returned ``PPM`` is
        the remainder.
        """

        if not isinstance(divisor, PPM):
            raise TypeError("divisor must be a PPM")
        self._ensure_format_compatible(divisor)
        if self.system.auxiliary_indices:
            critical_error(
                "PPM.div_std does not yet support auxiliary digits",
                auxiliary_indices=self.system.auxiliary_indices,
                digit_roles=tuple(str(role) for role in self.system.digit_roles),
            )

        mod2_index = self._index_of_base_modulus(2)
        if mod2_index is None:
            critical_error(
                "PPM.div_std requires an RNS system containing base modulus 2",
                ppm_system=self.system.name,
                moduli=self.system.moduli,
                powers=self.system.powers,
            )

        if divisor.is_zero():
            critical_error("divide by zero in PPM.div_std", dividend=self.format_native())

        remainder = PPM(0, system=self.system)
        if divisor.is_one():
            return remainder
        if self.is_zero():
            return remainder
        if self.is_equal(divisor):
            self.assign(1)
            return remainder

        dividend_orig = self.copy()
        divisor_orig = divisor.copy()
        dividend_copy = self.copy()
        dividend = self.copy()
        work_divisor = divisor.copy()
        accumulator = PPM(0, system=self.system)
        difference = PPM(0, system=self.system)
        temp = PPM(0, system=self.system)

        div_done = False
        iterations = 0
        while not div_done:
            iterations += 1
            if iterations > max_iterations:
                critical_error(
                    "PPM.div_std exceeded max_iterations",
                    max_iterations=max_iterations,
                    dividend=dividend.format_native(),
                    divisor=work_divisor.format_native(),
                    accumulator=accumulator.format_native(),
                )

            prime_index = 0
            if work_divisor._has_divisible_power_factor_before(work_divisor.num_digits):
                scale_count = 0
                while not work_divisor.is_one():
                    found, _, factor_index, power = dividend._dec_by_next_factor(
                        work_divisor,
                        prime_index,
                        work_divisor.num_digits,
                    )
                    if found:
                        factor = work_divisor.rn[factor_index].modulus**power
                        dividend.mod_div(factor)
                        work_divisor.mod_div(factor)
                        scale_count += 1
                        prime_index = factor_index + 1
                    elif work_divisor._has_divisible_power_factor_before(work_divisor.num_digits):
                        prime_index = 0
                    else:
                        break

                if work_divisor.rn[mod2_index].skip:
                    dividend.extend_to_current_power()
                    work_divisor.extend_to_current_power()

                if work_divisor.is_one() or dividend.is_zero():
                    if work_divisor.any_part_skips():
                        dividend.extend_to_current_power()
                        work_divisor.extend_to_current_power()

                    accumulator.add(dividend)

                    difference.assign(dividend_copy)
                    temp.assign(divisor_orig)
                    temp.mult(dividend)
                    difference.sub(temp)

                    if not dividend.is_zero():
                        dividend_copy.assign(difference)
                        dividend.assign(difference)
                        work_divisor.assign(divisor_orig)
                    else:
                        if difference.is_equal(temp) and not difference.is_zero():
                            accumulator.increment()
                        elif dividend.is_zero() and (
                            difference.is_equal(divisor_orig) or difference.compare(divisor_orig)
                        ):
                            accumulator.increment()

                        temp.assign(divisor_orig)
                        temp.mult(accumulator)
                        remainder.assign(dividend_orig)
                        remainder.sub(temp)

                        self.assign(accumulator)

                        if not self._division_check(dividend_orig, divisor_orig, accumulator, remainder):
                            critical_error(
                                "internal error in PPM.div_std division check",
                                dividend=dividend_orig.format_native(),
                                divisor=divisor_orig.format_native(),
                                quotient=accumulator.format_native(),
                                remainder=remainder.format_native(),
                            )
                        div_done = True
                else:
                    # The C++ branch is intentionally empty here.
                    pass
            else:
                work_divisor.increment()

        return remainder

    def to_int(self) -> int:
        """Convert this unsigned RNS value to a Python integer for output/debugging.

        This is an explicit conversion routine. It is allowed for printing and
        external representation, but arithmetic methods must not use it to
        compute residue-domain results.
        """

        from .mrn import MRN

        return MRN.from_ppm(self).to_int()

    def format_value(self, radix: int = 10, *, prefix: bool = False) -> str:
        """Return the represented unsigned value in a fixed-radix string.

        This is the fallback conversion print path. It works for any valid
        normalized ``PPM`` geometry because it converts through mixed-radix
        digits, then formats the resulting arbitrary-precision Python integer.
        """

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        value = self.to_int()
        text = _format_unsigned(value, radix)
        if prefix and radix == 2:
            return f"0b{text}"
        if prefix and radix == 16:
            return f"0x{text}"
        return text

    def print_value(
        self,
        radix: int = 10,
        *,
        prefix: bool = False,
        file: TextIO | None = None,
    ) -> None:
        print(self.format_value(radix, prefix=prefix), file=file or sys.stdout)

    def format_native(self, radix: int = 10) -> str:
        """Return the raw RNS digit string corresponding to C++ ``Prints``."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        def format_digit(index: int) -> str:
            digit = self.rn[index]
            if digit.skip:
                return "*"
            value = _format_unsigned(digit.digit, radix)
            if digit.power_valid != digit.power:
                value = f"X|{value}"
            return value

        value_fields = [format_digit(index) for index in self.system.value_indices]
        auxiliary_fields = [format_digit(index) for index in self.system.auxiliary_indices]
        if auxiliary_fields:
            return f"{' '.join(value_fields)} ({', '.join(auxiliary_fields)})"
        return " ".join(value_fields)

    def print_native(self, radix: int = 10, *, file: TextIO | None = None) -> None:
        print(self.format_native(radix), file=file or sys.stdout)

    def format_whdr(self, radix: int = 10, *, console_width: int | None = None) -> str:
        """Format modulus headers, separators, and residues."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")
        if console_width is not None and console_width < 1:
            raise ValueError("console_width must be positive")

        def column_for_index(index: int) -> tuple[str, str, int, str]:
            digit = self.rn[index]
            modulus = digit.full_modulus if digit.power_valid == 0 else digit.current_modulus
            header = _format_unsigned(modulus, radix)
            value = "*" if digit.skip else _format_unsigned(digit.digit, radix)
            marker = "-" if digit.skip or digit.power_valid == digit.power else "="
            width = len(header)
            if len(value) > width:
                value = "?" * width
            return header, value, width, marker

        def grouped_auxiliary_column(columns: list[tuple[str, str, int, str]]) -> tuple[str, str, int, str]:
            header = f"({' '.join(column[0].ljust(column[2]) for column in columns)})"
            marker = f"({' '.join(column[3] * column[2] for column in columns)})"
            value = f"({' '.join(column[1].ljust(column[2]) for column in columns)})"
            width = len(header)
            return header, value, width, marker

        columns = [column_for_index(index) for index in self.system.value_indices]
        auxiliary_columns = [column_for_index(index) for index in self.system.auxiliary_indices]
        if auxiliary_columns:
            columns.append(grouped_auxiliary_column(auxiliary_columns))

        groups: list[list[tuple[str, str, int, str]]] = []
        current: list[tuple[str, str, int, str]] = []
        current_width = 0
        for column in columns:
            column_width = column[2] + 1
            if console_width is not None and current and current_width + column_width > console_width:
                groups.append(current)
                current = []
                current_width = 0
            current.append(column)
            current_width += column_width
        if current:
            groups.append(current)

        lines: list[str] = []
        for group in groups:
            lines.append(" ".join(header.ljust(width) for header, _, width, _ in group))
            lines.append(
                " ".join(
                    marker.ljust(width) if len(marker) > 1 else marker * width
                    for _, _, width, marker in group
                )
            )
            lines.append(" ".join(value.ljust(width) for _, value, width, _ in group))
        return "\n".join(lines)

    def print_whdr(
        self,
        radix: int = 10,
        *,
        console_width: int | None = None,
        file: TextIO | None = None,
    ) -> None:
        print(self.format_whdr(radix, console_width=console_width), file=file or sys.stdout)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, PPM):
            return NotImplemented
        if not self.system.is_compatible(other.system):
            return False
        return all(
            (left.power_valid, left.skip, left.digit)
            == (right.power_valid, right.skip, right.digit)
            for left, right in zip(self.rn, other.rn)
        )

    def __str__(self) -> str:
        return self.format_value()

    def __repr__(self) -> str:
        system_name = self.system.name or "unnamed"
        return f"PPM({self.format_native()}, system={system_name!r})"
