"""Stored mixed-radix numbers."""

from __future__ import annotations

from dataclasses import dataclass

from .mixed_radix import MRDigit, iter_mixed_radix_digits
from .modtable import RNSNumberSystem
from .ppm import _SUPPORTED_RADICES, _format_unsigned, PPM


@dataclass(frozen=True, slots=True)
class MRN:
    """A materialized mixed-radix representation of a ``PPM`` value.

    Digits are stored least-significant first, matching the conversion order.
    Formatting defaults to most-significant first to mirror the original C++
    demonstration print routine.
    """

    digits: tuple[MRDigit, ...]
    system: RNSNumberSystem

    @classmethod
    def from_ppm(cls, value: PPM, indices: tuple[int, ...] | None = None) -> "MRN":
        if not isinstance(value, PPM):
            raise TypeError("value must be a PPM")
        return cls(tuple(iter_mixed_radix_digits(value, indices=indices)), value.system)

    @property
    def num_digits(self) -> int:
        return sum(not digit.skip for digit in self.digits)

    @property
    def radices(self) -> tuple[int, ...]:
        return tuple(digit.radix for digit in self.digits)

    def to_int(self) -> int:
        """Return the represented integer for conversion/debugging purposes."""

        seeded = False
        value = 0
        for digit in reversed(self.digits):
            if digit.skip:
                continue
            if not seeded:
                value = digit.digit
                seeded = True
            else:
                value *= digit.radix
                value += digit.digit
        return value

    def to_ppm(self, system: RNSNumberSystem | None = None) -> PPM:
        """Reconstruct this mixed-radix value into a normalized ``PPM``.

        Reconstruction is the inverse direction of mixed-radix decomposition:
        process retained digits from most-significant to least-significant,
        multiply the accumulated RNS value by the current mixed radix, and add
        the mixed-radix digit. This mirrors C++ ``MRN::Convert`` and stays in
        RNS arithmetic for the reconstruction itself.
        """

        result = PPM(0, system=system or self.system)
        seeded = False
        for digit in reversed(self.digits):
            if digit.skip:
                continue
            if not seeded:
                result.assign(digit.digit)
                seeded = True
            else:
                result.mult(digit.radix)
                result.add(digit.digit)
        return result

    def format_native(self, radix: int = 10, *, most_significant_first: bool = True) -> str:
        if radix not in _SUPPORTED_RADICES:
            raise ValueError(f"unsupported radix {radix}; expected one of {_SUPPORTED_RADICES}")

        digits = reversed(self.digits) if most_significant_first else iter(self.digits)
        fields = [
            "*" if digit.skip else _format_unsigned(digit.digit, radix)
            for digit in digits
        ]
        return ", ".join(fields)

    def __str__(self) -> str:
        return self.format_native()
