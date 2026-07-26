"""Tests for SPPM in hierarchy order."""

import io
import unittest
import warnings

from rns_pypal import (
    DigitRole,
    NEGATIVE,
    POSITIVE,
    RNSCriticalError,
    RNSDiagnosticLevel,
    RNSDiagnosticWarning,
    RNSRangeError,
    RNSNumberSystem,
    SIGN_INVALID,
    SIGN_VALID,
    SPPM,
)


class TestSPPM(unittest.TestCase):
    def setUp(self):
        self.system = RNSNumberSystem(
            moduli=[2, 3],
            powers=[4, 1],
            name="signed-48",
        )

    def test_signed_assignment_uses_complement_encoding(self):
        value = SPPM(-1, system=self.system)

        self.assertEqual(value.format_native(), "15 2")
        self.assertEqual(value.format_value(), "-1")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_signed_string_assignment_supports_decimal_and_hex(self):
        decimal_value = SPPM("-15", system=self.system)
        hex_value = SPPM("-0xf", system=self.system)

        self.assertEqual(decimal_value.format_value(), "-15")
        self.assertEqual(hex_value.format_value(), "-15")
        self.assertEqual(decimal_value.format_native(), hex_value.format_native())

    def test_signed_range_helpers(self):
        self.assertEqual(SPPM.signed_range(self.system), (-24, 23))
        self.assertEqual(SPPM.format_signed_range(self.system), "-24..23")

        output = io.StringIO()
        SPPM.print_signed_range(self.system, file=output)
        self.assertEqual(output.getvalue(), "-24..23\n")

    def test_sppm_export_includes_sign_metadata(self):
        value = SPPM(-7, system=self.system)

        exported = value.to_dict()

        self.assertEqual(exported["class"], "SPPM")
        self.assertEqual(exported["residues"], (9, 2))
        self.assertEqual(exported["sign_flag"], NEGATIVE)
        self.assertEqual(exported["sign_valid"], SIGN_VALID)
        self.assertEqual(exported["residue_sign"], NEGATIVE)
        self.assertEqual(exported["value"], "-7")

    def test_ordinary_assignment_wraps_without_range_check(self):
        value = SPPM(25, system=self.system)

        self.assertEqual(value.format_value(), "-23")

    def test_checked_assignment_accepts_in_range_external_value(self):
        value = SPPM(0, system=self.system)

        value.assign_checked("-0xf")

        self.assertEqual(value.format_value(), "-15")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_checked_assignment_can_warn_and_continue(self):
        value = SPPM(0, system=self.system)

        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            value.assign_checked(25, on_error=RNSDiagnosticLevel.WARNING)

        self.assertEqual(value.format_value(), "-23")
        self.assertEqual(len(caught), 1)
        self.assertTrue(issubclass(caught[0].category, RNSDiagnosticWarning))
        self.assertIn("outside signed range", str(caught[0].message))

    def test_checked_assignment_can_raise_recoverable_error(self):
        value = SPPM(0, system=self.system)

        with self.assertRaisesRegex(RNSRangeError, "outside signed range"):
            value.assign_checked(25, on_error=RNSDiagnosticLevel.ERROR)

    def test_checked_assignment_defaults_to_critical_error(self):
        value = SPPM(0, system=self.system)

        with self.assertRaisesRegex(RNSCriticalError, "outside signed range"):
            value.assign_checked(25)

    def test_calc_sign_uses_complement_range(self):
        positive = SPPM(23, system=self.system)
        negative = SPPM(-24, system=self.system)

        positive.sign_valid = SIGN_INVALID
        negative.sign_valid = SIGN_INVALID

        self.assertEqual(positive.calc_set_sign(), POSITIVE)
        self.assertEqual(negative.calc_set_sign(), NEGATIVE)
        self.assertEqual(positive.format_value(), "23")
        self.assertEqual(negative.format_value(), "-24")

    def test_sign_mismatch_detects_bad_cached_sign(self):
        value = SPPM(-3, system=self.system)

        value.sign_flag = POSITIVE
        value.sign_valid = SIGN_VALID

        self.assertTrue(value.sign_mismatch())
        self.assertEqual(value.format_value(), "-3")

    def test_calc_set_sign_raises_on_valid_cached_sign_mismatch(self):
        value = SPPM(-3, system=self.system)

        value.sign_flag = POSITIVE
        value.sign_valid = SIGN_VALID

        with self.assertRaisesRegex(RNSCriticalError, "valid sign flag disagrees"):
            value.calc_set_sign()

    def test_signed_compare_uses_sign_then_complement_order(self):
        self.assertEqual(SPPM(2, system=self.system).compare(SPPM(-1, system=self.system)), 1)
        self.assertEqual(SPPM(-1, system=self.system).compare(SPPM(2, system=self.system)), 0)
        self.assertEqual(SPPM(-1, system=self.system).compare(SPPM(-2, system=self.system)), 1)

    def test_add_same_sign_preserves_valid_sign(self):
        positive = SPPM(4, system=self.system)
        positive.add(SPPM(5, system=self.system))

        self.assertEqual(positive.format_value(), "9")
        self.assertEqual(positive.sign_flag, POSITIVE)
        self.assertEqual(positive.sign_valid, SIGN_VALID)

        negative = SPPM(-4, system=self.system)
        negative.add(SPPM(-5, system=self.system))

        self.assertEqual(negative.format_value(), "-9")
        self.assertEqual(negative.sign_flag, NEGATIVE)
        self.assertEqual(negative.sign_valid, SIGN_VALID)

    def test_add_mixed_sign_invalidates_sign_without_changing_value(self):
        value = SPPM(5, system=self.system)

        value.add(SPPM(-3, system=self.system))

        self.assertEqual(value.format_value(), "2")
        self.assertEqual(value.sign_valid, SIGN_INVALID)
        self.assertEqual(value.calc_set_sign(), POSITIVE)

    def test_subtraction_sign_disposition(self):
        value = SPPM(5, system=self.system)
        value.sub(SPPM(-3, system=self.system))

        self.assertEqual(value.format_value(), "8")
        self.assertEqual(value.sign_flag, POSITIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

        value.sub(SPPM(10, system=self.system))

        self.assertEqual(value.format_value(), "-2")
        self.assertEqual(value.sign_valid, SIGN_INVALID)

    def test_multiplication_computes_sign_when_inputs_valid(self):
        value = SPPM(-4, system=self.system)

        value.mult(SPPM(5, system=self.system))

        self.assertEqual(value.format_value(), "-20")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_multiplication_invalidates_sign_when_input_invalid(self):
        value = SPPM(-4, system=self.system)
        other = SPPM(5, system=self.system)
        other.sign_valid = SIGN_INVALID

        value.mult(other)

        self.assertEqual(value.format_value(), "-20")
        self.assertEqual(value.sign_valid, SIGN_INVALID)

    def test_negate_and_abs_use_complement_encoding(self):
        value = SPPM(7, system=self.system)
        value.negate()

        self.assertEqual(value.format_value(), "-7")
        self.assertEqual(value.sign_flag, NEGATIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

        value.abs()

        self.assertEqual(value.format_value(), "7")
        self.assertEqual(value.sign_flag, POSITIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_zero_is_canonical_positive_valid(self):
        value = SPPM(-1, system=self.system)
        value.add(SPPM(1, system=self.system))

        self.assertEqual(value.format_value(), "0")
        self.assertEqual(value.sign_flag, POSITIVE)
        self.assertEqual(value.sign_valid, SIGN_VALID)

    def test_sppm_rejects_auxiliary_digit_systems_for_now(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[4, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )

        with self.assertRaisesRegex(RNSCriticalError, "auxiliary digit"):
            SPPM(1, system=system)
