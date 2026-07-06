# RNS-PYPAL PPM reference

## Class summary

| Item | Value |
| --- | --- |
| Class | `PPM` |
| Module | `rns_pypal.ppm` |
| Inherits from | Python `object` |
| Derived classes | `SPPM` derives from `PPM`; `SPMF` is intended to derive from `SPPM` |
| Numeric role | Unsigned partial-power residue integer |
| Core responsibility | Owns residue digits, RNS system compatibility, unsigned arithmetic, mixed-radix conversion, partial-power handling, normalization, and unsigned output conversion |

`PPM` is the foundational numeric class for RNS-PYPAL. Higher-level signed and
fixed-point classes should reuse this unsigned residue engine and override only
the semantics that actually change.

This document describes the current unsigned integer class behavior. It is a
review reference for the Python implementation, not a final user manual.

## RNS system dependency

Every `PPM` value belongs to an `RNSNumberSystem`. That system defines the
ordered base moduli, normalized powers, auxiliary digit roles, and fractional
digit count retained for later `SPMF` work.

Important system rules:

- base moduli and full normalized moduli must be pairwise coprime;
- powers define full normalized moduli as `base ** power`;
- digit order is stable and significant for mixed-radix conversion;
- value digits define ordinary dynamic range;
- auxiliary digits may participate in arithmetic but do not extend ordinary
  comparison, conversion, or range; and
- multiple RNS systems may coexist in one Python process, but arithmetic
  operands must be compatible.

Useful `RNSNumberSystem` properties include:

| Member | Purpose |
| --- | --- |
| `num_digits` | Number of residue digits. |
| `full_moduli` | Tuple of `modulus ** power` values. |
| `value_indices` | Digit indices that define the represented value. |
| `auxiliary_indices` | Non-value digit indices. |
| `arithmetic_indices` | Digits updated by ordinary low-level arithmetic. |
| `comparison_indices` | Digits used for ordinary comparison. |
| `conversion_indices` | Digits used for ordinary output conversion. |
| `dynamic_range` | Product of value full moduli only. |
| `is_compatible(other)` | Checks normalized geometry compatibility. |

## Construction and assignment

```python
value = PPM(123, system=system)
value = PPM("12345678901234567890", system=system)
value = PPM("0x1e240", system=system)
value = PPM("0b101010", system=system)
copy_value = PPM(value)
```

| Method | Public? | Purpose |
| --- | --- | --- |
| `__init__(value=0, *, system=None)` | Yes | Creates a `PPM` from an integer, string, or compatible `PPM`. Copy construction can infer the system from the source. |
| `assign(value)` | Yes | Assigns an integer, unsigned string, or compatible `PPM`. Integer/string assignment resets to the normalized system. |
| `assign_checked(value, *, on_error="critical")` | Yes | Explicitly range-checks an external unsigned integer/string before assignment. Ordinary `assign()` still wraps. |
| `assign_pm(value, *, format_source=None)` | Yes | AssignPM-style partial-power assignment. Assigns into the current derived format, or adopts a source derived format before assigning. |
| `copy()` | Yes | Returns an independent value copy. |
| `derived_format_copy()` | Yes | Copies the value and promotes active `power_valid` values into the working `power` fields. |

String assignment is intentionally performed by repeated residue-domain
multiply/add over the text digits. This permits very large decimal strings and
avoids depending on Python's guarded direct decimal-to-int conversion.

`assign_checked()` is the conservative external-input path. It verifies that an
integer or unsigned string is in `0..dynamic_range-1` before assigning. Its
diagnostic severity can be selected as warning, recoverable error, or critical
error. The default is critical. Ordinary `assign()` follows the RNS-PYPAL
wraparound policy.

## Basic properties and iteration

| Method/property | Public? | Purpose |
| --- | --- | --- |
| `num_digits` | Yes | Number of residue digits. |
| `power_based` | Yes | Whether the system uses powers greater than one. |
| `__iter__()` | Yes | Iterates over the underlying `PPMDigit` objects. |
| `to_residues(include_auxiliary=True)` | Yes | Returns raw stored residues as a tuple. |
| `to_digit_records(include_auxiliary=True)` | Yes | Returns per-digit state dictionaries. |
| `to_dict(include_auxiliary=True)` | Yes | Returns a plain-Python snapshot including class, system, residues, and digit records. |

## Arithmetic methods

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

## Derived-format and normalization methods

| Method | Public? | Purpose |
| --- | --- | --- |
| `normalize()` | Yes | Restores a derived value to the full normalized system using streamed mixed-radix reconstruction. |
| `normalized_copy()` | Yes | Returns a normalized copy without mutating the original. |
| `extend_to_current_power()` | Yes | Reconstructs skipped/partial values into the current working power format, not necessarily full normal power. |
| `extended_to_current_power_copy()` | Yes | Returns an extended-to-current-power copy. |
| `any_part_skips()` | Yes | True when any digit is skipped or has reduced valid power. |

`normalize()` and `extend_to_current_power()` are fundamental to partial-power
RNS processing. The first returns a value to the original full system. The
second restores representability within the current derived power structure.

## Comparison and predicates

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

## Division internals

These helpers support `div_std()` and are not intended as stable user API.

| Method | Purpose |
| --- | --- |
| `_has_divisible_power_factor_before(end_index)` | Detects whether a working divisor has a usable factor. |
| `_dec_by_next_factor(factor_source, start_index, end_index)` | Subtracts an offset to make the dividend divisible by the next available factor. |
| `_index_of_base_modulus(base)` | Finds the digit index for a base modulus such as `2`. |
| `_division_check(dividend, divisor, quotient, remainder)` | Verifies `dividend == divisor * quotient + remainder` and `remainder < divisor`. |

## Conversion, formatting, and display

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
show them as a parenthesized suffix. This makes the extra residues visible for
debugging without implying that they extend the ordinary value range.

Conversion methods are allowed to use mixed-radix reconstruction or Python
integers because they are output/debug facilities. Arithmetic methods must not
use them to compute RNS results.

## Data export

`PPM` values can be exported into plain Python data structures:

| Method | Purpose |
| --- | --- |
| `to_residues(include_auxiliary=True)` | Returns a tuple of raw stored residue values. |
| `to_digit_records(include_auxiliary=True)` | Returns a tuple of dictionaries preserving digit state, including skip and power metadata. |
| `to_dict(include_auxiliary=True)` | Returns a dictionary snapshot containing class name, system geometry, residue tuple, and digit records. |

These export methods are intended for notebooks, diagnostics, test fixtures,
serialization, and interoperability with later array/data libraries. They are
boundary views of the RNS value. They do not make a positional integer the
ground truth, and arithmetic methods must not use exported arrays as a hidden
convert-operate-reencode path.

`include_auxiliary=False` restricts residue and digit-record exports to value
digits only. The default includes every digit because full diagnostic snapshots
should preserve redundant and overflow-check channels.

## Error and diagnostic helpers

RNS-PYPAL has a loud critical-error path for violated RNS preconditions.

| Object | Purpose |
| --- | --- |
| `RNSPypalError` | Base project exception. |
| `RNSCriticalError` | Critical error class; currently derives from `ValueError`. |
| `RNSRangeError` | Recoverable range-check exception. |
| `RNSDiagnosticWarning` | Warning class for non-halting diagnostics. |
| `RNSDiagnosticLevel` | Diagnostic severity selector. |
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
- `assign_checked()` warning/error/critical behavior;
- `assign_pm()` partial-power assignment behavior;
- explicit overflow-check digit prototypes; and
- `div_std()` including larger user-validated cases near the top of the system
  range.

Run the current verification with:

```powershell
.\.venv\Scripts\python.exe -m pytest
.\.venv\Scripts\python.exe -m ruff check rns_pypal tests docs\render_reference_html.py
```
