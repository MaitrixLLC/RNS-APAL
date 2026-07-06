# RNS-PYPAL unsigned reference index

This page is the unsigned-side documentation index for the current RNS-PYPAL
conversion. The original broad unsigned reference has been divided into smaller
class-library documents so each class can develop toward a serious user manual.

## Unsigned reference documents

| Document | Main class or topic | Purpose |
| --- | --- | --- |
| `PPM_REFERENCE.md` | `PPM` | Unsigned partial-power residue integer class. |
| `PPMDIGIT_REFERENCE.md` | `PPMDigit` | One residue channel, including per-digit modulus-power state. |
| `MIXED_RADIX_REFERENCE.md` | `MRDigit`, `MixedRadixDecomposer`, `MRN` | Streaming and stored mixed-radix conversion. |

Matching HTML files are generated beside the markdown files.

## Shared unsigned-side assumptions

The central rule is unchanged: arithmetic on RNS values must be performed in
the residue domain, with permitted mixed-radix streaming or derived
partial-power formats. Converting to a Python integer is allowed for input,
output, diagnostics, and test oracles, but not as the implementation of RNS
arithmetic.

The C++ reference source lives in `cpp_ref/` at the repository root. The Python
port assumes the newer partial-power `PPM` semantics as the foundation, rather
than older simple-modulus paths that do not support derived power formats.

Auxiliary digits are currently treated as redundancy and overflow-checking
plumbing, not ordinary range or precision extension. To gain more range or
precision, use a larger primary `RNSNumberSystem`.

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

Signed and fractional classes are documented separately as their verified
conversion slices develop.
