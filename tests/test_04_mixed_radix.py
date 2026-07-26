"""Tests for MixedRadix in hierarchy order."""

import unittest

from rns_pypal import (
    MRN,
    MixedRadixDecomposer,
    PPM,
    RNSNumberSystem,
    iter_mixed_radix_digits,
)


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

    def test_mrn_shift_right_discards_low_order_mixed_radix_digits(self):
        value = PPM(384, system=self.system)
        mrn = MRN.from_ppm(value)

        shifted = mrn.shifted_right(1)

        self.assertEqual(shifted.to_int(), 76)
        self.assertEqual(shifted.to_ppm().format_value(), "76")
