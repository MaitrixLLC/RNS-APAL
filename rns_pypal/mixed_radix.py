"""Streaming mixed-radix conversion helpers."""

from __future__ import annotations

from dataclasses import dataclass

from .ppm import PPM
from .utils import divide_residue_by_coprime_factor


@dataclass(frozen=True, slots=True)
class MRDigit:
    """One mixed-radix digit produced from an ordered RNS digit position.

    The digits are stored least-significant first.  ``radix`` is the effective
    modulus consumed at this position.  A skipped digit records that the input
    working format did not have an active residue channel at this position.
    """

    index: int
    digit: int
    radix: int
    skip: bool = False


class MixedRadixDecomposer:
    """Incrementally decompose a ``PPM`` into mixed-radix digits.

    Each call to ``step`` exposes one mixed-radix digit and mutates an internal
    working copy.  The consumed RNS digit is invalidated through ``skip_digit``.
    Remaining active residue channels are divided by the consumed radix through
    inverse modular multiplication.
    """

    def __init__(self, value: PPM) -> None:
        if not isinstance(value, PPM):
            raise TypeError("value must be a PPM")
        self.working = value.copy()
        self._next_index = 0
        self._complete = False
        self._ensure_ordered_digits()

    @property
    def is_complete(self) -> bool:
        return self._complete

    def _ensure_ordered_digits(self) -> None:
        for expected_index, digit in enumerate(self.working.rn):
            if digit.index != expected_index:
                raise ValueError("RNS digits must remain ordered by index")

    def __iter__(self) -> "MixedRadixDecomposer":
        return self

    def __next__(self) -> MRDigit:
        try:
            return self.step()
        except StopIteration:
            raise

    def step(self) -> MRDigit:
        if self._complete or self._next_index >= self.working.num_digits:
            self._complete = True
            raise StopIteration

        index = self._next_index
        self._next_index += 1
        digit = self.working.rn[index]

        radix = digit.full_modulus if digit.power_valid == 0 else digit.current_modulus
        if digit.skip:
            mixed_digit = MRDigit(index=index, digit=0, radix=radix, skip=True)
            self._complete = self._next_index >= self.working.num_digits
            return mixed_digit

        mixed_digit = MRDigit(index=index, digit=digit.digit, radix=radix)

        if mixed_digit.digit:
            self.working.sub(mixed_digit.digit)

        digit.skip_digit()
        self._divide_remaining_channels(radix)

        if self.working.is_zero() or self._next_index >= self.working.num_digits:
            self._complete = True

        return mixed_digit

    def _divide_remaining_channels(self, radix: int) -> None:
        for digit in self.working.rn:
            if digit.skip:
                continue
            digit.assign(
                divide_residue_by_coprime_factor(
                    residue=digit.digit,
                    divisor=radix,
                    modulus=digit.current_modulus,
                )
            )


def iter_mixed_radix_digits(value: PPM) -> MixedRadixDecomposer:
    """Return a stateful iterator over ``value``'s mixed-radix digits."""

    return MixedRadixDecomposer(value)
