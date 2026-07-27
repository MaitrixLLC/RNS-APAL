"""Tests for SPMF in hierarchy order."""

import unittest

from rns_pypal import (
    NEGATIVE,
    POSITIVE,
    RNSCriticalError,
    RNSEffectiveFormatError,
    RNSNumberSystem,
    RNSSystemCompatibilityError,
    SIGN_INVALID,
    SIGN_VALID,
    SPMF,
    SPPM,
)


class TestSPMF(unittest.TestCase):
    def setUp(self):
        self.system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[4, 2, 1],
            fractional_digits=2,
            name="fixed-point-demo",
        )

    def test_spmf_derives_from_sppm_and_records_fraction_metadata(self):
        value = SPMF(-7, system=self.system)

        self.assertIsInstance(value, SPPM)
        self.assertEqual(value.num_fract_digits, 2)
        self.assertEqual(value.normal_fract_digits, 2)
        self.assertEqual(value.fractional_indices, (0, 1))
        self.assertEqual(value.whole_indices, (2,))
        self.assertEqual(value.normal_fractional_indices, (0, 1))
        self.assertEqual(value.normal_whole_indices, (2,))
        self.assertEqual(value.unit_range, 144)
        self.assertEqual(value.normal_unit_range, 144)
        self.assertFalse(value.sliding_point_active)
        self.assertEqual(value.format_value(), "-7")

    def test_spmf_rejects_system_without_fractional_digits(self):
        integer_system = RNSNumberSystem(moduli=[2, 3], powers=[4, 1])

        with self.assertRaisesRegex(ValueError, "at least one fractional"):
            SPMF(0, system=integer_system)

    def test_spmf_copy_preserves_fraction_metadata(self):
        value = SPMF(-3, system=self.system)
        value.set_fraction_position(1)

        copied = value.copy()

        self.assertIsInstance(copied, SPMF)
        self.assertEqual(copied.format_native(), value.format_native())
        self.assertEqual(copied.num_fract_digits, value.num_fract_digits)
        self.assertEqual(copied.normal_fract_digits, value.normal_fract_digits)

    def test_spmf_export_includes_fixed_point_metadata(self):
        value = SPMF(5, system=self.system)
        value.set_fraction_position(1)

        exported = value.to_dict()

        self.assertEqual(exported["class"], "SPMF")
        self.assertEqual(exported["num_fract_digits"], 1)
        self.assertEqual(exported["normal_fract_digits"], 2)
        self.assertEqual(exported["fractional_indices"], (0,))
        self.assertEqual(exported["whole_indices"], (1, 2))
        self.assertEqual(exported["normal_fractional_indices"], (0, 1))
        self.assertEqual(exported["normal_whole_indices"], (2,))
        self.assertEqual(exported["unit_range"], 16)
        self.assertEqual(exported["normal_unit_range"], 144)
        self.assertTrue(exported["sliding_point_active"])

    def test_spmf_sliding_point_can_be_reset_to_normal(self):
        value = SPMF(5, system=self.system)

        value.set_fraction_position(1)
        self.assertTrue(value.sliding_point_active)
        self.assertEqual(value.fractional_indices, (0,))
        self.assertEqual(value.whole_indices, (1, 2))

        value.reset_fraction_position()

        self.assertFalse(value.sliding_point_active)
        self.assertEqual(value.fractional_indices, (0, 1))
        self.assertEqual(value.whole_indices, (2,))

    def test_spmf_external_assignment_resets_fraction_position(self):
        value = SPMF(5, system=self.system)
        value.set_fraction_position(1)

        value.assign(-3)

        self.assertEqual(value.format_value(), "-3")
        self.assertFalse(value.sliding_point_active)
        self.assertEqual(value.num_fract_digits, value.normal_fract_digits)

    def test_spmf_fraction_position_compatibility_check(self):
        left = SPMF(1, system=self.system)
        right = SPMF(2, system=self.system)

        left.ensure_fraction_position_compatible(right)

        right.set_fraction_position(1)

        self.assertFalse(left.has_same_fraction_position(right))
        with self.assertRaisesRegex(RNSCriticalError, "fraction positions are incompatible"):
            left.ensure_fraction_position_compatible(right)

    def test_spmf_normal_fraction_position_check(self):
        value = SPMF(1, system=self.system)

        value.ensure_normal_fraction_position()
        value.set_fraction_position(1)

        with self.assertRaisesRegex(RNSCriticalError, "not in normal fraction position"):
            value.ensure_normal_fraction_position()

    def test_spmf_inherits_system_compatibility_error_without_mutation(self):
        other_system = RNSNumberSystem(
            moduli=[2, 3, 7],
            powers=[4, 2, 1],
            fractional_digits=2,
        )
        value = SPMF(-7, system=self.system)
        before = value.to_dict()

        with self.assertRaisesRegex(
            RNSSystemCompatibilityError,
            "incompatible normalized RNS systems",
        ):
            value.add(SPMF(3, system=other_system))

        self.assertEqual(value.to_dict(), before)

    def test_spmf_inherits_effective_format_error_without_mutation(self):
        value = SPMF(-7, system=self.system)
        derived = SPMF(6, system=self.system)
        derived.mod_div(2)
        before = value.to_dict()

        with self.assertRaisesRegex(
            RNSEffectiveFormatError,
            "incompatible effective RNS formats",
        ):
            value.mult(derived)

        self.assertEqual(value.to_dict(), before)

    def test_spmf_assign_scaled_integer_and_format_parts(self):
        value = SPMF(0, system=self.system)

        value.assign_scaled_integer(216)

        self.assertEqual(value.to_scaled_integer(), 216)
        self.assertEqual(value.fixed_parts()["whole"], 1)
        self.assertEqual(value.fixed_parts()["fractional_numerator"], 72)
        self.assertEqual(value.format_ratio(), "216/144")
        self.assertEqual(value.format_decimal(), "1.5")
        self.assertEqual(value.format_fixed_native(), "8 0 . 1")

    def test_spmf_assign_whole_scales_by_unit_range(self):
        value = SPMF(0, system=self.system)

        value.assign_whole(-2)

        self.assertEqual(value.to_scaled_integer(), -288)
        self.assertEqual(value.format_decimal(), "-2.")
        self.assertEqual(value.fixed_parts()["whole"], 2)
        self.assertEqual(value.fixed_parts()["fractional_numerator"], 0)

    def test_spmf_assign_decimal_uses_trapdoor_io(self):
        value = SPMF(0, system=self.system)

        value.assign_decimal("-1.25")

        self.assertEqual(value.to_scaled_integer(), -180)
        self.assertEqual(value.format_ratio(), "-180/144")
        self.assertEqual(value.format_decimal(), "-1.25")

    def test_spmf_assign_ratio_rounds_to_nearest_scaled_integer(self):
        value = SPMF(0, system=self.system)

        value.assign_ratio(1, 3)

        self.assertEqual(value.to_scaled_integer(), 48)
        self.assertEqual(value.format_ratio(), "48/144")
        self.assertEqual(value.format_decimal(max_fraction_digits=6), "0.333333")

    def test_spmf_unit_helpers_create_one_point_zero(self):
        value = SPMF(0, system=self.system)

        unit_ppm = value.unit_ppm()
        unit_value = value.unit_value()
        value.assign_unit()

        self.assertEqual(unit_ppm.format_value(), "144")
        self.assertEqual(unit_value.to_scaled_integer(), 144)
        self.assertEqual(unit_value.format_decimal(), "1.")
        self.assertEqual(value.to_scaled_integer(), 144)
        self.assertEqual(value.format_decimal(), "1.")

    def test_spmf_decimal_format_always_includes_decimal_point(self):
        zero = SPMF(0, system=self.system)
        one = SPMF(0, system=self.system)
        negative_two = SPMF(0, system=self.system)

        one.assign_whole(1)
        negative_two.assign_whole(-2)

        self.assertEqual(zero.format_decimal(), "0.")
        self.assertEqual(one.format_decimal(), "1.")
        self.assertEqual(negative_two.format_decimal(), "-2.")

    def test_spmf_same_format_add_sub_and_compare_delegate_to_sppm(self):
        left = SPMF(0, system=self.system)
        right = SPMF(0, system=self.system)
        left.assign_decimal("1.25")
        right.assign_decimal("0.5")

        left.add(right)

        self.assertEqual(left.format_decimal(), "1.75")
        self.assertEqual(left.num_fract_digits, 2)
        self.assertEqual(left.compare(right), 1)

        left.sub(right)

        self.assertEqual(left.format_decimal(), "1.25")
        self.assertEqual(left.compare(right), 1)

    def test_spmf_adds_and_subtracts_whole_integer_operands(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[5, 3, 2, 1],
            fractional_digits=2,
        )
        value = SPMF(0, system=system)
        value.assign_decimal("1.25")

        value.add_whole(2)

        self.assertEqual(value.format_decimal(), "3.25")

        value.sub_whole("1")

        self.assertEqual(value.format_decimal(), "2.25")

        value.add(-3)

        self.assertEqual(value.format_decimal(), "-0.75")
        self.assertEqual(value.sign_valid, SIGN_INVALID)
        self.assertEqual(value.calc_set_sign(), NEGATIVE)

    def test_spmf_adds_whole_sppm_operand(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[5, 3, 2, 1],
            fractional_digits=2,
        )
        value = SPMF(0, system=system)
        whole = SPPM(-2, system=system)
        value.assign_decimal("3.5")

        value.add(whole)

        self.assertEqual(value.format_decimal(), "1.5")

    def test_spmf_multiplies_integer_operands_without_fractional_renormalization(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[5, 3, 2, 1],
            fractional_digits=2,
        )
        value = SPMF(0, system=system)
        value.assign_decimal("-1.25")

        value.mult_integer(3)

        self.assertEqual(value.format_decimal(), "-3.75")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

        value.mult("-2")

        self.assertEqual(value.format_decimal(), "7.5")
        self.assertEqual(value.sign_flag, POSITIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_spmf_multiplies_sppm_integer_operand(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[5, 3, 2, 1],
            fractional_digits=2,
        )
        value = SPMF(0, system=system)
        integer = SPPM(-3, system=system)
        value.assign_decimal("0.5")

        value.mult(integer)

        self.assertEqual(value.format_decimal(), "-1.5")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_spmf_mult_std_multiplies_same_format_fractional_values(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7, 11],
            powers=[4, 2, 2, 1, 1],
            fractional_digits=2,
        )
        left = SPMF(0, system=system)
        right = SPMF(0, system=system)
        left.assign_decimal("1.5")
        right.assign_decimal("0.5")

        left.mult_std(right)

        self.assertEqual(left.to_scaled_integer(), 108)
        self.assertEqual(left.format_decimal(), "0.75")
        self.assertEqual(left.sign_flag, POSITIVE)
        self.assertEqual(left.sign_valid, SIGN_VALID)

    def test_spmf_mult_std_generates_signed_result(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7, 11],
            powers=[4, 2, 2, 1, 1],
            fractional_digits=2,
        )
        left = SPMF(0, system=system)
        right = SPMF(0, system=system)
        left.assign_decimal("-1.5")
        right.assign_decimal("0.5")

        left.mult(right)

        self.assertEqual(left.to_scaled_integer(), -108)
        self.assertEqual(left.format_decimal(), "-0.75")
        self.assertEqual(left.sign_flag, NEGATIVE)
        self.assertEqual(left.sign_valid, SIGN_VALID)

    def test_spmf_mult_std_rounds_half_threshold_by_sign(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7, 11],
            powers=[4, 2, 2, 1, 1],
            fractional_digits=2,
        )
        positive = SPMF(0, system=system)
        positive_half = SPMF(0, system=system)
        negative = SPMF(0, system=system)
        negative_half = SPMF(0, system=system)

        positive.assign_scaled_integer(1)
        positive_half.assign_scaled_integer(72)
        negative.assign_scaled_integer(-1)
        negative_half.assign_scaled_integer(72)

        positive.mult_std(positive_half)
        negative.mult_std(negative_half)

        self.assertEqual(positive.to_scaled_integer(), 1)
        self.assertEqual(negative.to_scaled_integer(), -1)

    def test_spmf_arithmetic_rejects_unaligned_fraction_positions(self):
        left = SPMF(0, system=self.system)
        right = SPMF(0, system=self.system)
        right.set_fraction_position(1)

        with self.assertRaisesRegex(RNSCriticalError, "fraction positions are incompatible"):
            left.add(right)
        with self.assertRaisesRegex(RNSCriticalError, "fraction positions are incompatible"):
            left.sub(right)
        with self.assertRaisesRegex(RNSCriticalError, "fraction positions are incompatible"):
            left.compare(right)
        with self.assertRaisesRegex(RNSCriticalError, "fraction positions are incompatible"):
            left.mult(right)

    def test_spmf_arithmetic_rejects_invalid_mixed_type_operands(self):
        value = SPMF(0, system=self.system)

        with self.assertRaisesRegex(TypeError, "integer operand"):
            value.add(object())
        with self.assertRaisesRegex(TypeError, "integer operand"):
            value.sub(object())
        with self.assertRaisesRegex(TypeError, "requires another SPMF"):
            value.compare(1)
        with self.assertRaisesRegex(TypeError, "integer operand"):
            value.mult(object())
