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

    def __init__(self, value: PPM, indices: tuple[int, ...] | None = None) -> None:
        if not isinstance(value, PPM):
            raise TypeError("value must be a PPM")
        self.working = value.copy()
        self.indices = indices or value.system.conversion_indices
        self.digits: list[MRDigit] = []
        self._next_position = 0
        self._complete = False
        self._ensure_ordered_digits()
        self._validate_indices()

    @property
    def is_complete(self) -> bool:
        return self._complete

    def _ensure_ordered_digits(self) -> None:
        for expected_index, digit in enumerate(self.working.rn):
            if digit.index != expected_index:
                raise ValueError("RNS digits must remain ordered by index")

    def _validate_indices(self) -> None:
        if not self.indices:
            raise ValueError("mixed-radix conversion requires at least one digit index")
        if any(type(index) is not int or not 0 <= index < self.working.num_digits for index in self.indices):
            raise ValueError("mixed-radix digit indices are out of range")
        if len(set(self.indices)) != len(self.indices):
            raise ValueError("mixed-radix digit indices must not contain duplicates")

    def __iter__(self) -> "MixedRadixDecomposer":
        return self

    def __next__(self) -> MRDigit:
        try:
            return self.step()
        except StopIteration:
            raise

    def step(self) -> MRDigit:
        if self._complete or self._next_position >= len(self.indices):
            self._complete = True
            raise StopIteration

        index = self.indices[self._next_position]
        self._next_position += 1
        digit = self.working.rn[index]

        radix = digit.full_modulus if digit.power_valid == 0 else digit.current_modulus
        if digit.skip:
            mixed_digit = MRDigit(index=index, digit=0, radix=radix, skip=True)
            self.digits.append(mixed_digit)
            self._complete = self._next_position >= len(self.indices)
            return mixed_digit

        mixed_digit = MRDigit(index=index, digit=digit.digit, radix=radix)
        self.digits.append(mixed_digit)

        if mixed_digit.digit:
            self._subtract_from_conversion_channels(mixed_digit.digit)

        digit.skip_digit()
        self._divide_remaining_channels(radix)

        if self._conversion_channels_are_zero() or self._next_position >= len(self.indices):
            self._complete = True

        return mixed_digit

    def _remaining_indices(self) -> tuple[int, ...]:
        return self.indices[self._next_position :]

    def _subtract_from_conversion_channels(self, value: int) -> None:
        for index in self.indices:
            self.working.rn[index].sub(value)

    def _divide_remaining_channels(self, radix: int) -> None:
        for index in self._remaining_indices():
            digit = self.working.rn[index]
            if digit.skip:
                continue
            digit.assign(
                divide_residue_by_coprime_factor(
                    residue=digit.digit,
                    divisor=radix,
                    modulus=digit.current_modulus,
                )
            )

    def _conversion_channels_are_zero(self) -> bool:
        return all(
            self.working.rn[index].skip or self.working.rn[index].digit == 0
            for index in self._remaining_indices()
        )


def iter_mixed_radix_digits(value: PPM, indices: tuple[int, ...] | None = None) -> MixedRadixDecomposer:
    """Return a stateful iterator over ``value``'s mixed-radix digits."""

    return MixedRadixDecomposer(value, indices=indices)
