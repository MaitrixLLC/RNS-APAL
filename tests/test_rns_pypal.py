import io
import unittest

from rns_pypal import PPM, PPMDigit, RNSNumberSystem


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

    def test_copy_assignment_is_independent(self):
        source = PPM(10, system=self.system)
        destination = PPM(0, system=self.system)

        destination.assign(source)
        source.add(1)

        self.assert_residues(destination, 10)
        self.assert_residues(source, 11)

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

    def test_compare_is_not_faked_with_raw_residue_ordering(self):
        with self.assertRaisesRegex(NotImplementedError, "mixed-radix"):
            PPM(1, system=self.system).compare(PPM(2, system=self.system))

    def test_native_format_supports_decimal_binary_and_hex(self):
        value = PPM(10, system=self.system)

        self.assertEqual(value.format_native(10), "1 0 3")
        self.assertEqual(value.format_native(2), "1 0 11")
        self.assertEqual(value.format_native(16), "1 0 3")

    def test_demo_format_maps_to_cpp_print_demo(self):
        display_system = RNSNumberSystem(moduli=[2, 3], powers=[4, 3])
        value = PPM(10, system=display_system)

        self.assertEqual(value.format_demo(), "16 27\n-- --\n10 10")

        output = io.StringIO()
        value.print_demo(file=output)
        self.assertEqual(output.getvalue(), "16 27\n-- --\n10 10\n")


if __name__ == "__main__":
    unittest.main()
