"""Utility functions for RNS-PYPAL."""

from collections.abc import Sequence


def greatest_common_divisor(a: int, b: int) -> int:
    while b:
        a, b = b, a % b
    return abs(a)


def are_coprime(left: int, right: int) -> bool:
    """Return whether two integers share no common factor except one."""

    return greatest_common_divisor(left, right) == 1


def extended_gcd(a: int, b: int) -> tuple[int, int, int]:
    """Return ``(gcd, x, y)`` such that ``a * x + b * y == gcd``."""

    if b == 0:
        return abs(a), 1 if a >= 0 else -1, 0

    g, x, y = extended_gcd(b, a % b)
    return g, y, x - (a // b) * y


def multiplicative_inverse(value: int, modulus: int) -> int:
    """Return ``value``'s multiplicative inverse modulo ``modulus``.

    The inverse is the digit-level primitive needed when an RNS channel is
    divided by a value that is coprime to that channel's modulus. Mixed-radix
    conversion uses this repeatedly when it consumes one radix and updates the
    remaining residue channels.
    """

    if type(modulus) is not int or modulus <= 1:
        raise ValueError("modulus must be an integer greater than one")

    g, x, _ = extended_gcd(value, modulus)
    if g != 1:
        raise ValueError(f"{value} has no multiplicative inverse modulo {modulus}")
    return x % modulus


def divide_residue_by_coprime_factor(residue: int, divisor: int, modulus: int) -> int:
    """Divide one residue channel by a factor coprime to that channel's modulus.

    The returned value ``quotient`` satisfies:

    ``(quotient * divisor) % modulus == residue % modulus``

    This is the operation mixed-radix conversion needs after subtracting the
    consumed mixed-radix digit from every remaining residue channel.
    """

    inverse = multiplicative_inverse(divisor, modulus)
    return (residue * inverse) % modulus


def count_base_power_factor(value: int, base: int) -> int:
    """Return how many times ``base`` divides ``value`` exactly."""

    if type(value) is not int or value <= 0:
        raise ValueError("value must be a positive integer")
    if type(base) is not int or base <= 1:
        raise ValueError("base must be an integer greater than one")

    count = 0
    while value % base == 0:
        count += 1
        value //= base
    return count


def is_exact_base_power(value: int, base: int) -> bool:
    """Return whether ``value`` is exactly ``base ** k`` for some ``k >= 0``."""

    if type(value) is not int or value <= 0:
        raise ValueError("value must be a positive integer")
    if type(base) is not int or base <= 1:
        raise ValueError("base must be an integer greater than one")

    while value % base == 0:
        value //= base
    return value == 1


def is_pairwise_coprime(moduli: Sequence[int]) -> bool:
    n = len(moduli)
    for i in range(n):
        for j in range(i + 1, n):
            if not are_coprime(moduli[i], moduli[j]):
                return False
    return True


def crt(remainders: Sequence[int], moduli: Sequence[int]) -> int:
    if len(remainders) != len(moduli):
        raise ValueError("remainders and moduli must have the same length")
    if not is_pairwise_coprime(moduli):
        raise ValueError("moduli must be pairwise coprime for this CRT implementation")

    x = 0
    M = 1
    for modulus in moduli:
        M *= modulus

    for remainder, modulus in zip(remainders, moduli):
        m_i = M // modulus
        inv = multiplicative_inverse(m_i, modulus)
        x = (x + remainder * inv * m_i) % M

    return x


def power(base: int, exp: int) -> int:
    return base ** exp


# Compatibility aliases for early code and mathematical shorthand.
gcd = greatest_common_divisor
modinv = multiplicative_inverse
