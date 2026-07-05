"""Digit-level residue arithmetic."""

from __future__ import annotations

from dataclasses import dataclass


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
