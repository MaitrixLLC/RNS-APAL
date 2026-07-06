# RNS-PYPAL unsigned PPM reference

This document describes the unsigned integer side of the current RNS-PYPAL
conversion. It is a review reference for the Python implementation, not a final
user manual. The design intent remains traceable to the C++ `PPM`, `PPMDigit`,
`MRN`, and helper routines, while assuming the newer partial-power `PPM`
semantics as the foundation.

The C++ reference source lives in `cpp_ref/` at the repository root.

The central rule is unchanged: arithmetic on RNS values must be performed in
the residue domain, with permitted mixed-radix streaming or derived
partial-power formats. Converting to a Python integer is allowed for input,
output, diagnostics, and test oracles, but not as the implementation of RNS
arithmetic.

This reference also assumes the auxiliary-digit policy now used by the
architecture document: auxiliary digits are for redundancy and overflow
checking, not for ordinary range or precision extension. To gain more range or
precision, convert to a larger primary `RNSNumberSystem`; do not treat
redundant digits as extra value digits.

## Public unsigned API currently exported

The current `rns_pypal` package exports these unsigned-side objects and helpers:

- `RNSNumberSystem`
- `DigitRole`
- `PPMDigit`
- `PPM`
- `MRDigit`
- `MixedRadixDecomposer`
- `MRN`
- `iter_mixed_radix_digits`
- `are_coprime`
- `divide_residue_by_coprime_factor`
- `is_pairwise_coprime`
- `multiplicative_inverse`
- `RNSPypalError`
- `RNSCriticalError`
- `DEFAULT_SYSTEM`
- `REZ9_INTEGER_SYSTEM`

Signed and fractional classes exist as early placeholders, but they are not yet
part of the verified public conversion slice.

## RNSNumberSystem

`RNSNumberSystem` defines the geometry of an RNS system.

Constructor:

```python
RNSNumberSystem(
    moduli=[2, 3, 5],
    powers=[4, 2, 1],
    digit_roles=["value", "value", "value"],
    fractional_digits=0,
    name=None,
)
```

Important rules:

- `moduli` are base moduli.
- `powers` define each full normalized modulus as `base ** power`.
- The full normalized moduli must be pairwise coprime.
- `digit_roles` defaults to all `value` digits.
- Auxiliary roles currently include `redundant` and `overflow_check`.
- Auxiliary digits must be trailing digits.
- Auxiliary digits participate in ordinary arithmetic when present, but they do
  not define ordinary dynamic range, comparison, or output conversion.
- The order of the digit list is stable and significant for mixed-radix
  conversion.
- `fractional_digits` is retained for later `SPMF` work, but unsigned `PPM`
  currently treats all digits as integer-range digits.

Useful properties and methods:

| Member | Purpose |
| --- | --- |
| `num_digits` | Number of residue digits. |
| `full_moduli` | Tuple of `modulus ** power` values. |
| `value_indices` | Digit indices that define the represented value. |
| `auxiliary_indices` | Non-value digit indices. |
| `redundant_indices` | Redundant/error-checking digit indices. |
| `overflow_check_indices` | Overflow-checking digit indices. |
| `arithmetic_indices` | Digits updated by ordinary low-level arithmetic. |
| `comparison_indices` | Digits used for ordinary comparison. |
| `conversion_indices` | Digits used for ordinary output conversion. |
| `value_full_moduli` | Full moduli for value digits only. |
| `full_modulus(index)` | Full normalized modulus for one digit. |
| `dynamic_range` | Product of value full moduli only. |
| `unit_range` | Product of fractional value-digit full moduli. |
| `power_based` | True when at least one digit has power greater than one. |
| `is_compatible(other)` | Checks normalized geometry compatibility. |

## PPMDigit

`PPMDigit` represents one residue channel in a power-based RNS word.

Tracked fields:

| Field | Meaning |
| --- | --- |
| `index` | Stable digit index. |
| `digit` | Current residue value. |
| `modulus` | Base modulus, such as `2`, `3`, or `5`. |
| `power` | Current working maximum power for this digit. |
| `power_valid` | Currently active valid power. |
| `normal_power` | Full normalized power used for restoration. |
| `skip` | Whether this digit is currently invalid/removed. |

Normally `power == power_valid == normal_power`. Derived formats can reduce
`power_valid`, and `AssignPM`-style operations can promote a reduced
`power_valid` into the working `power` while retaining `normal_power`.

Important properties and methods:

| Member | Purpose |
| --- | --- |
| `full_modulus` | Returns `modulus ** power`. |
| `current_modulus` | Returns `modulus ** power_valid`; invalid for skipped digits. |
| `is_normalized` | True when digit is active and full-normalized. |
| `assign(value)` | Assigns residue modulo current modulus. |
| `add(value)` | Adds modulo current modulus. |
| `sub(value)` | Subtracts modulo current modulus. |
| `mult(value)` | Multiplies modulo current modulus. |
| `zero_power()` | Finds the largest active base power dividing the residue. |
| `pow_offset(power)` | Offset needed to make digit divisible by `base ** power`. |
| `can_reduce_by_base_power(divisor)` | Checks same-base direct division precondition. |
| `mod_div(divisor)` | Divides a digit by a scalar factor. |
| `skip_digit()` | Marks the digit skipped and clears its residue. |
| `copy()` | Returns an independent digit copy. |

`mod_div()` has two modes:

- If the divisor is a power of this digit's own base modulus, the digit is
  directly divided and `power_valid` is reduced.
- Otherwise, the divisor must be coprime to the current modulus, and inverse
  modular multiplication is used.

## PPM class

`PPM` is the mutable unsigned integer class. Methods generally mutate the
object, matching the C++ style.

### Construction and assignment

```python
value = PPM(123, system=system)
value = PPM("12345678901234567890", system=system)
value = PPM("0x1e240", system=system)
value = PPM("0b101010", system=system)
copy_value = PPM(value)
```

| Method | Public? | Purpose |
| --- | --- | --- |
| `__init__(value=0, *, system=None)` | Yes | Creates a PPM from an integer, string, or compatible `PPM`. Copy construction can infer the system from the source. |
| `assign(value)` | Yes | Assigns an integer, unsigned string, or compatible `PPM`. Integer/string assignment resets to the normalized system. |
| `assign_pm(value, *, format_source=None)` | Yes | AssignPM-style partial-power assignment. Assigns into the current derived format, or adopts a source derived format before assigning. |
| `copy()` | Yes | Returns an independent value copy. |
| `derived_format_copy()` | Yes | Copies the value and promotes active `power_valid` values into the working `power` fields. |
| `_assign_integer(value)` | Internal | Normalized integer assignment. |
| `_assign_text(value)` | Internal | Normalized string assignment by residue multiply/add. |
| `_assign_integer_in_current_format(value)` | Internal | Integer assignment into current effective format. |
| `_assign_text_in_current_format(value)` | Internal | String assignment into current effective format. |
| `_promote_valid_powers_to_current_format()` | Internal | Promotes current `power_valid` into working `power`. |
| `_apply_current_format_from(source)` | Internal | Builds this value's current format from another PPM's valid-power state. |

String assignment is intentionally performed by repeated residue-domain
multiply/add over the text digits. This permits very large decimal strings and
avoids depending on Python's guarded direct decimal-to-int conversion.

### Basic properties and iteration

| Method/property | Public? | Purpose |
| --- | --- | --- |
| `num_digits` | Yes | Number of residue digits. |
| `power_based` | Yes | Whether the system uses powers greater than one. |
| `__iter__()` | Yes | Iterates over the underlying `PPMDigit` objects. |

### Arithmetic methods

| Method | Public? | Purpose |
| --- | --- | --- |
| `add(other)` | Yes | Adds another compatible `PPM` or integer scalar. |
| `sub(other)` | Yes | Subtracts another compatible `PPM` or integer scalar. |
| `mult(other)` | Yes | Multiplies by another compatible `PPM` or integer scalar. |
| `increment()` | Yes | Adds one. |
| `decrement()` | Yes | Subtracts one. |
| `mod_div(divisor)` | Yes | Low-level RNS modular division by a scalar factor. May reduce `power_valid`. |
| `div_std(divisor, *, max_iterations=1_000_000)` | Yes | Unsigned arbitrary integer division. Mutates dividend into quotient and returns remainder. |
| `overflow_check_mismatches()` | Yes | Returns `(index, actual, expected)` entries for overflow-check digits that disagree with value digits. |
| `has_overflow()` | Yes | True when overflow-check digits indicate wrapped unsigned overflow. |

`add`, `sub`, and `mult` do not perform overflow checks. Like C/C++ unsigned
integer arithmetic, the result wraps in the declared residue system.

`mod_div()` is a low-level operation used by mixed-radix conversion,
normalization, and division. It is only valid when the required per-digit
division preconditions hold.

`div_std()` ports the stable C++ `DivStd -> DivPM7` path. It requires a base
modulus of `2`, works best with low prime base moduli such as `2`, `3`, and
`5`, and uses the divisor-increment rule when no usable factor is available.

`has_overflow()` is an explicit prototype check. It is useful only for systems
with trailing `overflow_check` digits. Arithmetic maintains those digits, and
the check compares them against the residues implied by the value digits alone.
The check is not automatic and is not yet a general redundant-digit correction
system.

### Derived-format and normalization methods

| Method | Public? | Purpose |
| --- | --- | --- |
| `normalize()` | Yes | Restores a derived value to the full normalized system using streamed mixed-radix reconstruction. |
| `normalized_copy()` | Yes | Returns a normalized copy without mutating the original. |
| `extend_to_current_power()` | Yes | Reconstructs skipped/partial values into the current working power format, not necessarily full normal power. |
| `extended_to_current_power_copy()` | Yes | Returns an extended-to-current-power copy. |
| `any_part_skips()` | Yes | True when any digit is skipped or has reduced valid power. |
| `_zero_with_current_power_format()` | Internal | Creates zero in the current working power format. |
| `_reconstruct_mixed_radix_to_current_power(mixed_radix)` | Internal | Reconstructs an MRN-like value into the current power format. |

`normalize()` and `extend_to_current_power()` are fundamental to partial-power
RNS processing. The first returns a value to the original full system. The
second restores representability within the current derived power structure.

### Comparison and predicates

| Method | Public? | Purpose |
| --- | --- | --- |
| `is_zero()` | Yes | True if every active residue is zero. |
| `is_one()` | Yes | True if the value equals one in all active residues. |
| `is_equal(other)` | Yes | RNS equality under compatible format. |
| `compare(other)` | Yes | Returns `1` if self is greater than other, otherwise `0`. |
| `__eq__(other)` | Yes | Python equality including digit format state. |

`compare()` is not residue-lexicographic. It performs a mixed-radix streamed
comparison because RNS digits are not positional. This is one of the core rules
that prevents flawed RNS comparison.

### Division internals

These helpers support `div_std()` and are not intended as stable user API.

| Method | Purpose |
| --- | --- |
| `_has_divisible_power_factor_before(end_index)` | Detects whether a working divisor has a usable factor. |
| `_dec_by_next_factor(factor_source, start_index, end_index)` | Subtracts an offset to make the dividend divisible by the next available factor. |
| `_index_of_base_modulus(base)` | Finds the digit index for a base modulus such as `2`. |
| `_division_check(dividend, divisor, quotient, remainder)` | Verifies `dividend == divisor * quotient + remainder` and `remainder < divisor`. |

### Conversion, formatting, and display

| Method | Public? | Purpose |
| --- | --- | --- |
| `to_int()` | Yes | Converts through mixed-radix digits to a Python integer for output/debugging. |
| `format_value(radix=10, prefix=False)` | Yes | Returns the represented unsigned value as radix 2, 10, or 16 text. |
| `print_value(radix=10, prefix=False, file=None)` | Yes | Prints `format_value()`. |
| `usable_range_max(system)` | Yes | Class method returning the largest ordinary unsigned value for a system, computed as zero minus one. |
| `format_usable_range(system, radix=10, prefix=False)` | Yes | Class method returning the ordinary unsigned range as text. |
| `print_usable_range(system, radix=10, prefix=False, file=None)` | Yes | Prints `format_usable_range()`. |
| `format_native(radix=10)` | Yes | Returns raw residue digits. Skipped digits print as `*`; partial digits are marked `X|`. |
| `print_native(radix=10, file=None)` | Yes | Prints `format_native()`. |
| `format_whdr(radix=10, console_width=None)` | Yes | Prints modulus headers, separator markers, and residue digits. |
| `print_whdr(radix=10, console_width=None, file=None)` | Yes | Prints `format_whdr()`. |
| `__str__()` | Yes | Decimal value string through `format_value()`. |
| `__repr__()` | Yes | Debug representation using raw residues. |

`format_whdr()` uses:

- `-` for normalized or skipped columns;
- `=` for reduced/partial-power columns; and
- `*` for skipped residue digits.

If a system contains auxiliary digits, `format_native()` and `format_whdr()`
show them as a parenthesized suffix. For example, a two-value-digit system with
two auxiliary digits may print as `2 1 (0, 3)` in native form. This makes the
extra residues visible for debugging without implying that they extend the
ordinary value range.

Conversion methods are allowed to use mixed-radix reconstruction or Python
integers because they are output/debug facilities. Arithmetic methods must not
use them to compute RNS results.

## Mixed-radix helpers

Mixed-radix conversion is used for comparison, normalization, conversion, and
later division/fractional algorithms. The conversion order follows the stable
RNS digit order, so changing digit order changes the mixed-radix system.

### MRDigit

`MRDigit` stores one generated mixed-radix digit:

| Field | Meaning |
| --- | --- |
| `index` | Original RNS digit index. |
| `digit` | Mixed-radix digit value. |
| `radix` | Radix consumed at this conversion step. |
| `skip` | Whether the source digit was skipped. |

### MixedRadixDecomposer

`MixedRadixDecomposer(value)` streams mixed-radix digits from a `PPM`.

| Member | Purpose |
| --- | --- |
| `working` | Current derived working copy. |
| `digits` | Generated `MRDigit` values. |
| `is_complete` | True when all source digits have been consumed. |
| `step()` | Performs one subtract/divide mixed-radix conversion step. |
| `decompose_all()` | Completes conversion and returns all generated digits. |

Each `step()`:

1. reads the next active residue as the mixed-radix digit;
2. subtracts that digit from all active channels;
3. skips the consumed digit; and
4. divides remaining channels by the consumed radix using inverse modular
   multiplication or same-base power reduction.

This streamed form is central because algorithms such as comparison do not need
to store every mixed-radix digit permanently.

### iter_mixed_radix_digits

`iter_mixed_radix_digits(value)` returns a `MixedRadixDecomposer`. It exists as
a convenience entry point for code that wants to stream digits.

### MRN

`MRN` is the stored mixed-radix representation used mainly for demonstration,
debugging, reconstruction, and conversion.

| Method | Purpose |
| --- | --- |
| `MRN.from_ppm(value)` | Materializes all mixed-radix digits from a PPM. |
| `to_int()` | Converts stored mixed-radix digits to a Python integer. |
| `to_ppm(system=None)` | Reconstructs a normalized `PPM` by multiply/add in RNS. |
| `format_native()` | Displays stored mixed-radix digits. |

`MRN.to_ppm()` mirrors the C++ reconstruction idea: after deconstructing an RNS
value into mixed-radix digits, it reconstructs by multiplying the accumulated
value by the next radix and adding the digit.

## Utility helper functions

These helpers live in `rns_pypal.utils`.

| Function | Exported? | Purpose |
| --- | --- | --- |
| `greatest_common_divisor(a, b)` | No | Euclidean GCD helper. |
| `gcd(a, b)` | No | Alias for `greatest_common_divisor`. |
| `are_coprime(left, right)` | Yes | True when two integers are coprime. |
| `extended_gcd(a, b)` | No | Extended Euclidean helper returning coefficients. |
| `multiplicative_inverse(value, modulus)` | Yes | Returns modular inverse of `value` modulo `modulus`. |
| `modinv(value, modulus)` | No | Alias for `multiplicative_inverse`. |
| `divide_residue_by_coprime_factor(residue, divisor, modulus)` | Yes | Divides one residue channel using inverse modular multiplication. |
| `count_base_power_factor(value, base)` | No | Counts how many powers of `base` divide `value`. |
| `is_exact_base_power(value, base)` | No | True when `value == base ** k` for positive integer `k`. |
| `is_pairwise_coprime(values)` | Yes | True when all values in a sequence are pairwise coprime. |
| `crt(residues, moduli)` | No | Chinese remainder helper for conversion/debug/test support. |
| `power(base, exponent)` | No | Small validated exponentiation helper. |

The most important arithmetic helper is `multiplicative_inverse()`, because
mixed-radix conversion and many partial-power operations depend on inverse
modular multiplication. Lookup-table implementations may be added later, but
the extended-Euclidean path is the flexible default.

## Error helpers

RNS-PYPAL has a loud critical-error path for violated RNS preconditions.

| Object | Purpose |
| --- | --- |
| `RNSPypalError` | Base project exception. |
| `RNSCriticalError` | Critical error class; currently derives from `ValueError`. |
| `critical_error(reason, **context)` | Raises `RNSCriticalError` with a `CRITICAL ERROR` prefix, call location, and context values. |

Critical errors are used when continuing would hide a broken mathematical
precondition, such as dividing by zero, using division without a required base-2
modulus, or attempting an invalid same-base partial-power reduction.

## Current verification coverage

The unsigned tests currently cover:

- pairwise-coprime system validation;
- digit add/subtract/multiply;
- inverse modular multiplication;
- mixed-radix streaming;
- MRN reconstruction;
- PPM add/subtract/multiply over a small full range;
- scalar arithmetic;
- mixed-radix comparison;
- value formatting and raw residue formatting;
- partial-power `mod_div`;
- normalization and current-power extension;
- derived-format compatibility checks;
- `assign()` for integers and strings;
- `assign_pm()` partial-power assignment behavior; and
- `div_std()` including larger user-validated cases near the top of the system
  range.

Run the current verification with:

```powershell
.\.venv\Scripts\python.exe -m pytest
.\.venv\Scripts\python.exe -m ruff check rns_pypal tests
```
