"""Signed RNS integer class for RNS-PYPAL."""

from __future__ import annotations

from .ppm import PPM
from .modtable import RNSNumberSystem


NEGATIVE = 1
POSITIVE = 0
SIGN_VALID = 1
SIGN_INVALID = 0


class SPPM(PPM):
    def __init__(self, value: int = 0, system: RNSNumberSystem | None = None):
        sign_value = NEGATIVE if value < 0 else POSITIVE
        super().__init__(value=abs(value), system=system)
        self.sign_flag = sign_value
        self.sign_valid = SIGN_VALID

    def assign(self, other: SPPM | int) -> None:
        if isinstance(other, SPPM):
            if not self.system.is_compatible(other.system):
                raise ValueError("Incompatible RNS systems")
            super().assign(other)
            self.sign_flag = other.sign_flag
            self.sign_valid = other.sign_valid
        else:
            value = int(other)
            super().assign(abs(value))
            self.sign_flag = NEGATIVE if value < 0 else POSITIVE
            self.sign_valid = SIGN_VALID

    def compare(self, other: SPPM) -> int:
        if not self.system.is_compatible(other.system):
            raise ValueError("Incompatible RNS systems")
        if self.sign_flag != other.sign_flag:
            return -1 if self.sign_flag == NEGATIVE else 1
        return super().compare(other)

    def negate(self) -> None:
        self.sign_flag = POSITIVE if self.sign_flag == NEGATIVE else NEGATIVE

    def abs(self) -> None:
        self.sign_flag = POSITIVE

    def add(self, other: SPPM | int) -> None:
        if isinstance(other, SPPM):
            if self.sign_flag == other.sign_flag:
                super().add(other)
            else:
                cmp = super().compare(other)
                if cmp == 0:
                    self.assign(0)
                    self.sign_flag = POSITIVE
                elif cmp > 0:
                    super().sub(other)
                else:
                    temp = SPPM(0, system=self.system)
                    temp.assign(other)
                    temp.sub(self)
                    temp.sign_flag = other.sign_flag
                    self.assign(temp)
            self.sign_valid = SIGN_VALID
        else:
            value = int(other)
            if value < 0:
                self.sub(abs(value))
            else:
                if self.sign_flag == NEGATIVE:
                    cmp = super().compare(SPPM(value, system=self.system))
                    if cmp >= 0:
                        super().sub(SPPM(value, system=self.system))
                    else:
                        temp = SPPM(value, system=self.system)
                        temp.sub(self)
                        self.assign(temp)
                else:
                    super().add(value)

    def sub(self, other: SPPM | int) -> None:
        if isinstance(other, SPPM):
            if self.sign_flag != other.sign_flag:
                super().add(other)
            else:
                cmp = super().compare(other)
                if cmp == 0:
                    self.assign(0)
                    self.sign_flag = POSITIVE
                elif cmp > 0:
                    super().sub(other)
                else:
                    temp = SPPM(0, system=self.system)
                    temp.assign(other)
                    temp.sub(self)
                    temp.sign_flag = NEGATIVE if self.sign_flag == POSITIVE else POSITIVE
                    self.assign(temp)
            self.sign_valid = SIGN_VALID
        else:
            value = int(other)
            self.add(-value)

    def __repr__(self) -> str:
        sign = "-" if self.sign_flag == NEGATIVE else "+"
        return f"<SPPM {sign}{self.print_digits()} system={self.system.name or 'unnamed'}>"
