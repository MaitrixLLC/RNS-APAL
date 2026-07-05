"""Fractional RNS class for RNS-PYPAL."""

from __future__ import annotations

from .sppm import SPPM
from .modtable import RNSNumberSystem
from .ppm import PPM


class SPMF(SPPM):
    def __init__(self, value: int = 0, system: RNSNumberSystem | None = None):
        super().__init__(value=value, system=system)
        self.num_fract_digits = system.num_frac_digits
        self.normal_fract_digits = self.num_fract_digits

    def assign_fp(self, decimal_string: str) -> None:
        if not decimal_string:
            raise ValueError("decimal string must not be empty")

        sign = 1
        if decimal_string[0] == "+":
            decimal_string = decimal_string[1:]
        elif decimal_string[0] == "-":
            sign = -1
            decimal_string = decimal_string[1:]

        if "." in decimal_string:
            whole_part, frac_part = decimal_string.split(".", 1)
        else:
            whole_part, frac_part = decimal_string, ""

        whole = int(whole_part) if whole_part else 0
        frac_value = int(frac_part) if frac_part else 0
        frac_scale = 10 ** len(frac_part)

        total = whole * (10 ** len(frac_part)) + frac_value
        scaled_value = total

        self.assign(0)
        self.PPM.__init__(self, value=0, system=self.system)

        range_value = 1
        for i in range(self.num_fract_digits):
            range_value *= self.system.full_modulus(i)

        numerator = scaled_value
        denom = frac_scale
        integer_value = numerator // denom
        remainder = numerator % denom

        self.assign(integer_value)
        if remainder:
            ratio = SPMF(0, system=self.system)
            ratio.assign_ratio(remainder, denom)
            self.add(ratio)

        if sign < 0:
            self.negate()

    def assign_ratio(self, numerator: int, denominator: int) -> None:
        if denominator == 0:
            raise ZeroDivisionError("denominator cannot be zero")
        if numerator == 0:
            self.assign(0)
            return

        unit = PPM(1, system=self.system)
        for i in range(self.num_fract_digits):
            unit.mult(PPM(self.system.full_modulus(i), system=self.system))

        scaled_num = numerator * unit
        scaled_num_value = scaled_num
        self.assign(scaled_num_value // denominator)

    def __repr__(self) -> str:
        return f"<SPMF value={self.print_digits()} system={self.system.name or 'unnamed'}>"
