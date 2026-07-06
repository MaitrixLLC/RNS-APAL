"""Digit-level residue arithmetic."""

from __future__ import annotations

from dataclasses import dataclass

from .errors import critical_error
from .utils import (
    count_base_power_factor,
    divide_residue_by_coprime_factor,
    is_exact_base_power,
)


@dataclass(slots=True)
class PPMDigit:
    """One digit of a power-based residue number."""

    index: int
    digit: int
    modulus: int
    power: int = 1
    power_valid: int | None = None
    skip: bool = False
    normal_power: int | None = None

    def __post_init__(self) -> None:
        if type(self.index) is not int or self.index < 0:
            raise ValueError("index must be a non-negative integer")
        if type(self.modulus) is not int or self.modulus <= 1:
            raise ValueError("modulus must be an integer greater than one")
        if type(self.power) is not int or self.power <= 0:
            raise ValueError("power must be a positive integer")
        if self.power_valid is None:
            self.power_valid = self.power
        if self.normal_power is None:
            self.normal_power = self.power
        if type(self.power_valid) is not int:
            raise TypeError("power_valid must be an integer")
        if type(self.normal_power) is not int or self.normal_power <= 0:
            raise ValueError("normal_power must be a positive integer")
        if self.power > self.normal_power:
            raise ValueError("power cannot exceed normal_power")
        if not 0 <= self.power_valid <= self.power:
            raise ValueError("power_valid must be between zero and power")
        if self.power_valid == 0:
            self.skip = True
            self.digit = 0
        else:
            self.digit %= self.current_modulus

    @property
    def full_modulus(self) -> int:
        return self.modulus**self.power

    @property
    def current_modulus(self) -> int:
        if self.power_valid == 0:
            raise ValueError("a skipped digit has no active modulus")
        return self.modulus**self.power_valid

    @property
    def is_normalized(self) -> bool:
        return not self.skip and self.power == self.normal_power == self.power_valid

    def assign(self, value: int) -> None:
        if self.skip:
            return
        self.digit = value % self.current_modulus

    def add(self, value: int) -> None:
        if self.skip:
            return
        self.digit = (self.digit + value) % self.current_modulus

    def sub(self, value: int) -> None:
        if self.skip:
            return
        self.digit = (self.digit - value) % self.current_modulus

    def mult(self, value: int) -> None:
        if self.skip:
            return
        self.digit = (self.digit * value) % self.current_modulus

    def zero_power(self) -> int | None:
        """Return the largest valid base power that divides this digit."""

        if self.skip or self.power_valid == 0:
            return None
        for power in range(self.power_valid, 0, -1):
            if self.digit % (self.modulus**power) == 0:
                return power
        return None

    def pow_offset(self, power: int) -> int:
        """Return the offset needed to make this digit divisible by base**power."""

        if type(power) is not int or power <= 0:
            raise ValueError("power must be a positive integer")
        if power > self.power_valid:
            raise ValueError("power cannot exceed power_valid")
        return self.digit % (self.modulus**power)

    def can_reduce_by_base_power(self, divisor: int) -> bool:
        """Return whether this digit can directly consume ``divisor`` powers."""

        if self.skip:
            return True
        if self.current_modulus % divisor != 0:
            return True
        if not is_exact_base_power(divisor, self.modulus):
            return True
        return self.digit % divisor == 0

    def mod_div(self, divisor: int) -> None:
        """Divide this residue channel by ``divisor``.

        If ``divisor`` is a power of this digit's base modulus, the operation
        consumes valid powers and changes the effective RNS format. Otherwise,
        the divisor must be coprime to the current modulus and inverse modular
        multiplication is used.
        """

        if self.skip:
            return
        if type(divisor) is not int or divisor <= 0:
            raise ValueError("divisor must be a positive integer")

        if self.current_modulus % divisor == 0 and is_exact_base_power(divisor, self.modulus):
            powers_consumed = count_base_power_factor(divisor, self.modulus)
            if powers_consumed > self.power_valid:
                critical_error(
                    "divisor consumes more valid powers than the digit contains",
                    digit_index=self.index,
                    digit=self.digit,
                    base_modulus=self.modulus,
                    divisor=divisor,
                    power=self.power,
                    power_valid=self.power_valid,
                    powers_consumed=powers_consumed,
                    current_modulus=self.current_modulus,
                )
            if self.digit % divisor != 0:
                critical_error(
                    "matching base-power digit is not divisible by requested divisor",
                    digit_index=self.index,
                    digit=self.digit,
                    base_modulus=self.modulus,
                    divisor=divisor,
                    power=self.power,
                    power_valid=self.power_valid,
                    current_modulus=self.current_modulus,
                )

            self.digit = (self.digit // divisor) % (
                self.modulus ** (self.power_valid - powers_consumed)
            )
            self.power_valid -= powers_consumed
            if self.power_valid == 0:
                self.skip_digit()
            return

        self.digit = divide_residue_by_coprime_factor(
            residue=self.digit,
            divisor=divisor,
            modulus=self.current_modulus,
        )

    def skip_digit(self) -> None:
        """Invalidate this residue channel in the current working format."""

        self.power_valid = 0
        self.skip = True
        self.digit = 0

    def copy(self) -> "PPMDigit":
        return PPMDigit(
            index=self.index,
            digit=self.digit,
            modulus=self.modulus,
            power=self.power,
            power_valid=self.power_valid,
            skip=self.skip,
            normal_power=self.normal_power,
        )

    def to_record(self) -> dict[str, int | bool | None]:
        """Return a plain-Python snapshot of this digit's current state."""

        return {
            "index": self.index,
            "digit": self.digit,
            "modulus": self.modulus,
            "power": self.power,
            "power_valid": self.power_valid,
            "normal_power": self.normal_power,
            "skip": self.skip,
            "full_modulus": self.full_modulus,
            "current_modulus": None if self.skip else self.current_modulus,
        }
