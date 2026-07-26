"""Tests for RNSNumberSystem in hierarchy order."""

import unittest

from rns_pypal import DigitRole, RNSNumberSystem


class TestRNSNumberSystem(unittest.TestCase):
    def test_normalized_geometry(self):
        system = RNSNumberSystem(
            moduli=[3, 5, 7],
            powers=[1, 2, 1],
            fractional_digits=0,
            name="test-system",
        )

        self.assertEqual(system.moduli, (3, 5, 7))
        self.assertEqual(system.full_moduli, (3, 25, 7))
        self.assertEqual(system.num_digits, 3)
        self.assertEqual(system.dynamic_range, 525)
        self.assertTrue(system.power_based)

    def test_rejects_non_coprime_full_moduli(self):
        with self.assertRaisesRegex(ValueError, "pairwise coprime"):
            RNSNumberSystem(moduli=[2, 4], powers=[1, 1])

    def test_name_does_not_affect_compatibility(self):
        left = RNSNumberSystem(moduli=[3, 5], powers=[1, 1], name="left")
        right = RNSNumberSystem(moduli=[3, 5], powers=[1, 1], name="right")

        self.assertTrue(left.is_compatible(right))

    def test_digit_roles_define_value_and_auxiliary_indices(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[
                DigitRole.VALUE,
                "value",
                DigitRole.OVERFLOW_CHECK,
            ],
        )

        self.assertEqual(system.full_moduli, (4, 3, 5))
        self.assertEqual(system.value_full_moduli, (4, 3))
        self.assertEqual(system.dynamic_range, 12)
        self.assertEqual(system.value_indices, (0, 1))
        self.assertEqual(system.auxiliary_indices, (2,))
        self.assertEqual(system.overflow_check_indices, (2,))
        self.assertEqual(system.redundant_indices, ())
        self.assertEqual(system.arithmetic_indices, (0, 1, 2))
        self.assertEqual(system.comparison_indices, (0, 1))
        self.assertEqual(system.conversion_indices, (0, 1))

    def test_rejects_auxiliary_only_system(self):
        with self.assertRaisesRegex(ValueError, "at least one value digit"):
            RNSNumberSystem(
                moduli=[3, 5],
                powers=[1, 1],
                digit_roles=[DigitRole.REDUNDANT, DigitRole.OVERFLOW_CHECK],
            )

    def test_rejects_auxiliary_digits_before_value_digits(self):
        with self.assertRaisesRegex(ValueError, "auxiliary digits must follow"):
            RNSNumberSystem(
                moduli=[3, 5, 7],
                powers=[1, 1, 1],
                digit_roles=[DigitRole.VALUE, DigitRole.REDUNDANT, DigitRole.VALUE],
            )

    def test_digit_roles_affect_compatibility(self):
        left = RNSNumberSystem(moduli=[3, 5], powers=[1, 1])
        right = RNSNumberSystem(
            moduli=[3, 5],
            powers=[1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.REDUNDANT],
        )

        self.assertFalse(left.is_compatible(right))

    def test_fractional_and_whole_indices_are_explicit(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[3, 2, 1, 1],
            fractional_digits=2,
        )

        self.assertEqual(system.fractional_indices, (0, 1))
        self.assertEqual(system.whole_indices, (2, 3))
        self.assertEqual(system.unit_range, 72)
