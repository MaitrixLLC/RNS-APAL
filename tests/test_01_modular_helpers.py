"""Tests for ModularHelpers in hierarchy order."""

import unittest

from rns_pypal import (
    are_coprime,
    divide_residue_by_coprime_factor,
    is_pairwise_coprime,
    multiplicative_inverse,
)


class TestModularHelpers(unittest.TestCase):
    def test_coprime_helpers(self):
        self.assertTrue(are_coprime(12, 25))
        self.assertFalse(are_coprime(12, 18))
        self.assertTrue(is_pairwise_coprime([3, 25, 7]))
        self.assertFalse(is_pairwise_coprime([3, 9, 5]))

    def test_multiplicative_inverse_identity(self):
        for value, modulus in [(2, 5), (5, 7), (11, 13), (10, 27)]:
            inverse = multiplicative_inverse(value, modulus)

            self.assertEqual((value * inverse) % modulus, 1)

    def test_divide_residue_by_coprime_factor(self):
        residue = 6
        divisor = 5
        modulus = 7

        quotient = divide_residue_by_coprime_factor(residue, divisor, modulus)

        self.assertEqual(quotient, 4)
        self.assertEqual((quotient * divisor) % modulus, residue)

    def test_mixed_radix_style_channel_update_uses_inverse_multiplication(self):
        value = 26
        consumed_modulus = 5
        remaining_modulus = 7
        consumed_digit = value % consumed_modulus
        remaining_residue = value % remaining_modulus

        reduced_residue = divide_residue_by_coprime_factor(
            remaining_residue - consumed_digit,
            consumed_modulus,
            remaining_modulus,
        )

        self.assertEqual(consumed_digit, 1)
        self.assertEqual(reduced_residue, ((value - consumed_digit) // consumed_modulus) % 7)

    def test_multiplicative_inverse_rejects_non_coprime_inputs(self):
        with self.assertRaisesRegex(ValueError, "no multiplicative inverse"):
            multiplicative_inverse(6, 9)
