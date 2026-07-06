# RNS-PYPAL PPMDigit reference

## Class summary

| Item | Value |
| --- | --- |
| Class | `PPMDigit` |
| Module | `rns_pypal.ppm` |
| Inherits from | Python `object` |
| Derived classes | None currently |
| Numeric role | One residue channel in a partial-power RNS word |
| Core responsibility | Stores a residue, its base modulus, current and normalized powers, skip state, and per-digit modular arithmetic behavior |

`PPMDigit` is the smallest active arithmetic object in the current RNS-PYPAL
port. A `PPM` value owns an ordered list of these digits. The digit order is
stable and significant for mixed-radix conversion even though the residue
digits themselves are not positional values.

This document is both reference and design note. It records the current Python
behavior while preserving the RNS-APAL partial-power interpretation needed for
later division, normalization, comparison, and fixed-point operations.

## Conceptual model

Each residue channel is more than an integer modulo a fixed modulus. In a
power-based RNS system, a digit has:

- a base modulus, such as `2`, `3`, or `5`;
- a normalized power, defining the full channel modulus;
- a current working power;
- a currently valid power; and
- a skip state used when the digit has been temporarily removed from a derived
  working format.

Normally:

```text
power == power_valid == normal_power
```

Derived formats can reduce `power_valid`. `AssignPM`-style operations can
promote a reduced `power_valid` into the current working `power` while retaining
`normal_power` as the route back to the full normalized system.

## Stored fields

| Field | Meaning |
| --- | --- |
| `index` | Stable digit index. |
| `digit` | Current residue value. |
| `modulus` | Base modulus. |
| `power` | Current working maximum power for this digit. |
| `power_valid` | Currently active valid power. |
| `normal_power` | Full normalized power used for restoration. |
| `skip` | Whether this digit is currently invalid or removed. |

The effective current modulus is:

```text
modulus ** power_valid
```

If `power_valid` reaches zero, the digit is skipped and no longer contributes
to the current working value until extension or normalization reconstructs it.

## Important properties and methods

| Member | Purpose |
| --- | --- |
| `full_modulus` | Returns `modulus ** power`. |
| `current_modulus` | Returns `modulus ** power_valid`; invalid for skipped digits. |
| `is_normalized` | True when the digit is active and full-normalized. |
| `assign(value)` | Assigns residue modulo the current modulus. |
| `add(value)` | Adds modulo the current modulus. |
| `sub(value)` | Subtracts modulo the current modulus. |
| `mult(value)` | Multiplies modulo the current modulus. |
| `zero_power()` | Finds the largest active base power dividing the residue. |
| `pow_offset(power)` | Offset needed to make the digit divisible by `base ** power`. |
| `can_reduce_by_base_power(divisor)` | Checks same-base direct division precondition. |
| `mod_div(divisor)` | Divides a digit by a scalar factor. |
| `skip_digit()` | Marks the digit skipped and clears its residue. |
| `copy()` | Returns an independent digit copy. |
| `to_record()` | Returns a plain-Python dictionary snapshot of residue, modulus, power, skip, and current-modulus state. |

## Modular division behavior

`mod_div(divisor)` has two mathematically different modes.

If the divisor is a power of this digit's own base modulus, the digit is
directly divided and `power_valid` is reduced. For example, a base-2 digit with
current modulus `16` may be divided by `2`, reducing its valid modulus to `8`
when the residue is divisible by `2`.

Otherwise, the divisor must be coprime to the current modulus. The operation
then multiplies the residue by the divisor's modular inverse with respect to
the current modulus.

These two cases must not be blurred. Same-base division changes the derived
format. Coprime inverse division preserves the digit's effective modulus.

## Supporting helper functions

Several utility functions are important to digit-level behavior:

| Function | Exported? | Purpose |
| --- | --- | --- |
| `are_coprime(left, right)` | Yes | True when two integers are coprime. |
| `multiplicative_inverse(value, modulus)` | Yes | Returns modular inverse of `value` modulo `modulus`. |
| `divide_residue_by_coprime_factor(residue, divisor, modulus)` | Yes | Divides one residue channel using inverse modular multiplication. |
| `count_base_power_factor(value, base)` | No | Counts how many powers of `base` divide `value`. |
| `is_exact_base_power(value, base)` | No | True when `value == base ** k` for positive integer `k`. |
| `is_pairwise_coprime(values)` | Yes | True when all values in a sequence are pairwise coprime. |

The flexible default inverse strategy is the extended Euclidean algorithm.
Lookup-table strategies may be added later, but they should be optimization
policy, not a change in digit semantics.

## Critical-error behavior

Digit-level operations raise critical errors when a mathematical precondition
is violated and continuing would hide a broken RNS state. Examples include:

- division by zero;
- direct same-base division when the residue is not divisible by the required
  base power;
- inverse division when the divisor is not coprime to the current modulus; and
- reading a current modulus from a skipped digit.

Critical errors use the project `CRITICAL ERROR` reporting path and include
context such as digit index, residue, divisor, and modulus when available.

## Data export

`PPMDigit.to_record()` is the digit-level export method. It returns ordinary
Python data suitable for notebooks, diagnostics, test assertions, and later
serialization work.

The record includes:

- `index`
- `digit`
- `modulus`
- `power`
- `power_valid`
- `normal_power`
- `skip`
- `full_modulus`
- `current_modulus`

For skipped digits, `current_modulus` is `None`. This avoids confusing a
skipped digit with an active digit whose residue happens to be zero.
