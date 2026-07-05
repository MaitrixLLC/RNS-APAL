"""Unsigned power-based residue integers."""

from __future__ import annotations

from collections.abc import Iterator
from typing import TextIO
import sys

from .digit import PPMDigit
from .modtable import RNSNumberSystem


_SUPPORTED_RADICES = (2, 10, 16)


def _format_unsigned(value: int, radix: int) -> str:
    if radix == 2:
        return format(value, "b")
    if radix == 10:
        return str(value)
    if radix == 16:
        return format(value, "x")
    raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")


class PPM:
    """A mutable unsigned integer encoded as independent residue digits.

    This first implementation slice supports normalized values only.  Each
    arithmetic method mutates the object, matching the original C++ class.
    """

    def __init__(self, value: int = 0, *, system: RNSNumberSystem) -> None:
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

    def assign(self, value: "PPM | int") -> None:
        if isinstance(value, PPM):
            self._ensure_system_compatible(value)
            self.rn = [digit.copy() for digit in value.rn]
            return
        self._assign_integer(value)

    def copy(self) -> "PPM":
        result = PPM(0, system=self.system)
        result.assign(self)
        return result

    def _ensure_system_compatible(self, other: "PPM") -> None:
        if not self.system.is_compatible(other.system):
            raise ValueError("incompatible normalized RNS systems")

    def _ensure_format_compatible(self, other: "PPM") -> None:
        self._ensure_system_compatible(other)
        for left, right in zip(self.rn, other.rn):
            if (left.power_valid, left.skip) != (right.power_valid, right.skip):
                raise ValueError(f"incompatible effective RNS formats at digit {left.index}")

    def add(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for left, right in zip(self.rn, other.rn):
                left.add(right.digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for digit in self.rn:
            digit.add(other)

    def sub(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for left, right in zip(self.rn, other.rn):
                left.sub(right.digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for digit in self.rn:
            digit.sub(other)

    def mult(self, other: "PPM | int") -> None:
        if isinstance(other, PPM):
            self._ensure_format_compatible(other)
            for left, right in zip(self.rn, other.rn):
                left.mult(right.digit)
            return
        if type(other) is not int:
            raise TypeError("PPM operands must be PPM or int instances")
        for digit in self.rn:
            digit.mult(other)

    def is_zero(self) -> bool:
        return all(digit.skip or digit.digit == 0 for digit in self.rn)

    def is_equal(self, other: "PPM") -> bool:
        self._ensure_format_compatible(other)
        return all(
            left.skip or left.digit == right.digit
            for left, right in zip(self.rn, other.rn)
        )

    def compare(self, other: "PPM") -> int:
        self._ensure_format_compatible(other)
        raise NotImplementedError("PPM comparison requires streaming mixed-radix conversion")

    def format_native(self, radix: int = 10) -> str:
        """Return the raw RNS digit string corresponding to C++ ``Prints``."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")
        fields: list[str] = []
        for digit in self.rn:
            if digit.skip:
                fields.append("*")
            else:
                value = _format_unsigned(digit.digit, radix)
                if digit.power_valid != digit.power:
                    value = f"X|{value}"
                fields.append(value)
        return " ".join(fields)

    def print_native(self, radix: int = 10, *, file: TextIO | None = None) -> None:
        print(self.format_native(radix), file=file or sys.stdout)

    def format_demo(self, radix: int = 10, *, console_width: int | None = None) -> str:
        """Format modulus headers, separators, and residues like C++ ``PrintDemo``."""

        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")
        if console_width is not None and console_width < 1:
            raise ValueError("console_width must be positive")

        columns: list[tuple[str, str, int]] = []
        for digit in self.rn:
            modulus = digit.full_modulus if digit.power_valid == 0 else digit.current_modulus
            header = _format_unsigned(modulus, radix)
            value = "*" if digit.skip else _format_unsigned(digit.digit, radix)
            width = len(header)
            if len(value) > width:
                value = "?" * width
            columns.append((header, value, width))

        groups: list[list[tuple[str, str, int]]] = []
        current: list[tuple[str, str, int]] = []
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
            lines.append(" ".join(header.ljust(width) for header, _, width in group))
            lines.append(" ".join("-" * width for _, _, width in group))
            lines.append(" ".join(value.ljust(width) for _, value, width in group))
        return "\n".join(lines)

    def print_demo(
        self,
        radix: int = 10,
        *,
        console_width: int | None = None,
        file: TextIO | None = None,
    ) -> None:
        print(self.format_demo(radix, console_width=console_width), file=file or sys.stdout)

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
        return self.format_native()

    def __repr__(self) -> str:
        system_name = self.system.name or "unnamed"
        return f"PPM({self.format_native()}, system={system_name!r})"
