import io
import unittest
import warnings

from rns_pypal import (
    DigitRole,
    NEGATIVE,
    PPM,
    PPMDigit,
    POSITIVE,
    SIGN_INVALID,
    SIGN_VALID,
    SPPM,
    MRN,
    MixedRadixDecomposer,
    RNSCriticalError,
    RNSDiagnosticLevel,
    RNSDiagnosticWarning,
    RNSRangeError,
    RNSNumberSystem,
    are_coprime,
    divide_residue_by_coprime_factor,
    iter_mixed_radix_digits,
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


class TestMixedRadix(unittest.TestCase):
    def setUp(self):
        self.system = RNSNumberSystem(
            moduli=[5, 7, 11],
            powers=[1, 1, 1],
            fractional_digits=0,
            name="mixed-radix-demo",
        )

    def test_streamed_digits_follow_rns_index_order(self):
        value = PPM(384, system=self.system)

        digits = list(iter_mixed_radix_digits(value))

        self.assertEqual([digit.index for digit in digits], [0, 1, 2])
        self.assertEqual([digit.radix for digit in digits], [5, 7, 11])
        self.assertEqual([digit.digit for digit in digits], [4, 6, 10])

    def test_streaming_decomposer_invalidates_consumed_digit(self):
        value = PPM(26, system=self.system)
        decomposer = MixedRadixDecomposer(value)

        first = decomposer.step()

        self.assertEqual(first.digit, 1)
        self.assertEqual(first.radix, 5)
        self.assertEqual(decomposer.working.format_native(), "* 5 5")
        self.assertEqual(decomposer.working.format_whdr(), "5 7 11\n- - --\n* 5 5 ")

        second = decomposer.step()

        self.assertEqual(second.digit, 5)
        self.assertEqual(decomposer.working.format_native(), "* * 0")
        self.assertTrue(decomposer.is_complete)
        with self.assertRaises(StopIteration):
            decomposer.step()

    def test_mrn_materializes_streamed_digits(self):
        value = PPM(26, system=self.system)

        mrn = MRN.from_ppm(value)

        self.assertEqual([digit.digit for digit in mrn.digits], [1, 5])
        self.assertEqual(mrn.radices, (5, 7))
        self.assertEqual(mrn.num_digits, 2)
        self.assertEqual(mrn.to_int(), 26)
        self.assertEqual(mrn.format_native(), "5, 1")
        self.assertEqual(mrn.format_native(most_significant_first=False), "1, 5")

    def test_mrn_reconstructs_to_normalized_ppm(self):
        value = PPM(384, system=self.system)
        mrn = MRN.from_ppm(value)

        reconstructed = mrn.to_ppm()

        self.assertEqual(reconstructed.format_value(), "384")
        self.assertEqual([digit.power_valid for digit in reconstructed], [1, 1, 1])
        self.assertEqual(reconstructed.format_native(), value.format_native())


class TestPPM(unittest.TestCase):
    def setUp(self):
        self.system = RNSNumberSystem(
            moduli=[3, 5, 7],
            powers=[1, 1, 1],
            fractional_digits=0,
            name="small-105",
        )

    def assert_residues(self, value: PPM, integer: int) -> None:
        expected = [integer % modulus for modulus in self.system.full_moduli]
        self.assertEqual([digit.digit for digit in value], expected)

    def test_integer_assignment_encodes_each_digit(self):
        value = PPM(10, system=self.system)

        self.assert_residues(value, 10)
        self.assertEqual(value.format_native(), "1 0 3")

    def test_string_assignment_accepts_decimal_and_hex(self):
        decimal_value = PPM("123456", system=self.system)
        hex_value = PPM("0x1e240", system=self.system)

        self.assert_residues(decimal_value, 123456)
        self.assert_residues(hex_value, 123456)
        self.assertEqual(decimal_value.format_native(), hex_value.format_native())

    def test_decimal_string_assignment_does_not_depend_on_python_int_conversion(self):
        text = "1234567890" * 450
        value = PPM(text, system=self.system)

        expected = []
        for modulus in self.system.full_moduli:
            residue = 0
            for character in text:
                residue = (residue * 10 + int(character)) % modulus
            expected.append(residue)

        self.assertEqual([digit.digit for digit in value], expected)

    def test_ordinary_unsigned_assignment_wraps_without_range_check(self):
        value = PPM(105, system=self.system)

        self.assertEqual(value.format_value(), "0")

    def test_checked_unsigned_assignment_accepts_in_range_external_value(self):
        value = PPM(0, system=self.system)

        value.assign_checked("0x68")

        self.assertEqual(value.format_value(), "104")

    def test_checked_unsigned_assignment_can_warn_and_continue(self):
        value = PPM(0, system=self.system)

        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            value.assign_checked(105, on_error=RNSDiagnosticLevel.WARNING)

        self.assertEqual(value.format_value(), "0")
        self.assertEqual(len(caught), 1)
        self.assertTrue(issubclass(caught[0].category, RNSDiagnosticWarning))
        self.assertIn("outside unsigned range", str(caught[0].message))

    def test_checked_unsigned_assignment_can_raise_recoverable_error(self):
        value = PPM(0, system=self.system)

        with self.assertRaisesRegex(RNSRangeError, "outside unsigned range"):
            value.assign_checked(105, on_error=RNSDiagnosticLevel.ERROR)

    def test_checked_unsigned_assignment_defaults_to_critical_error(self):
        value = PPM(0, system=self.system)

        with self.assertRaisesRegex(RNSCriticalError, "outside unsigned range"):
            value.assign_checked("1234567890" * 450)

    def test_copy_assignment_is_independent(self):
        source = PPM(10, system=self.system)
        destination = PPM(0, system=self.system)

        destination.assign(source)
        source.add(1)

        self.assert_residues(destination, 10)
        self.assert_residues(source, 11)

    def test_constructor_can_copy_ppm_without_repeating_system(self):
        source = PPM(10, system=self.system)
        destination = PPM(source)

        source.add(1)

        self.assert_residues(destination, 10)
        self.assert_residues(source, 11)

    def test_assign_pm_promotes_source_valid_powers_to_derived_format(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 1])
        source = PPM(42, system=system)
        source.mod_div(2)
        destination = PPM(0, system=system)

        destination.assign_pm(source)

        self.assertEqual(destination.format_value(), "21")
        self.assertEqual([digit.power for digit in destination], [3, 2, 1])
        self.assertEqual([digit.power_valid for digit in destination], [3, 2, 1])
        self.assertEqual([digit.normal_power for digit in destination], [4, 2, 1])

    def test_assign_pm_can_assign_text_into_source_derived_format(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 1])
        source_format = PPM(42, system=system)
        source_format.mod_div(2)
        destination = PPM(0, system=system)

        destination.assign_pm("0x2a", format_source=source_format)

        self.assertEqual(destination.format_value(), "42")
        self.assertEqual([digit.power for digit in destination], [3, 2, 1])
        self.assertEqual([digit.power_valid for digit in destination], [3, 2, 1])

    def test_digitwise_arithmetic_exhaustively_for_small_system(self):
        for left_value in range(self.system.dynamic_range):
            for right_value in range(self.system.dynamic_range):
                left = PPM(left_value, system=self.system)
                right = PPM(right_value, system=self.system)

                added = left.copy()
                added.add(right)
                self.assert_residues(added, left_value + right_value)

                subtracted = left.copy()
                subtracted.sub(right)
                self.assert_residues(subtracted, left_value - right_value)

                multiplied = left.copy()
                multiplied.mult(right)
                self.assert_residues(multiplied, left_value * right_value)

    def test_scalar_arithmetic_is_reduced_per_digit(self):
        value = PPM(4, system=self.system)
        value.add(108)
        self.assert_residues(value, 112)
        value.sub(9)
        self.assert_residues(value, 103)
        value.mult(12)
        self.assert_residues(value, 1236)

    def test_subtraction_wraps_in_the_unsigned_system(self):
        value = PPM(2, system=self.system)
        value.sub(5)

        self.assert_residues(value, -3)
        self.assertEqual(value.format_native(), "0 2 4")

    def test_zero_and_equality(self):
        self.assertTrue(PPM(0, system=self.system).is_zero())
        self.assertTrue(PPM(105, system=self.system).is_zero())
        self.assertTrue(PPM(10, system=self.system).is_equal(PPM(115, system=self.system)))
        self.assertFalse(PPM(10, system=self.system).is_equal(PPM(11, system=self.system)))

    def test_rejects_incompatible_systems(self):
        other_system = RNSNumberSystem(moduli=[3, 5, 11], powers=[1, 1, 1])
        value = PPM(1, system=self.system)

        with self.assertRaisesRegex(ValueError, "incompatible"):
            value.add(PPM(1, system=other_system))

    def test_compare_uses_mixed_radix_ordering(self):
        self.assertEqual(PPM(1, system=self.system).compare(PPM(2, system=self.system)), 0)
        self.assertEqual(PPM(2, system=self.system).compare(PPM(1, system=self.system)), 1)
        self.assertEqual(PPM(10, system=self.system).compare(PPM(10, system=self.system)), 0)

        # Raw residues would suggest 3 < 4 at the first digit, but 3 > 2
        # numerically. This guards against lexicographic residue comparison.
        self.assertEqual(PPM(3, system=self.system).compare(PPM(2, system=self.system)), 1)

    def test_native_format_supports_decimal_binary_and_hex(self):
        value = PPM(10, system=self.system)

        self.assertEqual(value.format_native(10), "1 0 3")
        self.assertEqual(value.format_native(2), "1 0 11")
        self.assertEqual(value.format_native(16), "1 0 3")

    def test_value_format_uses_mixed_radix_fallback_without_two_or_five_moduli(self):
        system = RNSNumberSystem(moduli=[7, 11, 13], powers=[1, 1, 1])
        value = PPM(624, system=system)

        self.assertEqual(value.format_value(10), "624")
        self.assertEqual(value.format_value(16), "270")
        self.assertEqual(value.format_value(16, prefix=True), "0x270")
        self.assertEqual(str(value), "624")
        self.assertEqual(value.format_native(), "1 8 0")

    def test_value_format_supports_values_larger_than_native_machine_words(self):
        system = RNSNumberSystem(
            moduli=[3, 5, 7, 11, 13, 17, 19, 23],
            powers=[10, 8, 6, 5, 4, 4, 4, 4],
        )
        integer = 2**130 + 123_456_789
        value = PPM(integer, system=system)

        self.assertGreater(integer, 2**64)
        self.assertEqual(value.to_int(), integer)
        self.assertEqual(value.format_value(10), str(integer))
        self.assertEqual(value.format_value(16), format(integer, "x"))

    def test_mod_div_reduces_power_based_digit_format(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        value = PPM(42, system=system)

        value.mod_div(2)

        self.assertEqual(value.rn[0].power_valid, 3)
        self.assertEqual(value.rn[0].current_modulus, 8)
        self.assertEqual(value.format_native(), "X|5 3 21")
        self.assertEqual(value.format_whdr(), "8 9 25\n= - --\n5 3 21")
        self.assertEqual(value.format_value(), "21")

    def test_normalize_restores_partial_power_value_to_full_format(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        value = PPM(42, system=system)
        value.mod_div(2)

        value.normalize()

        self.assertEqual(value.format_value(), "21")
        self.assertEqual(value.format_native(), "5 3 21")
        self.assertEqual(value.format_whdr(), "16 9 25\n-- - --\n5  3 21")
        self.assertEqual([digit.power_valid for digit in value], [4, 2, 2])
        self.assertFalse(any(digit.skip for digit in value))

    def test_normalized_copy_leaves_source_derived(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        source = PPM(42, system=system)
        source.mod_div(2)

        normalized = source.normalized_copy()

        self.assertEqual(source.format_native(), "X|5 3 21")
        self.assertEqual(normalized.format_native(), "5 3 21")
        self.assertEqual(normalized.format_value(), "21")

    def test_derived_format_copy_promotes_valid_power_to_current_power(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        source = PPM(42, system=system)
        source.mod_div(2)

        derived = source.derived_format_copy()

        self.assertEqual(source.format_native(), "X|5 3 21")
        self.assertEqual(source.format_whdr(), "8 9 25\n= - --\n5 3 21")
        self.assertEqual(derived.format_native(), "5 3 21")
        self.assertEqual(derived.format_whdr(), "8 9 25\n- - --\n5 3 21")
        self.assertEqual(derived.rn[0].power, 3)
        self.assertEqual(derived.rn[0].power_valid, 3)
        self.assertEqual(derived.rn[0].normal_power, 4)

    def test_extend_to_current_power_restores_partial_value_without_normalizing(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        source = PPM(42, system=system)
        source.mod_div(2)
        derived = source.derived_format_copy()

        derived.extend_to_current_power()

        self.assertEqual(derived.format_value(), "21")
        self.assertEqual(derived.format_native(), "5 3 21")
        self.assertEqual(derived.format_whdr(), "8 9 25\n- - --\n5 3 21")
        self.assertEqual(derived.rn[0].power, 3)
        self.assertEqual(derived.rn[0].power_valid, 3)
        self.assertEqual(derived.rn[0].normal_power, 4)

    def test_normalize_restores_derived_format_to_original_normal_power(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        source = PPM(42, system=system)
        source.mod_div(2)
        derived = source.derived_format_copy()

        derived.normalize()

        self.assertEqual(derived.format_value(), "21")
        self.assertEqual(derived.format_native(), "5 3 21")
        self.assertEqual(derived.format_whdr(), "16 9 25\n-- - --\n5  3 21")
        self.assertEqual(derived.rn[0].power, 4)
        self.assertEqual(derived.rn[0].power_valid, 4)
        self.assertEqual(derived.rn[0].normal_power, 4)

    def test_mod_div_rejects_non_divisible_matching_base_digit_without_mutation(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        value = PPM(43, system=system)

        with self.assertRaisesRegex(RNSCriticalError, "CRITICAL ERROR"):
            value.mod_div(2)

        self.assertEqual(value.format_native(), "11 7 18")
        self.assertEqual([digit.power_valid for digit in value], [4, 2, 2])

    def test_mod_div_critical_error_reports_context(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        value = PPM(43, system=system)

        with self.assertRaises(RNSCriticalError) as caught:
            value.mod_div(2)

        message = str(caught.exception)
        self.assertIn("CRITICAL ERROR", message)
        self.assertIn("location:", message)
        self.assertIn("function: mod_div", message)
        self.assertIn("digit_index: 0", message)
        self.assertIn("digit: 11", message)
        self.assertIn("divisor: 2", message)
        self.assertIn("ppm_native: '11 7 18'", message)
        self.assertIsInstance(caught.exception, ValueError)

    def test_mod_div_can_consume_digit_to_skip_state(self):
        system = RNSNumberSystem(moduli=[2, 3], powers=[1, 2])
        value = PPM(6, system=system)

        value.mod_div(2)

        self.assertTrue(value.rn[0].skip)
        self.assertEqual(value.rn[0].power_valid, 0)
        self.assertEqual(value.format_native(), "* 3")
        self.assertEqual(value.format_whdr(), "2 9\n- -\n* 3")
        self.assertEqual(value.format_value(), "3")

        value.extend_to_current_power()

        self.assertEqual(value.format_native(), "1 3")
        self.assertEqual(value.format_whdr(), "2 9\n- -\n1 3")
        self.assertEqual(value.format_value(), "3")
        self.assertEqual([digit.power_valid for digit in value], [1, 2])
        self.assertFalse(any(digit.skip for digit in value))

    def test_derived_formats_must_match_for_ppm_arithmetic(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 2])
        derived = PPM(42, system=system)
        derived.mod_div(2)
        normalized = PPM(21, system=system)

        with self.assertRaisesRegex(ValueError, "incompatible effective RNS formats"):
            derived.add(normalized)

        other_derived = PPM(10, system=system)
        other_derived.mod_div(2)
        derived.add(other_derived)

        self.assertEqual(derived.format_value(), "26")

    def test_div_std_exact_division(self):
        system = RNSNumberSystem(moduli=[2, 3, 5, 7], powers=[4, 2, 1, 1])
        dividend = PPM(123, system=system)
        divisor = PPM(3, system=system)

        remainder = dividend.div_std(divisor)

        self.assertEqual(dividend.format_value(), "41")
        self.assertTrue(remainder.is_zero())

    def test_div_std_returns_remainder(self):
        system = RNSNumberSystem(moduli=[2, 3, 5, 7], powers=[4, 2, 1, 1])
        dividend = PPM(123, system=system)
        divisor = PPM(10, system=system)

        remainder = dividend.div_std(divisor)

        self.assertEqual(dividend.format_value(), "12")
        self.assertEqual(remainder.format_value(), "3")

    def test_div_std_divides_largest_represented_value(self):
        system = RNSNumberSystem(moduli=[2, 3, 5, 7], powers=[4, 2, 1, 1])
        largest = system.dynamic_range - 1
        dividend = PPM(largest, system=system)
        divisor = PPM(5, system=system)

        remainder = dividend.div_std(divisor)

        self.assertEqual(dividend.format_value(), str(largest // 5))
        self.assertEqual(remainder.format_value(), str(largest % 5))

    def test_div_std_matches_python_oracle_for_small_range(self):
        system = RNSNumberSystem(moduli=[2, 3, 5], powers=[4, 2, 1])

        for dividend_integer in range(0, 80):
            for divisor_integer in range(1, 24):
                dividend = PPM(dividend_integer, system=system)
                divisor = PPM(divisor_integer, system=system)

                remainder = dividend.div_std(divisor)

                self.assertEqual(
                    dividend.format_value(),
                    str(dividend_integer // divisor_integer),
                    (dividend_integer, divisor_integer),
                )
                self.assertEqual(
                    remainder.format_value(),
                    str(dividend_integer % divisor_integer),
                    (dividend_integer, divisor_integer),
                )

    def test_div_std_larger_user_validated_cases(self):
        system = RNSNumberSystem(moduli=[2, 3, 5, 7, 11, 13], powers=[8, 5, 3, 2, 2, 1])
        cases = [
            (123_456, 7),
            (987_654, 123),
            (12_345_678, 1_001),
            (system.dynamic_range - 1, 5),
            (system.dynamic_range - 1, 12_345),
        ]

        for dividend_integer, divisor_integer in cases:
            with self.subTest(dividend=dividend_integer, divisor=divisor_integer):
                dividend = PPM(dividend_integer, system=system)
                divisor = PPM(divisor_integer, system=system)

                remainder = dividend.div_std(divisor)

                self.assertEqual(dividend.format_value(), str(dividend_integer // divisor_integer))
                self.assertEqual(remainder.format_value(), str(dividend_integer % divisor_integer))

    def test_div_std_requires_base_two_modulus(self):
        system = RNSNumberSystem(moduli=[3, 5, 7], powers=[2, 1, 1])
        dividend = PPM(10, system=system)
        divisor = PPM(2, system=system)

        with self.assertRaisesRegex(RNSCriticalError, "base modulus 2"):
            dividend.div_std(divisor)

    def test_with_header_format_maps_to_cpp_print_demo_style(self):
        display_system = RNSNumberSystem(moduli=[2, 3], powers=[4, 3])
        value = PPM(10, system=display_system)

        self.assertEqual(value.format_whdr(), "16 27\n-- --\n10 10")

        output = io.StringIO()
        value.print_whdr(file=output)
        self.assertEqual(output.getvalue(), "16 27\n-- --\n10 10\n")

    def test_auxiliary_digits_do_not_extend_value_range(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )

        high_value = PPM(15, system=system)
        wrapped_value = PPM(3, system=system)

        self.assertEqual(high_value.format_value(), "3")
        self.assertTrue(high_value.is_equal(wrapped_value))
        self.assertNotEqual(high_value, wrapped_value)
        self.assertEqual([digit.digit for digit in high_value], [3, 0, 0])
        self.assertEqual([digit.digit for digit in wrapped_value], [3, 0, 3])

    def test_auxiliary_digits_are_grouped_in_native_prints(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5, 7],
            powers=[2, 1, 1, 1],
            digit_roles=[
                DigitRole.VALUE,
                DigitRole.VALUE,
                DigitRole.REDUNDANT,
                DigitRole.OVERFLOW_CHECK,
            ],
        )
        value = PPM(10, system=system)

        self.assertEqual(value.format_native(), "2 1 (0, 3)")
        self.assertEqual(value.format_whdr(), "4 3 (5 7)\n- - (- -)\n2 1 (0 3)")

    def test_auxiliary_digits_participate_in_basic_arithmetic(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.REDUNDANT],
        )
        value = PPM(10, system=system)

        value.add(5)

        self.assertEqual(value.format_value(), "3")
        self.assertEqual([digit.digit for digit in value], [3, 0, 0])

    def test_overflow_check_passes_for_in_range_addition(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )
        value = PPM(4, system=system)

        value.add(5)

        self.assertEqual(value.format_value(), "9")
        self.assertFalse(value.has_overflow())
        self.assertEqual(value.overflow_check_mismatches(), ())
        self.assertEqual(value.format_native(), "1 0 (4)")

    def test_overflow_check_detects_wrapped_addition(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )
        value = PPM(10, system=system)

        value.add(5)

        self.assertEqual(value.format_value(), "3")
        self.assertTrue(value.has_overflow())
        self.assertEqual(value.overflow_check_mismatches(), ((2, 0, 3),))
        self.assertEqual(value.format_native(), "3 0 (0)")

    def test_format_usable_range_uses_value_digits(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )

        self.assertEqual(PPM.format_usable_range(system), "0..11")
        self.assertEqual(PPM.usable_range_max(system).format_native(), "3 2 (4)")

    def test_mixed_radix_conversion_defaults_to_value_digits(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.REDUNDANT],
        )
        value = PPM(5, system=system)
        mixed = MRN.from_ppm(value)

        self.assertEqual([digit.index for digit in mixed.digits], [0, 1])
        self.assertEqual(mixed.to_int(), 5)

    def test_div_std_rejects_auxiliary_digit_systems_for_now(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
        )
        dividend = PPM(10, system=system)
        divisor = PPM(2, system=system)

        with self.assertRaisesRegex(RNSCriticalError, "auxiliary digits"):
            dividend.div_std(divisor)

    def test_ppm_exports_residue_arrays_and_records(self):
        system = RNSNumberSystem(
            moduli=[2, 3, 5],
            powers=[2, 1, 1],
            digit_roles=[DigitRole.VALUE, DigitRole.VALUE, DigitRole.OVERFLOW_CHECK],
            name="export-demo",
        )
        value = PPM(10, system=system)

        self.assertEqual(value.to_residues(), (2, 1, 0))
        self.assertEqual(value.to_residues(include_auxiliary=False), (2, 1))

        records = value.to_digit_records()
        self.assertEqual(records[0]["index"], 0)
        self.assertEqual(records[0]["digit"], 2)
        self.assertEqual(records[0]["current_modulus"], 4)
        self.assertEqual(records[2]["modulus"], 5)

        exported = value.to_dict()
        self.assertEqual(exported["class"], "PPM")
        self.assertEqual(exported["residues"], (2, 1, 0))
        self.assertEqual(exported["system"]["name"], "export-demo")
        self.assertEqual(exported["system"]["digit_roles"], ("value", "value", "overflow_check"))


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


if __name__ == "__main__":
    unittest.main()
