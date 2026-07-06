# RNS-PYPAL SPMF reference

## Class summary

| Item | Value |
| --- | --- |
| Class | `SPMF` |
| Module | `rns_pypal.spmf` |
| Inherits from | `SPPM` |
| Derived classes | None currently |
| Numeric role | Signed fixed-point partial-power residue value |
| Core responsibility | Adds fixed-point fraction-point metadata on top of signed `SPPM` residue mechanics |

`SPMF` is the signed fixed-point layer of RNS-PYPAL. The current Python slice
establishes the class format, sliding-point metadata, preliminary trap-door
I/O, unit-value helpers, and same-format add/subtract/compare. Fractional
multiplication, scaling, division, rounding, and normalization rules remain
future conversion slices.

## Fixed-point geometry

An `SPMF` value belongs to an `RNSNumberSystem` whose `fractional_digits` field
defines how many value digits belong to the fractional range.

The fractional digits must occupy the least-significant RNS indexes:

```text
fractional_indices = (0, 1, ..., fractional_digits - 1)
whole_indices      = remaining value digit indexes
```

This ordering is deliberate. Mixed-radix decomposition consumes RNS digits in
stable index order, so fixed-point algorithms can decompose fractional digits
first before proceeding into whole/integer-range digits.

The unit range is:

```text
unit_range = product(full_moduli[index] for index in fractional_indices)
```

This is the scale factor implied by the fractional digit set. Low-valued prime
powers in the fractional range are preferred because they provide a useful mix
of fractional denominators.

The basic fixed-point identity is:

```text
scaled_integer = whole_part * unit_range + fractional_numerator
represented_value = scaled_integer / unit_range
```

For signed values, the `SPPM` method-of-complements sign interpretation is
handled first. The whole/fractional separation then applies to the positive
magnitude.

## Whole and fractional separation

RNS-PYPAL treats fixed-point separation as an RNS decomposition problem.

The fractional numerator is the integer represented by the fractional digit
range. It can be obtained by retaining or reconstructing the mixed-radix
contribution of the fractional digits while the whole-range digits are ignored
or skipped for that extraction.

The whole part is obtained by consuming the fractional digits through
mixed-radix-style reduction. As each fractional digit is consumed, that digit is
marked skipped and the remaining channels are updated through modular division.
The remaining whole-range state can then be base-extended or reconstructed into
an integer RNS value. That integer is the whole part, not a value still scaled
by `unit_range`.

The C++ `SPMF::FracSep` routine is the main reference evidence. It separates a
fixed-point value by mixed-radix-style digit reduction, accumulating fractional
digits into one result and whole digits into another result using the current
partial-power digit moduli.

## Fractional multiplication design note

Fractional multiplication has an initial Python port through `mult_std()`,
following the C++ `SPMF::MultStd` selection of `Mult4b`.

At a high level, `SPMF` multiplication contains a raw integer multiplication
followed by scaling out one extra copy of the fractional range. This scaling
resembles the operation used to obtain a whole value from fractional digits, but
it is arithmetic normalization rather than display conversion. The result must
return to the declared fixed-point scale.

The standard same-format multiply has a raw-capacity rule. Since the operands
are multiplied as an integer-style residue product before fractional
normalization, the RNS format must be large enough to hold that intermediate
product. As a rule of thumb, preserving the same fractional and whole digit
counts in the final result requires raw RNS capacity for the square of the
intended fixed-point value range. This determines the largest fixed-point value
that can be multiplied by itself and still fit before normalization.

This mirrors binary fixed-point arithmetic: the multiply needs enough internal
product bits before the result is shifted or rounded back to the destination
format. A future `SPMF` API may explicitly multiply through a larger internal
RNS work format and then cast or normalize back to the public format, but that
should be a separate advanced operation rather than the baseline same-format
multiply.

Rounding is controlled by the fractional mixed-radix reduction stream. As the
fractional positions are consumed, the discarded fractional portion is compared
against the half-range threshold of the fixed-point unit. If the discarded
portion reaches that rounding threshold, one retained fixed-point unit is added;
otherwise the retained result is left unchanged. The current port follows the
C++ `Mult4b` tie behavior: an exact positive half-range case increments before
sign finalization; the negative case is handled through the later correction
constant path.

For signed values, the C++ `SPMF::MultStd` path currently delegates to `Mult4b`.
That routine computes the raw residue product, derives the sign from the
method-of-complements range, shifts the mixed-radix representation right by the
fractional digit count, and applies a whole-range correction constant when the
intermediate product is negative.

The related tandem-complement methods in the C++ class remain useful reference
material for understanding the signed normalization problem, but `Mult4b` is
the first ported application path. The Python design should still leave room
for a later single-RNS-value signed multiplication algorithm once it is
specified and tested.

## Construction

```python
system = RNSNumberSystem(
    moduli=[2, 3, 5],
    powers=[4, 2, 1],
    fractional_digits=2,
)

x = SPMF(-7, system=system)
```

Current construction accepts the same underlying residue inputs as `SPPM`:

- `SPMF`
- `SPPM`
- `PPM`
- `int`
- signed numeric string

For direct construction, those inputs are treated as the underlying scaled
fixed-point residue value. Use the explicit assignment helpers below when the
external value is a whole integer, decimal text, or ratio.

## Preliminary trap-door I/O policy

`SPMF` I/O is allowed to use Python arbitrary-precision integer and string
support as a boundary-layer fallback. This is a pragmatic research feature: it
lets many RNS geometries be assigned, inspected, and printed before every
specialized RNS-native conversion routine has been ported.

The planned baseline strategy is:

1. parse a fixed-radix external value using Python's arbitrary-precision
   integer/string tools;
2. scale the parsed value into the current or normal fractional unit range;
3. encode the resulting scaled integer into RNS residues; and
4. for output, interpret the fixed-point residue value as a scaled integer
   numerator over `unit_range` or `normal_unit_range`.

This is intentionally classified as I/O conversion. It must not be used to
implement fixed-point arithmetic. Addition, subtraction, comparison,
multiplication, division, scaling, rounding, square root, and later numerical
algorithms must remain RNS-native or explicitly mixed-radix/RNS-derived.

Over time, conversion dispatch should become capability-based. If the modulus
set supports a specialized RNS-native decimal or fixed-radix conversion path,
that path should be preferred. The trap-door path remains the general fallback
for flexible research systems.

## Preliminary assignment and display helpers

| Method | Purpose |
| --- | --- |
| `assign_scaled_integer(value)` | Assigns an already-scaled integer in normal fixed-point format. |
| `assign_whole(value)` | Assigns a whole integer by multiplying it by the normal unit range. |
| `assign_ratio(numerator, denominator)` | Assigns a ratio by scaling through the unit range and rounding to nearest scaled integer. |
| `assign_decimal(value)` | Assigns decimal text through the trap-door I/O path. |
| `to_scaled_integer()` | Returns the signed scaled integer for diagnostics and I/O. |
| `fixed_parts()` | Returns sign, whole part, fractional numerator, unit range, and scaled integer. |
| `format_scaled_integer(radix=10, prefix=False)` | Formats the underlying signed scaled integer. |
| `format_ratio()` | Formats the exact diagnostic ratio `scaled_integer/unit_range`. |
| `format_decimal(max_fraction_digits=12)` | Formats a truncated decimal diagnostic string. Always includes a decimal point. |
| `format_fixed_native(radix=10)` | Formats raw residues with a fixed-point marker after the fractional digits. |
| `print_fixed_native(...)` | Prints `format_fixed_native()`. |
| `print_decimal(...)` | Prints `format_decimal()`. |
| `print_ratio(...)` | Prints `format_ratio()`. |

These helpers are boundary conversion features. They are intended to make the
first fixed-point values inspectable and testable across many RNS systems.

By convention, `SPMF` decimal output always includes a decimal point. Whole
fixed-point values therefore print as `1.`, `0.`, or `-2.` rather than `1`,
`0`, or `-2`. This makes fixed-point output visually distinct from integer
`SPPM` output.

## Metadata

| Member | Meaning |
| --- | --- |
| `num_fract_digits` | Current fractional digit count. |
| `normal_fract_digits` | Normalized fractional digit count. |
| `fractional_indices` | Value digit indexes in the current fractional range. |
| `whole_indices` | Value digit indexes in the current whole/integer range. |
| `normal_fractional_indices` | Value digit indexes in the normalized fractional range. |
| `normal_whole_indices` | Value digit indexes in the normalized whole/integer range. |
| `unit_range` | Product of full moduli in the current fractional range. |
| `normal_unit_range` | Product of full moduli in the normalized fractional range. |
| `sliding_point_active` | True when the current fraction point differs from the normal position. |

`num_fract_digits` and `normal_fract_digits` mirror the C++ distinction between
current and normalized fraction-point state.

## Sliding-point policy

The sliding point is the current fixed-point fraction position carried by an
`SPMF` value. The normal point is the fraction position defined by the
`RNSNumberSystem`.

This first Python slice supports the metadata and validation plumbing:

| Method | Purpose |
| --- | --- |
| `set_fraction_position(fractional_digits)` | Metadata-only change to the current fraction position. Does not scale residues. |
| `reset_fraction_position()` | Restores the current point to `normal_fract_digits`. |
| `has_same_fraction_position(other)` | Checks whether two `SPMF` values have compatible current and normal positions. |
| `ensure_fraction_position_compatible(other)` | Raises a critical error when two operands are not aligned. |
| `ensure_normal_fraction_position()` | Raises a critical error when a value has not returned to the normal position. |

This is deliberately conservative. Internal algorithms may temporarily move the
current fraction point, but public top-level results should normally return to
the normalized fraction position unless a method explicitly documents that it
returns a sliding-point value.

Addition, subtraction, comparison, and any other operation whose meaning
depends on fixed-point scale must check fraction-position compatibility before
the operation begins. If two operands are not aligned, a future implementation
must either scale/align them through RNS operations or fail clearly. It must not
silently treat different fraction positions as equivalent.

## Data export

`SPMF` inherits the residue and sign export methods from `SPPM` and extends
`to_dict()` with fixed-point metadata:

- `num_fract_digits`
- `normal_fract_digits`
- `fractional_indices`
- `whole_indices`
- `normal_fractional_indices`
- `normal_whole_indices`
- `unit_range`
- `normal_unit_range`
- `sliding_point_active`

These fields make exported snapshots unambiguous for notebooks, diagnostics,
and future serialization.

## Unit-value helpers

The current unit value, corresponding to `1.0`, is the product of the effective
moduli in the current fractional range. This follows the C++ `GetUnitPM()` /
`AssignUnitPM()` idea.

| Method | Purpose |
| --- | --- |
| `unit_ppm()` | Returns a `PPM` holding the current fixed-point unit. |
| `unit_value()` | Returns an `SPMF` value equal to `1.0`. |
| `assign_unit()` | Mutates the value to `1.0`. |

For a normalized system with fractional moduli `16` and `9`, the unit range is
`144`, so the scaled integer representation of `1.0` is `144`.

## Same-format arithmetic

Same-format addition, subtraction, comparison, and standard multiplication are
low-level signed fixed-point operations backed by the `SPPM`/`PPM` residue
machinery:

| Method | Behavior |
| --- | --- |
| `add(other)` | Requires another aligned `SPMF`; delegates to `SPPM.add()`. |
| `sub(other)` | Requires another aligned `SPMF`; delegates to `SPPM.sub()`. |
| `compare(other)` | Requires another aligned `SPMF`; delegates to `SPPM.compare()`. |
| `mult_std(other)` | Requires another aligned `SPMF`; performs C++ `MultStd` / `Mult4b`-style fixed-point multiply. |
| `mult(other)` | Uses `mult_std()` for aligned `SPMF` operands; integer operands are covered below. |

These operations require identical system, effective residue format, and
current/normal fraction-position metadata. Addition, subtraction, and comparison
do not move the fraction point. Standard multiplication scales the raw product
back to the same fraction point by discarding the low fractional mixed-radix
positions, applying rounding, and setting a valid sign flag.

Mixed-type integer operations are documented separately because their scaling
rules differ from same-format fractional arithmetic.

## Mixed-type integer operations

The first mixed-type slice ports the simpler C++ operations:

- `SPMF::Add1(__int64)`
- `SPMF::Sub1(__int64)`
- `SPMF::Mult(__int64)`
- `SPMF::Mult(SPPM*)`

The Python API exposes these as:

| Method | Behavior |
| --- | --- |
| `add_whole(other)` | Adds an integer as whole fixed-point units: `other * unit_range`. |
| `sub_whole(other)` | Subtracts an integer as whole fixed-point units. |
| `add1(other)` | Compatibility alias for `add_whole()`. |
| `sub1(other)` | Compatibility alias for `sub_whole()`. |
| `mult_integer(other)` | Multiplies the scaled fixed-point integer by an integer operand; no fractional renormalization is needed. |
| `add(other)` / `sub(other)` | Accept either aligned `SPMF` operands or integer operands. Integer operands are treated as whole units. |
| `mult(other)` | Accepts aligned `SPMF` operands for fractional multiply or integer operands for integer scaling. |

Accepted integer operand types are deliberately explicit:

- `int`
- signed numeric `str`
- `SPPM`

An `SPMF` operand is never treated as an integer operand. It routes to
fixed-point fractional arithmetic instead. This distinction matters because an
`SPMF` carries a fraction point while an `SPPM` represents an unscaled signed
integer.

For addition and subtraction, an integer operand is first converted into an
aligned temporary `SPMF` value equal to `integer * unit_range`, then ordinary
same-format fractional add/subtract is used. For multiplication by an integer,
the current scaled integer is multiplied directly using inherited `SPPM`
arithmetic; the fraction point remains unchanged.

## Current limitations

- Scaling, division, inverse, square root, and Goldschmidt-style routines are
  not implemented yet.
- Fractional multiplication currently follows the `Mult4b` reference path only;
  larger internal work formats and alternate signed algorithms remain future
  work.
- Full RNS-native fixed-point I/O dispatch is not implemented yet; current
  fixed-point I/O uses the documented trap-door fallback.
- Full arbitrary sliding-point alignment/scaling is not implemented yet.
- Signed auxiliary digit policy is still inherited from `SPPM`, which currently
  rejects auxiliary digit systems.
