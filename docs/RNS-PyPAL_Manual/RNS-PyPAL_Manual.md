# Residue Number System

## General-Purpose, Arbitrary-Precision Arithmetic Library

# Tutorial, Guide, and Introduction to RNS-PyPAL

**Development Markdown edition**  
Adapted from *Tutorial, Guide and Introduction to RNS-APAL* (V0.200)

This manual is for people experimenting with the RNS-PyPAL Python library. It
preserves the historical RNS-APAL tutorial's progression and mathematical
purpose, but uses only the RNS-PyPAL API verified in this repository. Read the
adjacent [README.md](README.md) before relying on a section marked
**Placeholder**.

For the page-ordered historical prose retained from the supplied PDF, read
[RNS-PyPAL_Manual_Literal_Prose.md](RNS-PyPAL_Manual_Literal_Prose.md). Its
C++ source blocks are replaced by Python-conversion placeholders; this guide is
the corresponding verified Python-facing adaptation.

## Preface and Historical Context

RNS-PyPAL is the active Python port and experimentation environment derived
from the original C++ RNS-APAL work. RNS-APAL established the practical
research direction of general-purpose computation in residue number systems:
integer and fixed-point calculations should remain in the residue domain until
an explicit input, output, or diagnostic conversion boundary is reached.

The original manual is retained at
`cpp_ref/User_Guide/RNS-APAL_Manual_V0-200.pdf` as historical reference. The
C++ library, its algorithms, and its development history remain significant
evidence, but RNS-PyPAL does not claim every historical routine is complete or
mathematically accepted. Python examples below use the verified core only.

RNS is the native arithmetic domain in RNS-PyPAL. A `PPM`, `SPPM`, or `SPMF`
object is not a Python integer or float wrapped in residue digits. Python
integers, decimal text, and mixed-radix reconstruction are boundary tools for
initialization, output, diagnostics, and tests; they are not the implementation
of RNS arithmetic.

## Contents

1. [Setup and First Session](#setup-and-first-session)
2. [Project State and Performance Notes](#project-state-and-performance-notes)
3. [RNS Systems and Core Types](#rns-systems-and-core-types)
4. [The Fundamental Unsigned Residue Class: PPM](#the-fundamental-unsigned-residue-class-ppm)
5. [Basic PPM Arithmetic](#basic-ppm-arithmetic)
6. [Partial Powers, Modular Division, and Base Extension](#partial-powers-modular-division-and-base-extension)
7. [PPM Value Testing and Comparison](#ppm-value-testing-and-comparison)
8. [Signed Integer Residue Representation: SPPM](#signed-integer-residue-representation-sppm)
9. [Fractional Representation in Residues: SPMF](#fractional-representation-in-residues-spmf)
10. [Fixed-Point Arithmetic and Fractional Multiplication](#fixed-point-arithmetic-and-fractional-multiplication)
11. [Advanced Historical Algorithms](#advanced-historical-algorithms)

## Setup and First Session

Use the repository virtual environment from the repository root:

```powershell
.venv\Scripts\python.exe -m pytest
.venv\Scripts\python.exe -m ruff check rns_pypal tests
```

Expected output:

```text
pytest reports no failures
All checks passed!
```

An interactive session can import the verified public API directly:

```python
from rns_pypal import PPM, RNSNumberSystem, SPMF, SPPM

integer_system = RNSNumberSystem(
    name="tutorial-integers",
    moduli=(2, 3, 5, 7),
    powers=(3, 2, 1, 1),
)

print(integer_system.full_moduli)
```

Expected output:

```text
(8, 9, 5, 7)
```

Unlike the historical C++ program, there is no process-global initialization
call. Construct an immutable `RNSNumberSystem`, then give that system to every
numeric value. This allows several compatible or incompatible RNS systems to
coexist in one Python process without silently sharing global configuration.

## Project State and Performance Notes

The original RNS-APAL release recorded experimental algorithms alongside the
stable public paths. RNS-PyPAL keeps that history as evidence, but the Python
project treats only source-backed and test-backed behavior as current support.
The status of deferred work is maintained in `PROJECT_STATUS.md`, while
`ARCHITECTURE.md` defines the intended arithmetic and validation policy.

The same practical caution applies to performance. Parallel residue add,
subtract, and multiply are simple per-channel operations. Operations that
need ordering or format reconstruction, including comparison, normalization,
integer division, and diagnostic conversion, do more work through the
mixed-radix stream. Use correctness and explicit system geometry as the first
selection criteria; benchmark a concrete workload before drawing conclusions
about Python or hardware performance.

## RNS Systems and Core Types

An RNS system is defined by ordered base moduli and normalized powers. The
effective modulus of digit $i$ is $m_i^{p_i}$, and every effective modulus must
be pairwise coprime. The product of the value-digit moduli is the dynamic
range. Digit order is significant for mixed-radix decomposition.

```python
system = RNSNumberSystem(
    name="tutorial-fixed",
   moduli=(2, 3, 5, 7, 11, 13, 17),
   powers=(3, 2, 1, 1, 1, 1, 1),
    fractional_digits=2,
)

print(system.full_moduli)       # (8, 9, 5, 7, 11, 13, 17)
print(system.dynamic_range)     # 6126120
print(system.unit_range)        # 72
```

Expected output:

```text
(8, 9, 5, 7, 11, 13, 17)
6126120
72
```

The hierarchy is preserved as real Python inheritance:

```text
PPMDigit -> PPM -> SPPM -> SPMF
```

`PPMDigit` stores a residue and its per-digit effective-format state. `PPM`
is the unsigned residue integer. `SPPM` adds complement-based signed
interpretation and cached sign metadata. `SPMF` adds a fixed-point fraction
position over `SPPM`.

### Python Counterparts to ModTable and PPMDigit

The historical `ModTable` mixed system-wide geometry and arithmetic strategy
in C++ static configuration. Its RNS-PyPAL replacement is `RNSNumberSystem`.
It validates ordered moduli, powers, digit roles, pairwise coprimality, and the
fractional-position declaration once, then shares that immutable description
among values.

Each `PPMDigit` retains the residue and the state required by partial-power
algorithms: its index, base modulus, current `power`, available `power_valid`,
normalized `normal_power`, and skip state. A skipped digit has been consumed
by an algorithm and does not participate in the effective working format until
the appropriate extension or normalization operation restores it.

## The Fundamental Unsigned Residue Class: PPM

`PPM` holds one residue in every digit of its declared system. Assignment from
Python integers, numeric strings, hexadecimal strings, binary strings, and
compatible `PPM` values is supported. Ordinary assignment wraps in the RNS
range, matching fixed-width C/C++ style; `assign_checked()` explicitly checks
an external input range.

```python
value = PPM(1234, system=integer_system)
same_value = PPM("0x4d2", system=integer_system)

assert value.is_equal(same_value)
print(value.format_native())    # raw residues
print(value.format_value())     # explicit decimal boundary conversion
print(value.to_residues())      # plain residue tuple for inspection
```

Expected output:

```text
2 1 4 2
1234
(2, 1, 4, 2)
```

`format_native()` exposes the RNS word. `format_value(radix=2, 10, or 16)`
uses a conversion boundary for display. The returned text must not be confused
with the representation used by arithmetic methods.

### Native and Fixed-Radix Display

Raw display is deliberately explicit because a residue word is not a sequence
of decimal digits. `format_native()` prints active residues in RNS digit order;
an asterisk identifies a skipped digit and a partial-power marker identifies a
digit whose current valid power differs from its working power. `format_whdr()`
adds modulus-header information for experimental inspection. Auxiliary digits,
when present, are shown as a parenthesized suffix and do not define the ordinary
numeric range.

Fixed-radix output is an observation boundary. `format_value()` supports
binary, decimal, and hexadecimal for `PPM` and `SPPM`. The fallback conversion
uses mixed-radix reconstruction for output, not to implement any arithmetic.

### Random Test Assignment

`assign_rnd(num_digits, seed=...)` produces a pseudorandom decimal input and
then performs ordinary normalized assignment. It is useful for repeatable
examples, not for cryptography.

```python
value.assign_rnd(24, seed=20260726)
print(value.format_value())
```

Expected output:

```text
2151
```

## Basic PPM Arithmetic

Basic addition, subtraction, and multiplication are parallel-array
computations: each residue channel is updated independently modulo its current
effective modulus. They mutate the left-hand object, following the C++ class
style.

```python
left = PPM(900, system=integer_system)
right = PPM(125, system=integer_system)

left.add(right)
assert left.format_value() == "1025"
print(left.format_value())

left.sub(25)
left.mult(2)
assert left.format_value() == "2000"
print(left.format_value())
```

Expected output:

```text
1025
2000
```

The declared dynamic range is finite. Basic arithmetic does not automatically
detect whether a mathematical result fit in the intended application range;
residue channels wrap by design. Select a system with sufficient range or use
an explicit diagnostic feature.

### Unsigned Integer Division

`PPM.div_std(divisor)` is the first RNS-native unsigned integer division port.
It mutates the dividend into the quotient and returns a `PPM` remainder. The
algorithm requires a base-2 modulus in the system.

```python
dividend = PPM(1000, system=integer_system)
remainder = dividend.div_std(PPM(64, system=integer_system))

assert dividend.format_value() == "15"
assert remainder.format_value() == "40"
print(dividend.format_value())
print(remainder.format_value())
```

Expected output:

```text
15
40
```

## Partial Powers, Modular Division, and Base Extension

RNS-PyPAL preserves the original distinction between a normalized system and a
temporary derived format. A power-based digit can consume a factor of its own
base modulus, reducing `power_valid`; zero valid power skips the digit.
Remaining channels divide through modular inverse multiplication.

### Read the Working Number Format

This section is deliberately a format-reading exercise, not only an API
reference. Use `format_whdr()` whenever a partial-power operation changes a
working value. Its three lines show, in order, the **current effective modulus**
of each value digit, a state marker, and the stored residue. A normal active
digit has a `-` marker. A digit with fewer valid powers than its current working
power has an `=` marker. A skipped digit has `*` in the residue line.

The following small system has full moduli $(16, 9, 25)$. The value $42$ is
divisible by $2$, so its base-two digit may consume one factor of $2$.

```python
partial_power_system = RNSNumberSystem(
   name="partial-power-demo",
   moduli=(2, 3, 5),
   powers=(4, 2, 2),
)

value = PPM(42, system=partial_power_system)
print(value.format_native())
print(value.format_whdr())

value.mod_div(2)
print(value.format_native())
print(value.format_whdr())
assert value.format_value() == "21"
```

Expected output:

```text
10 6 17
16 9 25
-- - --
10 6 17

X|5 3 21
8 9 25
= - --
5 3 21
```

The first header says that the normalized base-two channel has modulus $2^4 =
16$. After `mod_div(2)`, that channel has effective modulus $2^3 = 8$, and its
residue is $5$, which represents the quotient $21$ modulo $8$. The other
channels did not lose powers: they were updated by modular inverse
multiplication, producing residues $3$ modulo $9$ and $21$ modulo $25$.

`X|5` in native format and `=` in the header are both warnings that this value
is in a temporary partial-power state. They do not mean that the value is
incorrect; they mean that its effective RNS format is no longer the full parent
system. Ordinary arithmetic is only valid with another value that has exactly
the same active digits and valid powers.

### Derived Format Versus Normalized Format

`derived_format_copy()` promotes the available powers to the copy's current
working powers while preserving `normal_power` as the route back to the parent
system. The header remains $(8, 9, 25)$, but the `=` marker disappears because
the derived copy now treats $8$ as its current base-two modulus.

```python
derived = value.derived_format_copy()
print(derived.format_native())
print(derived.format_whdr())

derived.normalize()
print(derived.format_native())
print(derived.format_whdr())
```

Expected output:

```text
5 3 21
8 9 25
- - --
5 3 21

5 3 21
16 9 25
-- - --
5  3 21
```

`normalize()` performs mixed-radix decomposition and RNS reconstruction to
base-extend the value back to the original full format. The represented value
remains $21$; only the available residue channels and their moduli change.

When a digit consumes all of its valid power, it is skipped rather than merely
marked partial. For example, dividing `PPM(6, system=RNSNumberSystem(moduli=(2,
3), powers=(1, 2)))` by $2$ displays:

```text
2 9
- -
* 3
```

The asterisk shows that the base-two channel has no effective modulus in this
working state. `extend_to_current_power()` can reconstruct that skipped residue
when the target is the current derived format; `normalize()` instead returns to
the original normalized parent system.

### Modular-Division Preconditions

`mod_div(divisor)` is a low-level RNS operation, not a general Python-integer
division shortcut. For every channel whose own base factor is consumed, the
value must be divisible by the requested divisor before the call. In the first
example, $42 \bmod 2 = 0$, so division by $2$ is legal. If the base-two residue
is odd, RNS-PyPAL raises `RNSCriticalError` and leaves the value unchanged.

Use `PPM.div_std()` for the separate RNS-native unsigned integer-division
algorithm. Its quotient/remainder contract and preconditions are different from
the derived-format transformation shown here.

Use `extend_to_current_power()` when a working value must be restored only to
its current derived power, and `normalize()` when it must be restored to the
parent system's full `normal_power`. Arithmetic between derived values requires
matching effective moduli and skip state; RNS-PyPAL raises a compatibility error
instead of silently mixing formats.

## PPM Value Testing and Comparison

Raw residue arrays are not positional digit arrays. A numerical comparison
uses streaming mixed-radix decomposition, evaluating digits from the declared
least-significant RNS position through the most-significant one.

```python
first = PPM(401, system=integer_system)
second = PPM(400, system=integer_system)

assert first.compare(second) == 1
assert not first.is_zero()
assert PPM(0, system=integer_system).is_zero()
print(first.compare(second))
```

Expected output:

```text
1
```

`MixedRadixDecomposer` yields the same ordering information without requiring
every high-level operation to materialize a complete positional number. `MRN`
is the stored mixed-radix form for experiments, debugging, and reconstruction.

## Signed Integer Residue Representation: SPPM

`SPPM` uses method-of-complements residues as the authoritative signed
representation. `sign_flag` and `sign_valid` are cached metadata, not a second
numeric representation. Zero is canonical positive with a valid sign.

### Signed Ranges and Sign Control

For a normalized unsigned value range $0$ through $M - 1$, the signed
complement interpretation divides the range into positive and negative regions.
RNS-PyPAL exposes the declared range with `SPPM.signed_range(system)` and can
perform a checked external assignment with `assign_checked()`. Checked
assignment is intentionally separate from ordinary arithmetic because range
checking is not free in a general RNS workflow.

The cached sign becomes invalid when a method cannot establish the result sign
without further RNS work. `calc_sign()` derives the sign from complement range;
`calc_set_sign()` also validates a previously claimed valid cache. A mismatch
between a valid cache and the encoded residue value is treated as a critical
condition rather than silently corrected.

```python
signed = SPPM(-120, system=integer_system)
increment = SPPM(45, system=integer_system)

signed.add(increment)
assert signed.format_value() == "-75"
assert signed.compare(SPPM(-80, system=integer_system)) == 1
print(signed.format_value())
print(signed.compare(SPPM(-80, system=integer_system)))
```

Expected output:

```text
-75
1
```

Use `SPPM.signed_range(system)` to inspect the representable complement range
and `assign_checked()` when boundary validation is needed. Ordinary signed
assignment follows the normal wraparound policy. Mixed-sign addition can make
the cached sign invalid; `calc_set_sign()` explicitly recovers and validates
that cache from the residue encoding.

> **Placeholder - signed division.** The historical manual's signed-division
> discussion remains a conceptual reference. RNS-PyPAL does not yet expose a
> verified signed division API.

## Fractional Representation in Residues: SPMF

`SPMF` represents a signed fixed-point value as a complement-based scaled
integer. The first `fractional_digits` value positions form the fractional
range $RF$ (called `unit_range` in Python):

$$
\text{represented value} = \frac{\text{scaled integer}}{RF}.
$$

```python
fraction = SPMF(0, system=system)
fraction.assign_decimal("100.25")

print(fraction.format_decimal())
print(fraction.format_ratio())
print(fraction.format_fixed_native())
```

Expected output:

```text
100.25
7218/72
2 0 . 3 1 2 3 10
```

`assign_decimal()` and `assign_ratio()` are deliberately documented
input-boundary helpers. Their current implementation may use Python
arbitrary-precision integer arithmetic to scale external text into the RNS
format. This does not change the requirement that arithmetic on existing SPMF
operands remain RNS-native.

### Printing Fixed-Point Residues

`format_decimal()` is a diagnostic decimal view with a selectable maximum
number of fractional digits. `format_ratio()` exposes the exact boundary form
`scaled_integer/unit_range`, which is often the better way to inspect a value
whose fractional range is not a power of ten. `format_fixed_native()` displays
the underlying residue digits and places a marker after the fractional RNS
positions.

These output methods are intentionally separate from the fixed-point arithmetic
methods. They help an experimenter inspect the representation without making
decimal or Python integer arithmetic part of the calculation itself.

### Range and Precision Requirements

The fixed-point scale is determined by the product of the fractional effective
moduli. Moving more RNS digit positions into the fractional range generally
increases fractional resolution while reducing the remaining whole-number
range. The system must accommodate both the intended final values and the
largest intermediate product required by the algorithm. Overflow checking is
not automatic.

The current fraction point is metadata on the live value. `set_fraction_position()`
is plumbing for internal sliding-point algorithms and does **not** scale the
residue word. Public same-format arithmetic requires compatible current fraction
positions.

## Fixed-Point Arithmetic and Fractional Multiplication

Same-format `SPMF.add()`, `sub()`, and `compare()` delegate to signed residue
arithmetic after validating systems, effective formats, and fraction positions.
Whole integers may be added or subtracted with `add_whole()` / `sub_whole()`.

```python
left = SPMF(0, system=system)
right = SPMF(0, system=system)
left.assign_decimal("100.25")
right.assign_decimal("2.5")

left.add(right)
left.sub_whole(1)
assert left.format_decimal() == "101.75"
print(left.format_decimal())
```

Expected output:

```text
101.75
```

For same-format fractional multiplication, `mult_std()` follows the initial
RNS-APAL `Mult4b`-style path: it forms an RNS raw product, streams
mixed-radix digits to discard fractional positions, performs the documented
rounding comparison, and applies the signed correction constant when needed.

```python
left = SPMF(0, system=system)
right = SPMF(0, system=system)
left.assign_decimal("12.5")
right.assign_decimal("2.0")

left.mult_std(right)
assert left.format_decimal() == "25."
print(left.format_decimal())
```

Expected output:

```text
25.
```

The raw intermediate product must fit in the selected RNS geometry before
fractional normalization. Choosing an RNS system that fits each operand is not
enough; it must also have enough raw range for their product. Integer
multiplication uses `mult_integer()` and does not require fractional
renormalization.

## Advanced Historical Algorithms

The source manual documents multiply-accumulate, raw-product summation,
fractional division and inverse, scaling, Goldschmidt division, and a Newton
square-root routine. They remain important research and design material, but
they are not presented as runnable RNS-PyPAL code yet.

### Multiply-Accumulate and Product Summation

> **Placeholder.** RNS-PyPAL has RNS-native `PPM` and `SPMF` multiplication
> primitives, but it has no dedicated public MAC or raw-product accumulation
> API. Do not treat a manual sequence of raw operations as equivalent to the
> C++ container routines without a dedicated algorithm and tests.

### Fractional Division, Inverse, and Goldschmidt Scaling

> **Placeholder.** `SPMF` fractional division, inverse, scaling/normalization,
> and Goldschmidt methods are deferred. The Python API currently exposes only
> fraction-point metadata controls; it does not provide a validated operation
> that rescales residues when the point moves.

### Newton Square Root

> **Placeholder.** RNS-PyPAL does not yet provide a validated `SPMF.sqrt()`.
> The historical Newton method remains a future high-level algorithm because it
> depends on the unimplemented fixed-point division contract.

## Experimentation Checklist

1. Declare a small, explicit `RNSNumberSystem` for the experiment.
2. Use `PPM` first; validate residue behavior before relying on `SPPM` or `SPMF`.
3. Keep arithmetic inside the RNS object methods.
4. Use `format_value()`, `format_decimal()`, `format_ratio()`, and `to_dict()`
   only as observation boundaries.
5. Confirm an example against the numbered test hierarchy and the focused
   notebook for the same numeric class.
6. Consult [README.md](README.md) for every capability difference from the
   historical C++ manual.

## License and Source Note

This Markdown manual is a derivative documentation work based on the original
RNS-APAL manual and source retained in this repository. The original work is
identified in `ARCHITECTURE.md` as CC BY-NC-SA; preserve the applicable terms
when redistributing this adaptation.