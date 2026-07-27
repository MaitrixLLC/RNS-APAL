# RNS-PyPAL Project Status

## Session Startup

Read this file after `AGENTS.md` and `ARCHITECTURE.md` at the beginning of a
repository session. Treat it as a bounded task queue, not as mathematical
specification. Verify every task against the current code, tests, architecture,
and C++ reference before implementing it.

## Current Future Work

### PPM Routines Identified in C++ Review

The following C++ `PPM` routines have no Python `PPM` counterpart after
excluding redundant, historical, clock-counting, and explicitly experimental
paths.

1. `PPM::GetRange(PPM *val, int num_digits)`
   - Add an RNS-native prefix-range constructor for the product of the first
     `num_digits` full digit moduli.
   - This is used by the C++ `SPPM` and `SPMF` paths for signed and fractional
     range values.
   - Reference: `cpp_ref/ppm.cpp`, `PPM::GetRange`.

2. `PPM::GetFullRange(PPM *ppm)`
   - Add a derived-format-aware maximum-range operation. It must compute the
     largest representable value in the live effective format, not only in the
     declared normalized `RNSNumberSystem`.
   - Reference: `cpp_ref/ppm.cpp`, `PPM::GetFullRange`.

3. `PPM::Zero(int start_index)`
   - Add, or deliberately reject with documentation, an active-digit suffix
     zero predicate beginning at a specified index.
   - Reference: `cpp_ref/ppm.cpp`, `PPM::Zero(int)`.

4. `PPM::TruncateFirst(PPM *ppm, int numdigs)`
   - Clarify the C++ contract before porting. The implementation retains the
     first low-order RNS positions and base-extends, but it does not use its
     `ppm` parameter and has no repository call sites.
   - Do not port until the intended source/destination semantics and exact
     truncation invariant are established with focused tests.
   - Reference: `cpp_ref/ppm.cpp`, `PPM::TruncateFirst`.

## Deferred C++ Paths

Do not treat the following as current Python porting tasks without a separate
review: alternate `DivPM*` algorithms, `CompareDif`, `ModDiv2`, `Cfr`, clock
counters, backup/restore, historical conversion routines, `Factorial`,
`Sqrt`, and `GetMultRange`. They are redundant, historical, experimental, or
depend on unverified research algorithms.

## Verification Rule

For any task above, first add exact tests for normalized, partial-power,
skipped-digit, and boundary cases that apply. Preserve RNS-native arithmetic;
do not implement a new arithmetic operation with positional
convert-operate-reencode shortcuts.
