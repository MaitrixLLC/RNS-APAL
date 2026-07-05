"""Utility functions for RNS-PYPAL."""

from typing import Sequence, Tuple


def gcd(a: int, b: int) -> int:
    while b:
        a, b = b, a % b
    return abs(a)


def extended_gcd(a: int, b: int) -> Tuple[int, int, int]:
    if b == 0:
        return abs(a), 1 if a >= 0 else -1, 0
    else:
        g, x, y = extended_gcd(b, a % b)
        return g, y, x - (a // b) * y


def modinv(a: int, m: int) -> int:
    g, x, _ = extended_gcd(a, m)
    if g != 1:
        raise ValueError("modular inverse does not exist")
    return x % m


def is_pairwise_coprime(moduli: Sequence[int]) -> bool:
    n = len(moduli)
    for i in range(n):
        for j in range(i + 1, n):
            if gcd(moduli[i], moduli[j]) != 1:
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
        inv = modinv(m_i, modulus)
        x = (x + remainder * inv * m_i) % M

    return x


def power(base: int, exp: int) -> int:
    return base ** exp
