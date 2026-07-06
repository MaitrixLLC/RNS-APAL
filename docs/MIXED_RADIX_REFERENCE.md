# RNS-PYPAL mixed-radix reference

## Class summary

| Item | Value |
| --- | --- |
| Classes | `MRDigit`, `MixedRadixDecomposer`, `MRN` |
| Module | `rns_pypal.mixed_radix` |
| Inherits from | Python `object` |
| Derived classes | None currently |
| Numeric role | Ordered mixed-radix conversion support for RNS values |
| Core responsibility | Stream or store mixed-radix digits generated from `PPM` values, supporting comparison, normalization, reconstruction, conversion, and later rounding/division algorithms |

Mixed-radix conversion is central to RNS-PYPAL. RNS digits are non-positional, so
raw residue arrays cannot be compared lexicographically. To compare, round,
normalize, or print general RNS values, the library needs an ordered
representation. RNS-PYPAL obtains that ordered representation through
mixed-radix conversion.

This document covers both the streaming conversion helper and the stored
mixed-radix representation. The streaming form is the preferred primitive when
an algorithm can evaluate each mixed-radix digit and discard it immediately.

## Conversion order

Mixed-radix conversion follows the stable order of the `PPM.rn` digit list.
Digit index zero is consumed first, then index one, and so on. Changing the RNS
digit order changes the mixed-radix number system.

This order dependency is especially important for future fixed-point work,
where fractional RNS digits must be converted before integer-position digits.

## Streaming decomposition rule

At each active digit position, forward conversion:

1. reads the current residue at that position as the next mixed-radix digit;
2. subtracts that digit from the working RNS value;
3. skips the consumed digit; and
4. divides the remaining active channels by the consumed radix.

The division in the remaining channels uses inverse modular multiplication when
the consumed radix is coprime to the remaining modulus. When the remaining
channel shares the same base modulus and the divisor is an exact base power,
the operation reduces that digit's valid power instead.

This is why mixed-radix conversion naturally creates derived RNS formats as it
proceeds. Each consumed digit is invalidated or skipped.

## `MRDigit`

`MRDigit` stores one generated mixed-radix digit.

| Field | Meaning |
| --- | --- |
| `index` | Original RNS digit index. |
| `digit` | Mixed-radix digit value. |
| `radix` | Radix consumed at this conversion step. |
| `skip` | Whether the source digit was skipped. |

`MRDigit` is intentionally small. It records enough information for stored
mixed-radix display and reconstruction.

## `MixedRadixDecomposer`

`MixedRadixDecomposer(value)` streams mixed-radix digits from a `PPM`.

| Member | Purpose |
| --- | --- |
| `working` | Current derived working copy. |
| `digits` | Generated `MRDigit` values. |
| `is_complete` | True when all source digits have been consumed. |
| `step()` | Performs one subtract/divide mixed-radix conversion step. |
| `decompose_all()` | Completes conversion and returns all generated digits. |

The decomposer owns a working copy of the source value. It mutates that working
copy as digits are consumed, so callers can inspect the derived skip and
partial-power state during conversion without mutating the original source.

This streamed form is a core RNS-PYPAL design rule. Algorithms such as
comparison, rounding, division, and fractional multiplication often need only
the next mixed-radix digit plus a small amount of operation-specific state.
They do not need to store the complete mixed-radix number.

## `iter_mixed_radix_digits`

`iter_mixed_radix_digits(value)` returns a `MixedRadixDecomposer`. It exists as
a convenience entry point for code that wants to stream digits without naming
the class directly.

## `MRN`

`MRN` is the materialized mixed-radix representation. It is useful for
demonstration, debugging, conversion, and algorithms that truly need to retain
every generated mixed-radix digit.

| Method | Purpose |
| --- | --- |
| `MRN.from_ppm(value)` | Materializes all mixed-radix digits from a `PPM`. |
| `to_int()` | Converts stored mixed-radix digits to a Python integer. |
| `to_ppm(system=None)` | Reconstructs a normalized `PPM` by multiply/add in RNS. |
| `format_native()` | Displays stored mixed-radix digits. |

`MRN.to_ppm()` mirrors the C++ reconstruction idea. After deconstructing an RNS
value into mixed-radix digits, reconstruction proceeds in the opposite
direction by multiplying the accumulated RNS value by the next radix and adding
the digit.

## Comparison and discard pattern

`PPM.compare()` should not compare raw residues. It uses the mixed-radix stream
to compare generated digits in positional significance order. Less-significant
mixed-radix differences may be overwritten by later more-significant
differences.

This pattern is not merely a memory optimization. It is one of the main RNS
design ideas in this project: generate a mixed-radix digit, evaluate it for the
operation at hand, then discard it while preserving only the reduced working
state.

## Conversion boundary

Mixed-radix conversion is allowed in conversion, comparison, normalization,
rounding, division support, and diagnostic code. It must not be confused with
the forbidden shortcut of converting operands to a positional integer, doing
the arithmetic there, and converting back to RNS.
