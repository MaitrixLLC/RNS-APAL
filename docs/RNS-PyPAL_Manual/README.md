# RNS-PyPAL Manual Conversion Notes

## Purpose

[RNS-PyPAL_Manual.md](RNS-PyPAL_Manual.md) is the development Markdown
counterpart to `cpp_ref/User_Guide/RNS-APAL_Manual_V0-200.pdf`. It preserves the
source manual's educational sequence and historical significance while using
the verified RNS-PyPAL public API for examples.

[RNS-PyPAL_Manual_Literal_Prose.md](RNS-PyPAL_Manual_Literal_Prose.md) retains
the page-ordered non-code prose from the supplied PDF. Its detected C++ source
blocks are replaced with explicit Python-conversion placeholders. Read it with
the primary manual: the literal companion preserves historical context, while
the primary manual explains verified RNS-PyPAL behavior.

The Markdown manual is not a claim of feature parity with the C++ source. The
table below records material discrepancies at conversion time.

## Editorial Purpose

The manual is both a standard reference and a teaching document. Explanations
should show how an RNS representation changes during an operation, not merely
list method names and results. For partial-power, skipped-digit, normalization,
mixed-radix, signed, and fixed-point topics, prefer a small reproducible trace
that displays the live format, operands, effective moduli, and restoration path.
Use the raw/header display methods where they make derived RNS formats visible.

Every runnable Python example in `RNS-PyPAL_Manual.md` must be followed by an
**Expected output** block. Assertion-only examples should print the observed
result as well as check it, so a reader can compare a live session to the
manual without inferring hidden state. Keep output blocks adjacent to their
examples. When output includes `format_whdr()` or another multi-line raw
representation, use a blank line between distinct states so the modulus header,
state-marker row, and residue row remain legible.

## Verified Conversion Basis

- Architecture: `ARCHITECTURE.md`
- Current task queue: `PROJECT_STATUS.md`
- Public API: `rns_pypal/__init__.py`
- Executable evidence: `tests/test_01_modular_helpers.py` through
  `tests/test_07_spmf.py`
- Historical source manual: `cpp_ref/User_Guide/RNS-APAL_Manual_V0-200.pdf`

## Discrepancies and Placeholders

| Historical manual topic | RNS-PyPAL status | Manual treatment |
| --- | --- | --- |
| Global `init_RNS_APAL(...)` setup | Replaced by explicit immutable `RNSNumberSystem` objects. | Python setup and examples. |
| C++ pointers, `new`, and console print methods | Replaced by ordinary Python objects and `format_*` / `print_*` methods. | Python examples. |
| `PPM` assignment, conversion, basic arithmetic, comparison, partial powers, base extension | Implemented and covered by the PPM test layer. | Python examples. |
| Unsigned `PPM::DivStd` | Implemented as `PPM.div_std()`; base-2 system required. | Python example and precondition note. |
| Historical `GetRange`, `GetFullRange`, `Zero(start_index)`, and `TruncateFirst` | Deferred in `PROJECT_STATUS.md`. | Not exposed as Python APIs. |
| Signed assignment, comparison, add/subtract/multiply, sign cache | Initial `SPPM` slice is implemented. | Python examples and cache caveat. |
| Signed division | Not implemented. | Section-level placeholder. |
| Decimal/ratio input and diagnostic output for `SPMF` | Implemented as preliminary trap-door I/O. | Explicit boundary-layer note. |
| Same-format `SPMF` add/subtract/compare/multiply and integer mixed operations | Implemented first slice; multiplication has raw-capacity and system compatibility requirements. | Python examples and limitations. |
| MAC, product summation, fractional division/inverse, scaling, Goldschmidt routines, square root | Deferred or not yet validated in the Python port. | Explicit placeholders in their historical sections. |
| Auxiliary digit error correction and signed/fixed-point auxiliary support | Metadata and limited unsigned prototype support only. | Not used by manual examples. |

## Known Source Conflict

The historical Rez-9 fixed-point geometry is not used in examples. Active C++
sources disagree about whether it contains seven or eight fractional digits.
The Python bundled `REZ9_INTEGER_SYSTEM` intentionally declares an integer
system only. The manual therefore uses small, explicit tutorial systems whose
geometry is visible next to each experiment.

## Validation Status

The manual's Python snippets should be validated after each content revision
with the repository virtual environment. The manual itself does not replace
the complete pytest hierarchy, the focused notebooks, or mathematical review.

Run the standard check from the repository root:

```powershell
.venv\Scripts\python.exe -m pytest
.venv\Scripts\python.exe -m ruff check rns_pypal tests
```

## Next Documentation Work

When a deferred Python feature becomes source- and test-backed, replace its
manual placeholder and update this table in the same change. Keep historical
C++ algorithms and current RNS-PyPAL behavior distinct until that evidence
exists.