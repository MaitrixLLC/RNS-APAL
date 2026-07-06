# RNS-PYPAL signed SPPM reference

## Class summary

| Item | Value |
| --- | --- |
| Class | `SPPM` |
| Module | `rns_pypal.sppm` |
| Inherits from | `PPM` |
| Derived classes | `SPMF` is intended to derive from `SPPM` |
| Numeric role | Signed partial-power residue integer |
| Core responsibility | Adds method-of-complements signed interpretation, sign metadata, sign validation, signed range handling, and signed arithmetic dispatch on top of `PPM` residue mechanics |

`SPPM` is a true derived class of `PPM`. The underlying residue value remains a
`PPM`-style RNS word; `SPPM` adds signed interpretation and sign-control policy.

This document describes the current signed integer slice of RNS-PYPAL. It is a
review reference for the Python implementation, not a final user manual.

`SPPM` is a signed interpretation layered on top of `PPM`. The underlying
method-of-complements residue encoding is the authoritative value. The cached
sign flag is redundant metadata used for speed, dispatch, and consistency
checking.

## Core model

`SPPM` stores the same residue digits as `PPM`, plus:

| Field | Meaning |
| --- | --- |
| `sign_flag` | Cached sign, `POSITIVE` or `NEGATIVE`. |
| `sign_valid` | Whether the cached sign is trusted. |

The sign constants are:

| Constant | Value | Meaning |
| --- | ---: | --- |
| `POSITIVE` | `0` | Positive or canonical zero. |
| `NEGATIVE` | `1` | Negative. |
| `SIGN_INVALID` | `0` | Cached sign is not trusted. |
| `SIGN_VALID` | `1` | Cached sign is trusted. |

The complement encoding is the ground truth. If `sign_valid` is true and the
cached sign disagrees with the complement-encoded residues, explicit sign
validation raises a critical error.

## Signed range

For the current signed integer slice, the ordinary signed range is derived from
the value digit dynamic range:

```text
minimum = -(dynamic_range // 2)
maximum = (dynamic_range - 1) // 2
```

For a system with dynamic range `48`, the signed range is:

```text
-24..23
```

Useful helpers:

| Method | Purpose |
| --- | --- |
| `SPPM.signed_range(system)` | Returns `(minimum, maximum)`. |
| `SPPM.format_signed_range(system)` | Returns `"minimum..maximum"`. |
| `SPPM.print_signed_range(system, file=None)` | Prints the signed range. |
| `SPPM.positive_range_max(system)` | Returns the largest positive signed value as a `PPM`. |
| `SPPM.negative_range_min_magnitude(system)` | Returns the magnitude of the most-negative value as a `PPM`. |

## Construction and assignment

Examples:

```python
x = SPPM(-7, system=system)
y = SPPM("-0xf", system=system)
z = SPPM(ppm_value)
```

| Method | Purpose |
| --- | --- |
| `__init__(value=0, *, system=None)` | Creates a signed value from `SPPM`, `PPM`, int, or signed string. |
| `assign(value)` | Assigns from `SPPM`, `PPM`, int, or signed string. |
| `assign_checked(value, *, on_error="critical")` | Explicitly checks an external int/string against the signed range before assigning. |
| `copy()` | Returns an independent `SPPM` copy including sign metadata. |
| `to_dict(include_auxiliary=True)` | Returns a plain-Python snapshot including inherited residue data and signed metadata. |

Ordinary `assign()` follows C/C++ fixed-width integer style: external values
wrap into the residue system. Conservative callers should use
`assign_checked()`.

`assign_checked()` supports diagnostic severities:

| Severity | Behavior |
| --- | --- |
| `"warning"` | Warn and continue with ordinary assignment. |
| `"error"` | Raise recoverable `RNSRangeError`. |
| `"critical"` | Raise hard-stop `RNSCriticalError`. |

## Sign recovery and validation

| Method | Purpose |
| --- | --- |
| `calc_sign()` | Returns the sign implied by the complement encoding without changing metadata. |
| `calc_set_sign()` | Validates/recomputes the cached sign. Raises critical error if a valid cached sign disagrees with residues. |
| `sign_mismatch()` | Non-mutating query for valid-sign/residue disagreement. |
| `get_sign_flag()` | Returns cached sign flag. |
| `get_sign_valid()` | Returns cached sign validity. |

If `sign_valid` is invalid, `calc_set_sign()` recovers the sign from the
residue value and marks it valid. If `sign_valid` is valid and disagrees with
the residue-implied sign, that is a critical error.

## Signed operations

`SPPM` uses `PPM` arithmetic as the residue engine. Signed operations update or
invalidate the cached sign according to the SPPM sign-disposition policy.

| Method | Behavior |
| --- | --- |
| `add(other)` | Residue add; preserves same valid sign, invalidates mixed/unknown sign. |
| `sub(other)` | Residue subtract; applies subtraction sign-disposition rules. |
| `mult(other)` | Residue multiply; computes result sign when both input signs are valid. |
| `increment()` | Signed add one. |
| `decrement()` | Signed subtract one. |
| `negate()` | Computes complement negation and flips valid sign. |
| `abs()` | Recovers sign if needed, then negates negative values. |
| `compare(other)` | Uses sign first, then unsigned `PPM` comparison. |

Ordinary arithmetic does not automatically perform overflow checking. A valid
positive plus a valid positive preserves a positive cached sign under the same
assumption made by C/C++ integer arithmetic: the caller is responsible for range
discipline.

## Formatting

| Method | Purpose |
| --- | --- |
| `format_value(radix=10, prefix=False)` | Returns the signed value string using complement encoding as ground truth. |
| `print_value(radix=10, prefix=False, file=None)` | Prints `format_value()`. |
| `format_native(radix=10)` | Inherited raw residue display from `PPM`. |
| `print_whdr(...)` | Inherited header/residue display from `PPM`. |

`format_value()` does not trust stale sign metadata. It interprets the
complement-encoded residues directly.

## Data export

`SPPM` inherits the residue export methods from `PPM`:

- `to_residues(include_auxiliary=True)`
- `to_digit_records(include_auxiliary=True)`
- `to_dict(include_auxiliary=True)`

`SPPM.to_dict()` extends the base `PPM` snapshot with:

- `sign_flag`
- `sign_valid`
- `residue_sign`
- `value`

The exported `value` is a boundary display string. The residues and
method-of-complements interpretation remain the authoritative signed state.

## Current limitations

- `SPPM` currently rejects auxiliary digit systems until signed auxiliary
  policies are designed and tested.
- Signed division is not implemented yet.
- Fixed-point behavior belongs to `SPMF`, not this signed integer slice.

## Quick example

```python
from rns_pypal import SPPM, RNSNumberSystem

system = RNSNumberSystem(moduli=[2, 3], powers=[4, 1])

x = SPPM(-7, system=system)
print(x.format_native())  # complement residues
print(x.format_value())   # -7

x.sign_valid = 0
x.calc_set_sign()
print(x.sign_flag, x.sign_valid)
```
