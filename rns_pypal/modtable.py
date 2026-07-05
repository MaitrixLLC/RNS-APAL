"""Normalized residue-number-system definitions."""

from __future__ import annotations

from dataclasses import dataclass
from math import prod
from typing import Iterable

from .utils import is_pairwise_coprime


@dataclass(frozen=True, slots=True)
class RNSNumberSystem:
    """The immutable geometry of a normalized RNS number system.

    ``moduli[i]`` is the prime/base modulus for digit ``i`` and
    ``powers[i]`` is its normalized power.  The effective full modulus of
    that digit is therefore ``moduli[i] ** powers[i]``.
    """

    moduli: tuple[int, ...]
    powers: tuple[int, ...]
    fractional_digits: int = 0
    name: str | None = None

    def __init__(
        self,
        moduli: Iterable[int],
        powers: Iterable[int],
        fractional_digits: int = 0,
        name: str | None = None,
    ) -> None:
        object.__setattr__(self, "moduli", tuple(moduli))
        object.__setattr__(self, "powers", tuple(powers))
        object.__setattr__(self, "fractional_digits", fractional_digits)
        object.__setattr__(self, "name", name)
        self._validate()

    def _validate(self) -> None:
        if not self.moduli:
            raise ValueError("an RNS system must contain at least one modulus")
        if len(self.moduli) != len(self.powers):
            raise ValueError("moduli and powers must have equal lengths")
        if any(type(modulus) is not int or modulus <= 1 for modulus in self.moduli):
            raise ValueError("every base modulus must be an integer greater than one")
        if any(type(power) is not int or power <= 0 for power in self.powers):
            raise ValueError("every normalized power must be a positive integer")
        if type(self.fractional_digits) is not int or not 0 <= self.fractional_digits <= self.num_digits:
            raise ValueError("fractional_digits must be between zero and num_digits")
        if self.name is not None and not isinstance(self.name, str):
            raise TypeError("name must be a string or None")

        if not is_pairwise_coprime(self.full_moduli):
            raise ValueError("full digit moduli must be pairwise coprime")

    @property
    def num_digits(self) -> int:
        return len(self.moduli)

    @property
    def num_frac_digits(self) -> int:
        """Compatibility spelling for the original C++ terminology."""

        return self.fractional_digits

    @property
    def modulus(self) -> tuple[int, ...]:
        """Compatibility alias for early RNS-PYPAL code."""

        return self.moduli

    @property
    def full_moduli(self) -> tuple[int, ...]:
        return tuple(base**power for base, power in zip(self.moduli, self.powers))

    @property
    def dynamic_range(self) -> int:
        return prod(self.full_moduli)

    @property
    def unit_range(self) -> int:
        return prod(self.full_moduli[: self.fractional_digits])

    @property
    def power_based(self) -> bool:
        return any(power > 1 for power in self.powers)

    def full_modulus(self, index: int) -> int:
        return self.moduli[index] ** self.powers[index]

    def is_compatible(self, other: object) -> bool:
        return isinstance(other, RNSNumberSystem) and (
            self.moduli,
            self.powers,
            self.fractional_digits,
        ) == (
            other.moduli,
            other.powers,
            other.fractional_digits,
        )
