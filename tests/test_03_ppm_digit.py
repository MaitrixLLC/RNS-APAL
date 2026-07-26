"""Tests for PPMDigit in hierarchy order."""

import unittest

from rns_pypal import PPMDigit


class TestPPMDigit(unittest.TestCase):
    def test_normalized_digit_arithmetic(self):
        digit = PPMDigit(index=0, digit=10, modulus=3, power=2)

        self.assertEqual(digit.digit, 1)
        self.assertEqual(digit.full_modulus, 9)
        self.assertTrue(digit.is_normalized)

        digit.add(10)
        self.assertEqual(digit.digit, 2)
        digit.sub(5)
        self.assertEqual(digit.digit, 6)
        digit.mult(4)
        self.assertEqual(digit.digit, 6)

    def test_skip_digit_invalidates_residue_channel(self):
        digit = PPMDigit(index=0, digit=10, modulus=3, power=2)

        digit.skip_digit()

        self.assertTrue(digit.skip)
        self.assertEqual(digit.power_valid, 0)
        self.assertEqual(digit.digit, 0)
        with self.assertRaisesRegex(ValueError, "skipped digit"):
            _ = digit.current_modulus

    def test_mod_div_consumes_matching_base_power(self):
        digit = PPMDigit(index=0, digit=10, modulus=2, power=4)

        digit.mod_div(2)

        self.assertEqual(digit.digit, 5)
        self.assertEqual(digit.power_valid, 3)
        self.assertEqual(digit.current_modulus, 8)

    def test_mod_div_uses_inverse_for_coprime_channel(self):
        digit = PPMDigit(index=1, digit=6, modulus=3, power=2)

        digit.mod_div(2)

        self.assertEqual(digit.digit, 3)
        self.assertEqual(digit.power_valid, 2)
        self.assertEqual(digit.current_modulus, 9)

    def test_digit_exports_plain_record(self):
        digit = PPMDigit(index=1, digit=17, modulus=3, power=2)

        self.assertEqual(
            digit.to_record(),
            {
                "index": 1,
                "digit": 8,
                "modulus": 3,
                "power": 2,
                "power_valid": 2,
                "normal_power": 2,
                "skip": False,
                "full_modulus": 9,
                "current_modulus": 9,
            },
        )

        digit.skip_digit()
        self.assertIsNone(digit.to_record()["current_modulus"])
