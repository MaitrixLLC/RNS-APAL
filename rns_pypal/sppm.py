"""Signed partial-power modulus integers."""

from __future__ import annotations

from typing import TextIO
import sys

from .errors import critical_error
from .modtable import RNSNumberSystem
from .ppm import _SUPPORTED_RADICES, PPM


POSITIVE = 0
NEGATIVE = 1
SIGN_INVALID = 0
SIGN_VALID = 1


class SPPM(PPM):
    """Signed integer interpretation layered on top of ``PPM`` residues.

    The method-of-complements encoded residue value is authoritative. The sign
    flag is cached metadata used for speed and consistency checks.
    """

    def __init__(
        self,
        value: "SPPM | PPM | int | str" = 0,
        *,
        system: RNSNumberSystem | None = None,
    ) -> None:
        if isinstance(value, PPM) and system is None:
            system = value.system
        self.sign_flag = POSITIVE
        self.sign_valid = SIGN_VALID
        super().__init__(0, system=system)
        self.assign(value)

    def _ensure_signed_supported(self) -> None:
        if self.system.auxiliary_indices:
            critical_error(
                "SPPM does not yet support auxiliary digit systems",
                auxiliary_indices=self.system.auxiliary_indices,
                digit_roles=tuple(str(role) for role in self.system.digit_roles),
            )

    def _zero_clean(self) -> bool:
        if self.is_zero():
            self.sign_flag = POSITIVE
            self.sign_valid = SIGN_VALID
            return True
        return False

    def _complement_in_place(self) -> None:
        zero = PPM(0, system=self.system)
        zero.sub(self)
        PPM.assign(self, zero)

    @staticmethod
    def _split_signed_text(value: str) -> tuple[int, str]:
        text = value.strip()
        if not text:
            raise ValueError("SPPM string assignment requires at least one digit")
        sign = POSITIVE
        if text[0] == "+":
            text = text[1:]
        elif text[0] == "-":
            sign = NEGATIVE
            text = text[1:]
        if not text:
            raise ValueError("SPPM string assignment requires at least one digit")
        return sign, text

    def _assign_signed_integer(self, value: int) -> None:
        self._ensure_signed_supported()
        if type(value) is not int:
            raise TypeError("SPPM integer values must be int instances")
        if value < 0:
            PPM.assign(self, -value)
            self._complement_in_place()
            self.sign_flag = NEGATIVE
        else:
            PPM.assign(self, value)
            self.sign_flag = POSITIVE
        self.sign_valid = SIGN_VALID
        self._zero_clean()

    def _assign_signed_text(self, value: str) -> None:
        sign, unsigned_text = self._split_signed_text(value)
        PPM.assign(self, unsigned_text)
        if sign == NEGATIVE:
            self._complement_in_place()
        self.sign_flag = sign
        self.sign_valid = SIGN_VALID
        self._zero_clean()

    def assign(self, value: "SPPM | PPM | int | str") -> None:
        if isinstance(value, SPPM):
            self._ensure_system_compatible(value)
            PPM.assign(self, value)
            self.sign_flag = value.sign_flag
            self.sign_valid = value.sign_valid
            self._zero_clean()
            return
        if isinstance(value, PPM):
            self._ensure_system_compatible(value)
            PPM.assign(self, value)
            self.sign_flag = self.calc_sign()
            self.sign_valid = SIGN_VALID
            self._zero_clean()
            return
        if isinstance(value, str):
            self._assign_signed_text(value)
            return
        self._assign_signed_integer(value)

    def copy(self) -> "SPPM":
        result = SPPM(0, system=self.system)
        result.assign(self)
        return result

    @classmethod
    def positive_range_max(cls, system: RNSNumberSystem) -> PPM:
        """Return the largest ordinary positive signed value for ``system``."""

        if system.dynamic_range < 2:
            raise ValueError("signed range requires dynamic_range of at least two")
        return PPM((system.dynamic_range - 1) // 2, system=system)

    @classmethod
    def negative_range_min_magnitude(cls, system: RNSNumberSystem) -> PPM:
        """Return the magnitude of the most-negative complement value."""

        return PPM(system.dynamic_range // 2, system=system)

    def calc_sign(self) -> int:
        """Return the sign implied by the complement-encoded residues."""

        if self.is_zero():
            return POSITIVE
        range_max = self.positive_range_max(self.system)
        return NEGATIVE if PPM.compare(self, range_max) else POSITIVE

    def calc_set_sign(self) -> int:
        """Compute the residue-implied sign and mark the cached sign valid."""

        sign = self.calc_sign()
        if self.sign_valid == SIGN_VALID and self.sign_flag != sign:
            critical_error(
                "SPPM valid sign flag disagrees with complement encoding",
                cached_sign=self.sign_flag,
                residue_sign=sign,
                value_native=self.format_native(),
            )
        self.sign_flag = sign
        self.sign_valid = SIGN_VALID
        self._zero_clean()
        return self.sign_flag

    def sign_mismatch(self) -> bool:
        """Return whether a valid cached sign disagrees with the residues."""

        return self.sign_valid == SIGN_VALID and self.sign_flag != self.calc_sign()

    def get_sign_flag(self) -> int:
        return self.sign_flag

    def get_sign_valid(self) -> int:
        return self.sign_valid

    def negate(self) -> None:
        if self._zero_clean():
            return
        self._complement_in_place()
        if self.sign_valid == SIGN_VALID:
            self.sign_flag = POSITIVE if self.sign_flag == NEGATIVE else NEGATIVE
        self._zero_clean()

    def abs(self) -> None:
        if self.sign_valid != SIGN_VALID:
            self.calc_set_sign()
        if self.sign_flag == NEGATIVE:
            self.negate()

    def add(self, other: "SPPM | int | str") -> None:
        if not isinstance(other, SPPM):
            other = SPPM(other, system=self.system)
        self._ensure_format_compatible(other)

        left_zero = self.is_zero()
        right_zero = other.is_zero()
        left_valid = self.sign_valid
        right_valid = other.sign_valid
        left_sign = self.sign_flag
        right_sign = other.sign_flag

        PPM.add(self, other)

        if self._zero_clean():
            return
        if left_valid == SIGN_VALID and right_valid == SIGN_VALID:
            if right_zero:
                self.sign_flag = left_sign
                self.sign_valid = SIGN_VALID
            elif left_zero:
                self.sign_flag = right_sign
                self.sign_valid = SIGN_VALID
            elif left_sign == right_sign:
                self.sign_flag = left_sign
                self.sign_valid = SIGN_VALID
            else:
                self.sign_valid = SIGN_INVALID
        else:
            self.sign_valid = SIGN_INVALID

    def sub(self, other: "SPPM | int | str") -> None:
        if not isinstance(other, SPPM):
            other = SPPM(other, system=self.system)
        self._ensure_format_compatible(other)

        left_zero = self.is_zero()
        right_zero = other.is_zero()
        left_valid = self.sign_valid
        right_valid = other.sign_valid
        left_sign = self.sign_flag
        right_sign = other.sign_flag

        PPM.sub(self, other)

        if self._zero_clean():
            return
        if left_valid == SIGN_VALID and right_valid == SIGN_VALID:
            if right_zero:
                self.sign_flag = left_sign
                self.sign_valid = SIGN_VALID
            elif left_zero:
                self.sign_flag = POSITIVE if right_sign == NEGATIVE else NEGATIVE
                self.sign_valid = SIGN_VALID
            elif left_sign != right_sign:
                self.sign_flag = left_sign
                self.sign_valid = SIGN_VALID
            else:
                self.sign_valid = SIGN_INVALID
        else:
            self.sign_valid = SIGN_INVALID

    def mult(self, other: "SPPM | int | str") -> None:
        if not isinstance(other, SPPM):
            other = SPPM(other, system=self.system)
        self._ensure_format_compatible(other)

        left_valid = self.sign_valid
        right_valid = other.sign_valid
        left_sign = self.sign_flag
        right_sign = other.sign_flag

        PPM.mult(self, other)

        if self._zero_clean():
            return
        if left_valid == SIGN_VALID and right_valid == SIGN_VALID:
            self.sign_flag = POSITIVE if left_sign == right_sign else NEGATIVE
            self.sign_valid = SIGN_VALID
        else:
            self.sign_valid = SIGN_INVALID

    def increment(self) -> None:
        self.add(1)

    def decrement(self) -> None:
        self.sub(1)

    def compare(self, other: "SPPM | int | str") -> int:
        if not isinstance(other, SPPM):
            other = SPPM(other, system=self.system)
        self._ensure_format_compatible(other)

        left_sign = self.sign_flag if self.sign_valid == SIGN_VALID else self.calc_set_sign()
        right_sign = other.sign_flag if other.sign_valid == SIGN_VALID else other.calc_set_sign()

        if left_sign == POSITIVE and right_sign == NEGATIVE:
            return 1
        if left_sign == NEGATIVE and right_sign == POSITIVE:
            return 0
        return PPM.compare(self, other)

    def _signed_magnitude_copy(self) -> PPM:
        magnitude = PPM(self)
        if self.calc_sign() == NEGATIVE:
            zero = PPM(0, system=self.system)
            zero.sub(magnitude)
            magnitude = zero
        return magnitude

    def format_value(self, radix: int = 10, *, prefix: bool = False) -> str:
        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        sign = self.calc_sign()
        magnitude = self._signed_magnitude_copy()
        text = magnitude.format_value(radix, prefix=prefix)
        if sign == NEGATIVE and text not in ("0", "0b0", "0x0"):
            return f"-{text}"
        return text

    def print_value(
        self,
        radix: int = 10,
        *,
        prefix: bool = False,
        file: TextIO | None = None,
    ) -> None:
        print(self.format_value(radix, prefix=prefix), file=file or sys.stdout)

    def __str__(self) -> str:
        return self.format_value()

    def __repr__(self) -> str:
        sign = "-" if self.sign_flag == NEGATIVE else "+"
        validity = "valid" if self.sign_valid == SIGN_VALID else "invalid"
        system_name = self.system.name or "unnamed"
        return f"SPPM({sign}{self.format_native()}, sign={validity}, system={system_name!r})"
