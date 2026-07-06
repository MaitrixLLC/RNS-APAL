
# RNS-PYPAL

RNS-PYPAL is a Python port of the original C++ RNS-APAL residue number system library. The initial goal is mathematical and behavioral fidelity for the stable core, followed by carefully tested Python-specific improvements.

This document describes the intended architecture. The C++ headers and implementations are essential reference evidence, but they are not assumed to be defect-free. Stable active behavior, mathematical invariants, and explicit equivalence tests determine what the Python library should preserve.

The original C++ RNS-APAL source and Visual Studio project files are kept under
`cpp_ref/`. Future source analysis should look there first, for example
`cpp_ref/ppm.cpp`, `cpp_ref/ppm.h`, `cpp_ref/mrn.cpp`, `cpp_ref/sppm.cpp`, and
`cpp_ref/spmf.cpp`. The root of the repository is reserved for the Python port,
tests, documentation, notebooks, and project tooling.

## Project goals

- Provide an importable Python library for residue number system (RNS) arithmetic.
- Preserve the original numeric hierarchy:
  - `PPM` -- unsigned residue integers
  - `SPPM` -- signed residue integers
  - `SPMF` -- signed fixed-point residue values
- Preserve partial-power and derived-number-system behavior.
- Execute numeric arithmetic in the residue domain without positional convert-operate-reencode shortcuts.
- Add explicit system metadata and operand compatibility checks.
- Allow multiple RNS systems to coexist in one Python process.
- Port and verify the library in small, independently tested slices.

## RNS-primary viewpoint

RNS-PYPAL treats the residue number system as the primary computational number
system. It does not treat RNS as a foreign encoding of binary integers, decimal
integers, or mixed-radix values.

This viewpoint is a central design rule for the library. A `PPM` value is not
"really" a Python integer stored in residues. It is an unsigned RNS value. An
`SPPM` value is not "really" a signed binary integer stored in residues. It is
a signed RNS value interpreted through method-of-complements rules. Later,
`SPMF` values should likewise be fixed-point RNS values, not positional numbers
temporarily hidden inside an RNS container.

Mixed-radix conversion should therefore be described as decomposition, not as a
return to the true representation. It is an RNS algorithmic tool used when an
operation needs ordered digit significance: comparison, normalization,
reconstruction, rounding, division support, fractional multiplication, or
output conversion. A mixed-radix digit stream is a way for RNS computation to
expose ordered information while still controlling the operation from the RNS
side.

Likewise, decimal, hexadecimal, binary text, and Python arbitrary-precision
integers are boundary representations. They are useful for initialization,
printing, debugging, tests, and human inspection. They are not the mathematical
ground truth of an RNS-PYPAL value, and they must not become the hidden engine
of arithmetic.

RNS-PYPAL values should nevertheless be easy to export into ordinary Python
data structures. Class-based value formats may expose residue arrays, digit
records, dictionaries, and later structured serialization forms for notebooks,
analysis tools, file formats, and interoperability. These exports are boundary
views of the RNS state. They must preserve enough metadata to avoid ambiguity
about system geometry, digit order, skipped digits, partial powers, auxiliary
digits, and signed metadata, but they do not redefine the value as a positional
number.

This mirrors the historical maturation of binary computing. Early computers
needed decimal/binary conversion at their boundaries, but binary eventually
became understood as the machine's native arithmetic domain and decimal became
primarily an input/output concern. RNS-PYPAL adopts the analogous stance for
residue arithmetic: RNS is the native domain; conversion and decomposition are
supporting methods.

## Scientific and algebraic interoperability

A key target of RNS-PYPAL is to let RNS values participate in high-level Python
algebraic, scientific, and matrix-oriented workflows while preserving
RNS-native arithmetic semantics.

Ordinary Python containers such as `list[PPM]`, `list[list[PPM]]`, dictionaries,
and later optional array-library adapters should be able to organize RNS
values. External libraries may provide structure, storage, iteration,
visualization, scheduling, and algorithm orchestration. They must not silently
replace RNS arithmetic with positional integer or floating-point arithmetic.

The scalar operation is the semantic boundary. If a high-level vector, matrix,
or algebraic routine performs addition, subtraction, multiplication,
comparison, division, scaling, rounding, or fixed-point work on RNS values, that
operation must dispatch to the appropriate RNS-PYPAL scalar method or to an
RNS-PYPAL-controlled vector/matrix helper built from those scalar methods.

This makes three interoperability layers acceptable:

1. plain Python containers that hold `PPM`, `SPPM`, and later `SPMF` values;
2. RNS-PYPAL helper routines for vectors, matrices, dot products, transforms,
   and other structured calculations; and
3. optional adapters for external libraries, provided they preserve RNS-PYPAL
   arithmetic dispatch and do not coerce values into Python `int`, `float`, or
   another positional type except at explicit conversion boundaries.

This rule keeps the library useful to Python researchers without surrendering
the RNS-primary design. High-level libraries may organize RNS computation; they
do not define the arithmetic ground truth.

## Python development environment

The Python port is configured through `pyproject.toml`. This file is the
project-level definition for the test and lint tools; it is separate from the
future TOML files that will define individual RNS number systems.

The recommended development environment is:

- Python 3.12 or newer
- `pytest` for equivalence and regression tests
- `ruff` for lightweight linting
- optionally `ipython` for interactive exploration
- optionally `jupyterlab` and `ipykernel` for web-based notebooks

From an activated virtual environment, the ordinary verification commands are:

```powershell
python -m pytest
python -m ruff check rns_pypal tests
```

For notebook-based exploration, install the optional exploration tools and
launch JupyterLab from the repository root:

```powershell
python -m pip install ipython ipykernel jupyterlab
python -m jupyter lab
```

The starter notebook is `notebooks/rns_pypal_scratch.ipynb`. It imports the
local library, runs the current test suite, and shows basic `PPM` native/demo
formatting. Notebook output should remain scratch state; committed notebooks
should generally avoid saved execution noise unless the output is intentionally
part of documentation.

If a notebook cell is copied into another notebook, it should locate the
repository root before importing local code:

```python
from pathlib import Path
import sys
import pytest

def find_repo_root(start=None):
    path = Path.cwd() if start is None else Path(start).resolve()
    for candidate in (path, *path.parents):
        if (candidate / "rns_pypal").exists() and (candidate / "tests").exists():
            return candidate
    raise RuntimeError("Could not find the RNS-PYPAL repository root")

repo_root = find_repo_root()
if str(repo_root) not in sys.path:
    sys.path.insert(0, str(repo_root))

result = pytest.main([str(repo_root / "tests")])
assert result == 0
```

The current `pytest` configuration runs the `tests` directory quietly and
disables pytest's cache provider because this workspace may reject writes to
`.pytest_cache`. The current Ruff configuration intentionally stays modest. It
checks for ordinary Python errors and unused imports, but it does not yet force
large modernization or import-sorting changes across dormant prototype modules.

## Original C++ architecture

### Global system configuration

The C++ `ModTable` stores most configuration in static state. Its constructor receives:

- mode and routine selectors
- requested total digit count
- requested fractional digit count
- base modulus values
- modulus power values

Consequently, the original implementation is effectively configured around one active RNS system at a time. Constants and compile-time options in `config.h` also select custom versus generated moduli, power-based formats, and modular-division strategies.

Later C++ initialization added `init_RNS_APAL(...)`, which accepts the system geometry at runtime. That interface represents the newer intended direction and can override the older header-oriented workflow in application code. However, the present custom-system path in `ModTable` still reads global `Modulus` and `ModPowers` arrays in places instead of consistently using the arrays passed to its constructor. The port must therefore distinguish the upgraded interface's intent from accidental dependence on legacy globals.

### `PPMDigit`

Each RNS digit contains more than a residue value. The C++ class tracks:

- `Index` -- position in the RNS digit array
- `Digit` -- residue value
- `DigitCopy` -- backup used by destructive operations
- `Skip` -- whether algorithms should omit the digit
- `Modulus` -- base modulus
- `Power` -- normal modulus power for the current format
- `PowerValid` -- currently valid portion of that power
- `NormalPower` -- original normalized power used when restoring format

The full modulus of a digit is `Modulus ** Power`; its current effective modulus is `Modulus ** PowerValid`. Reducing `PowerValid` creates a partial-power digit. Reaching zero causes the C++ implementation to skip the digit until base extension restores it.

### `PPM`

`PPM` owns an ordered array named `Rn` containing `PPMDigit` objects. It provides unsigned assignment, conversion, comparison, digit-wise modular arithmetic, modular division, integer division, truncation, normalization, and base extension.

The simple add, subtract, and multiply operations use parallel-array computation: the operation is applied independently to each active residue digit. Operations involving partial powers must also preserve the derived format and skip state.

The C++ class contains several experimental algorithms and hardware-performance counters. These are not all required for the first faithful Python core; the stable public entry points should be identified before their experimental alternatives are ported.

### Normalized and derived RNS formats

An RNS value cannot be interpreted independently of its number system. Its system identity includes, at minimum, the ordered base moduli and their normalized powers. Two residue arrays with different effective modulus sets are not directly compatible merely because they contain the same number of digits.

RNS-APAL deliberately permits an operation to change the working number system temporarily. In a power-based system, a digit whose normalized modulus is `m ** p` may be reduced to `m ** q`, where `0 < q < p`. A digit can also be removed from the current working modulus set by reducing its valid power to zero or marking it skipped. These partial-power and truncated-modulus states form a derived RNS format with a smaller dynamic range.

The C++ digit fields distinguish three related states:

- `NormalPower` identifies the full power belonging to the originally instantiated system.
- `Power` identifies the full power of the current base or derived format.
- `PowerValid` identifies the power currently available during a lower-level operation.

Normally all three are equal. Division, scaling, mixed-radix decomposition, and related algorithms can reduce `PowerValid`. `AssignPM` can make the source's valid-power structure the destination's temporary `Power` structure while retaining `NormalPower` as the route back to the original system. A valid power of zero removes that digit from the current format and sets its skip state.

These derived formats are intentional working representations, not independent public results. They let low-level algorithms consume modulus factors, truncate range, and operate on reduced systems without attempting to make every possible partial format a permanent application-level number type.

The normal lifecycle is:

1. instantiate a value in a declared full-power normalized system;
2. enter one or more compatible derived formats inside an advanced operation;
3. track the effective powers and skipped positions while that operation is in progress;
4. reconstruct missing residues through mixed-radix conversion and base extension;
5. restore every digit to its original `NormalPower`; and
6. return the public result in the full-power normalized system.

`ExtendPart2Norm()` restores skipped and partial digits to the current format's `Power`. `Normalize()` goes further: it restores `Power` and `PowerValid` to `NormalPower` and base-extends the value into the originally instantiated full system. The Python port must preserve this distinction even if its eventual method names differ.

Arithmetic between two working values is valid only when their effective formats are compatible: corresponding base moduli, valid powers, and active/skipped positions must describe the same residue channels. Normalized values additionally require the same declared system identity. Compatibility must therefore be checked at both the system level and, when derived values are exposed internally, the effective-format level.

### Mixed-radix conversion and `MRN`

Residue digits are non-positional: the numeric ordering of residue digits at any one modulus does not establish the ordering of the represented integers. General comparison therefore cannot lexicographically compare the raw `Rn` arrays. The values must first be resolved into an ordered representation, and RNS-APAL does this through mixed-radix conversion.

The C++ `MRN` and `MRDigit` classes provide an explicit, stored mixed-radix representation. They support conversion to and from `PPM`, integer conversion, skipped and partial-power positions, and mixed-radix shifting. The standalone class is useful for exposition and for operations that need to retain or reconstruct the converted value, but the conversion principle is also embedded directly in several `PPM` and `SPMF` algorithms.

RNS-APAL performs the forward conversion from least-significant mixed-radix position to most-significant position. At each active digit position it:

1. reads the current residue at that position as the next mixed-radix digit;
2. subtracts that digit from the working residue number, making the value divisible by the position's effective modulus;
3. divides the remaining active residue channels by that modulus using inverse modular multiplication;
4. skips the consumed position and continues with the reduced working value.

The division in step 3 is performed independently in the remaining residue channels. If the consumed radix is `m` and a remaining channel has modulus `n`, division by `m` is multiplication by `m`'s modular inverse modulo `n`. Pairwise-coprime moduli make that inverse available. This per-channel inverse multiplication is the engine of the mixed-radix decomposition.

The Python port represents this with two related tools:

- `MixedRadixDecomposer` is the streaming helper. It owns a working copy of a
  `PPM`, produces one `MRDigit` at a time, invalidates the consumed RNS digit by
  marking it skipped, and updates the remaining residue channels through
  inverse modular multiplication. This is the intended primitive for compare,
  rounding, division, and any operation that should evaluate and discard each
  mixed-radix digit.
- `MRN` is the materialized mixed-radix class. It consumes the same streaming
  helper but stores every produced `MRDigit` for demonstration, debugging, and
  algorithms that genuinely need a retained mixed-radix representation.

Both forms use the order of the `PPM.rn` digit list. That list is an ordered
representation of the declared RNS system even though the residue digits are not
themselves positional values. Mixed-radix conversion therefore depends on this
declared order: digit index zero is consumed first, then index one, and so on.
Changing the digit order changes the mixed-radix number system being used. This
will be especially important for fixed-point values, where fractional RNS digits
must be converted before integer-position digits.

### Streaming mixed-radix processing

A central RNS-APAL technique is that the complete mixed-radix number often does not need to be stored. Each digit can be evaluated as soon as it is produced and then discarded. Only the reduced residue state and a small amount of operation-specific state need to continue to the next position.

`PPM::Compare` demonstrates this directly. It decomposes two working copies in parallel and updates one comparison state as each pair of mixed-radix digits is exposed. A later non-equal digit overrides the decision from less-significant positions. Conversion can terminate when a reduced operand reaches zero; if both terminate together, the last non-equal mixed-radix digit determines the ordering. Thus comparison uses digit-by-digit mixed-radix conversion without constructing two stored `MRN` objects.

The same streaming pattern supports more complex operations:

- `ComparePart` evaluates only a selected number of mixed-radix positions.
- Integer division repeatedly uses comparisons produced by this ordered conversion process.
- Fractional division uses comparison to decide remainder rounding.
- Fractional multiplication variants discard fractional mixed-radix positions, evaluate discarded positions for rounding, and recompose retained positions into an RNS result as decomposition proceeds.
- Signed fractional multiplication can decompose a product and its complement in parallel, tracking sign and rounding decisions without retaining every converted digit.

This is more than a memory optimization. It is an architectural rule for faithful implementations of comparison, rounding, division, and fractional multiplication. Python implementations should expose a reusable streaming mixed-radix iterator or equivalent internal primitive, while materializing a complete mixed-radix value only when an operation actually requires one.

### `SPPM`

`SPPM` derives from `PPM` and adds signed integer behavior through two fields:

- `SignFlag` -- cached sign-magnitude flag (`POSITIVE` or `NEGATIVE`)
- `SignValid` -- whether that cached sign is currently trustworthy

The residue digits remain the authoritative complement-based representation. The sign flag is a performance shortcut, not a replacement representation. Addition and subtraction can invalidate the cached sign. Methods such as `CalcSign` and `CalcSetSign` recover it from the signed range, while multiplication can derive a result sign from valid operand signs subject to overflow constraints.

Zero is canonicalized as positive with a valid sign.

For RNS-PYPAL, the method-of-complements encoding is always the ground truth.
The sign flag is cached metadata used for speed, consistency checks, and
algorithm dispatch. It may be omitted, invalidated, recomputed, or validated,
but it must never replace the encoded residue value as the actual signed
representation.

The coexistence of complement encoding and a sign-magnitude-style flag is
intentional redundant notation. Positional number systems carry ordering and
weight information directly in their digit positions, which makes comparison,
sign interpretation, and range reasoning comparatively natural. RNS digits do
not carry positional weights, so those operations require additional structure,
typically mixed-radix conversion, comparison ranges, or explicit metadata.

The cached sign flag provides some of that control without changing the
underlying RNS arithmetic. When valid, it can speed signed dispatch and avoid
unnecessary sign recovery. When checked against the complement-encoded value, it
can detect invalid cached metadata, arithmetic errors, and in some cases
overflow-like conditions. Thus the two sign notations are not competing
representations; they are cooperating layers: complement residues define the
number, while the redundant sign flag helps control and validate operations.

Sign-producing and sign-validating operations:

- Sign validation, sign extension, or sign recovery explicitly computes the
  sign from the complement-encoded value and marks the cached sign valid.
- Signed fractional multiplication generates the sign bit as part of the
  operation. In that operation, the sign is not merely preserved; it is produced
  by the algorithm.
- Compare-style operations may later detect disagreement between a cached sign
  flag and the complement-encoded value. If the cached sign is marked valid,
  such disagreement is a critical error, not a reason to silently reinterpret or
  correct the value. If the cached sign is invalid, the operation may explicitly
  recover the sign from the complement encoding and mark it valid.

Initial sign-flag disposition rules should follow the library's C/C++-style
range policy: no automatic overflow checking is performed by ordinary
arithmetic. Under that policy:

| Operation | Cached sign disposition |
| --- | --- |
| assignment from signed input | compute and mark valid |
| checked assignment from signed input | explicitly validate external range, then assign |
| copy | copy sign flag and validity |
| explicit sign validation/recovery | compute from residues and mark valid |
| positive + positive, both signs valid | preserve valid positive sign, assuming no overflow |
| negative + negative, both signs valid | preserve valid negative sign, assuming no overflow |
| add with mixed signs | sign generally becomes invalid unless the operation explicitly computes it |
| add when either input sign is invalid | result sign invalid unless explicitly computed |
| subtraction | same policy as addition after applying the mathematical sign reversal of the subtrahend |
| multiplication, both signs valid | compute result sign by ordinary sign multiplication |
| multiplication when either input sign invalid | result sign invalid unless explicitly computed |
| negation, sign valid | flip sign except canonical zero remains valid positive |
| normalization/base extension | preserve sign only when the operation guarantees the encoded value is unchanged; otherwise invalidate or explicitly recompute |

These rules deliberately separate sign propagation from overflow detection. For
example, positive plus positive may preserve a valid positive sign under the
same assumption made by C/C++ integer arithmetic: the caller is responsible for
knowing whether the operation stayed in range. A later explicit sign validation,
comparison consistency check, or redundant/overflow-check digit routine may
detect that the cached sign and complement encoding disagree. That detection is
valuable because it can reveal arithmetic errors, invalid cached metadata, or
overflow conditions. When the cached sign was marked valid, the disagreement
must be treated as a critical error.

Ordinary signed assignment from an outside positional value follows the same
wraparound policy as C/C++ integer assignment into a fixed-width type.
Conservative callers that need boundary enforcement should use an explicit
checked assignment path. Checked assignment validates the external value against
the signed complement range before encoding. The diagnostic severity should be
selectable: warning and continue, recoverable error, or critical error.

### `SPMF`

`SPMF` derives from `SPPM` and interprets part of the residue system as fixed-point fractional range. It tracks:

- `NumFractDigits` -- current radix-point position
- `NormalFractDigits` -- normalized radix-point position

Fractional digits are the first digits in `Rn`. The fractional range is
therefore the least-significant index range used by the default mixed-radix
decomposition order. This is intentional: fixed-point algorithms must be able
to consume fractional digits first before moving into whole/integer-range
digits.

`SPMF` also carries the concept of a sliding point. The normalized fraction
position belongs to the declared `RNSNumberSystem`; the current fraction
position belongs to the live `SPMF` value. Internal algorithms may temporarily
move the current fraction point during scaling, fractional multiplication,
division, rounding, or conversion between fixed-point formats. The conservative
first policy is that public top-level operations should return results to the
normal fraction position unless an API explicitly promises a sliding-point
result.

Arbitrary sliding-point arithmetic requires alignment rules similar in spirit
to floating-point exponent alignment. Addition, subtraction, comparison, and
other operations that depend on fixed-point scale must check that operands have
compatible current fraction positions before the operation begins. If positions
do not match, the operation must explicitly scale/align one or both operands
through RNS operations or raise a clear error. It must not silently treat two
different fixed-point positions as equivalent.

`SPMF` adds decimal and ratio assignment, fixed-point printing, fractional
multiplication, scaling, division, inverse, normalization, and square-root
routines. The C++ header contains multiple research versions of several
algorithms; application-facing container methods such as `MultStd` should guide
which implementation is treated as canonical.

Early `SPMF` input/output support may use a deliberately marked trap-door
conversion path based on Python arbitrary-precision integers and strings. This
is acceptable for fixed-point assignment, diagnostic printing, and research
inspection because I/O is a boundary layer. For example, a decimal or rational
input may be scaled into the fractional range as a positional integer and then
encoded into RNS residues. Reverse conversion may interpret the scaled
fixed-point residue value as an integer numerator over the fractional unit
range and format that value for display.

This trap-door I/O path does not weaken the RNS-native arithmetic rule. It must
not be used to implement fixed-point addition, subtraction, comparison,
multiplication, division, scaling, square root, or other mathematical
operations. It exists so researchers can exercise many RNS geometries before
specialized RNS-native conversion routines are available. As specialized
conversion algorithms are ported from C++ or developed in Python, I/O dispatch
should become capability-based: prefer a proven RNS-native conversion when the
modulus set supports it, otherwise fall back to the documented trap-door
conversion path.

The basic fixed-point identity is:

```text
unit_range = product(current fractional digit moduli)
scaled_integer = whole_part * unit_range + fractional_numerator
represented_value = scaled_integer / unit_range
```

For signed values, sign handling is performed first through the `SPPM`
method-of-complements interpretation; the separation formulas then apply to the
positive magnitude.

From the RNS-primary point of view, extracting the fixed-point pieces is an RNS
decomposition problem. The fractional numerator is the integer represented by
the fractional digit range. It can be obtained by retaining/reconstructing the
mixed-radix contribution of the fractional digits while the whole-range digits
are ignored or skipped for that extraction. The whole part is obtained by
consuming the fractional digits through the same digit-by-digit mixed-radix
reduction process. Those consumed fractional digits become skipped; the
remaining whole-range state is then base-extended/reconstructed into an integer
RNS value that is no longer scaled by `unit_range`.

The C++ `SPMF::FracSep` routine is the reference evidence for this idea: it
performs mixed-radix-style reduction, accumulates fractional digits into one
result, accumulates whole digits into another result, and uses the current
partial-power moduli for the digit weights. Python should preserve that
interpretation even when an early diagnostic I/O fallback uses Python integers
to format the final pieces.

Fractional multiplication is one of the first major `SPMF` algorithms that must
use this decomposition machinery carefully. Conceptually, the raw operation
contains an integer multiplication followed by scaling out one extra copy of the
fractional range. That scaling resembles fractional-to-whole separation, but it
is not merely an I/O conversion: it is part of the arithmetic needed to return
the product to the declared fixed-point scale.

The standard same-format multiply has an important capacity rule. Because the
fixed-point values are multiplied through an integer-style residue product
before fractional normalization, the available raw RNS range must be large
enough to hold that intermediate product. As a rule of thumb, preserving the
same number of fractional digits and whole digits in the final result requires
an RNS format whose raw range supports the square of the intended fixed-point
value range. Equivalently, the system geometry defines the largest fixed-point
fractional value that can be multiplied by itself and still fit before
normalization scales the product back down. This mirrors binary fixed-point
hardware/software practice: enough product bits must exist during the multiply
before the result is shifted or rounded back into the destination format.

Future advanced APIs may support a larger internal RNS work format for
fractional multiplication, then normalize or cast the result back into the
public `SPMF` format. That should be a separate, explicit operation. The
baseline `SPMF` multiply should assume same-format raw capacity unless a method
documents otherwise.

Rounding is also driven by the mixed-radix digit stream. As fractional positions
are reduced, the discarded fractional portion is compared against the half-range
threshold of the fractional unit. If the discarded portion reaches the rounding
threshold, the arithmetic result receives one unit of the retained fixed-point
scale; otherwise it does not. The exact tie policy should be verified against
the selected C++ routine before the Python method is finalized.

Signed fractional multiplication has an additional complement-encoding issue.
The current C++ `SPMF::MultStd` path delegates to `Mult4b`, which computes the
raw residue product, derives the intermediate sign from the complement range,
shifts the mixed-radix representation right by the fractional digit count, and
applies a whole-range correction constant when the intermediate product is
negative. Related C++ variants process the ordinary product and complemented
product as tandem candidate magnitudes; those variants remain valuable
reference material for the signed normalization problem, even though `Mult4b`
is the first application path being ported. This is not assumed to be the final
mathematics. RNS-PYPAL should leave room for a later single-value signed
fractional multiplication method once that newer math is introduced and tested.

The first Python `SPMF` implementation slice supports preliminary trap-door
assignment/display helpers, unit-value helpers, same-format add, subtract,
compare, and the first same-format `MultStd` / `Mult4b` fractional multiply
port. Same-format arithmetic checks system/effective-format compatibility and
fraction-position compatibility before operating. Addition, subtraction, and
comparison delegate to inherited `SPPM` behavior and do not move the fraction
point. Standard multiplication uses the raw residue product, mixed-radix
fractional shift/truncation, rounding comparison, and signed correction
constant path described above.

The first mixed-type `SPMF` slice also supports whole-integer add/subtract and
integer multiplication. Whole add/subtract corresponds to the C++ `Add1` and
`Sub1` helpers: the integer operand is converted to an aligned fixed-point
temporary by multiplying by the current unit range, then ordinary same-format
fractional add/subtract is used. Integer multiplication corresponds to C++
`Mult(__int64)` and `Mult(SPPM*)`: the scaled fixed-point integer is multiplied
directly through inherited signed residue arithmetic, and the fraction point is
unchanged. Accepted integer operands are explicit Python `int`, signed numeric
strings, and `SPPM` values; an `SPMF` operand is always treated as fractional,
not as an integer.

## Python architecture

### Numeric class hierarchy

RNS-PYPAL should preserve the numeric hierarchy as real Python inheritance
where the lower layer supplies valid behavior for the higher layer:

```text
PPM
└── SPPM
    └── SPMF
```

This is not merely a documentation convenience. It is a library design rule.
`PPM` is the primitive unsigned partial-power residue engine. `SPPM` should
derive from `PPM` and reuse its digit storage, system compatibility checks,
partial-power handling, mixed-radix primitives, formatting helpers, and
unsigned residue operations wherever those operations remain valid.

`SPPM` should override only the behavior whose meaning changes under signed
method-of-complements interpretation: signed assignment, sign recovery and
validation, signed comparison, signed display, sign disposition, and signed
arithmetic dispatch. When an intermediate operation is known to be an unsigned
magnitude operation, `SPPM` may deliberately fall back to `PPM` behavior rather
than reimplementing the same residue mechanics.

`SPMF` should likewise derive from `SPPM` when fixed-point work begins. It will
add fractional-digit interpretation, scaling, fractional multiplication,
rounding, and fixed-point conversion rules while preserving the signed integer
and unsigned residue machinery underneath.

Composition or wrapper objects may still be useful for helper algorithms, such
as mixed-radix streams or temporary working states. They should not replace the
core numeric inheritance relationship unless a later design decision explicitly
documents why inheritance would violate the arithmetic model.

### `RNSNumberSystem`

RNS-PYPAL replaces the C++ process-global configuration with an explicit `RNSNumberSystem` object. It stores the ordered base moduli, normalized powers, fractional digit count, and an optional name. The total digit count is derived from the modulus list rather than stored redundantly.

Each Python numeric object refers to a system. Operations between numeric objects must reject incompatible systems. This is an intentional upgrade over the C++ static configuration and is the foundation for supporting multiple systems in one runtime.

The system object describes the normalized target and must remain distinct from a value's temporary working format. A derived or partial-power value must retain enough format information to distinguish its current effective moduli from the system's normalized moduli. The exact ownership of that mutable state--on each `PPMDigit`, in separate working-format metadata, or both--should remain a small implementation decision until equivalence tests clarify what is required. It must not erase the identity of the normalized parent system.

The C++ `mode` field controls how a system is constructed, and `routine` selects an implementation strategy. Neither is part of the mathematical identity of an explicitly defined Python system. Two values with the same normalized system remain compatible even if their runtime arithmetic strategies differ.

### Value digits and auxiliary digits

`RNSNumberSystem` may describe both value digits and auxiliary digits. Value
digits define the represented number, its public dynamic range, ordinary
conversion, and comparison order. Auxiliary digits are additional residue
channels carried with the RNS word for advanced checking features.

The initial digit roles are:

- `value` -- a normal digit that participates in the represented number;
- `redundant` -- an auxiliary digit reserved for error detection and future
  error correction; and
- `overflow_check` -- an auxiliary digit reserved for overflow or range
  violation detection.

Auxiliary digits do not extend the represented value range and do not increase
the value precision. If an algorithm needs more mathematical range or more
precision, it should convert or cast the value into a larger primary
`RNSNumberSystem`, perform the internal calculation there, and then
round/truncate/normalize back into the intended result system. This applies to
future high-precision algorithms such as Goldschmidt-style fixed-point
division. Extra precision is a larger primary system concern, not a redundant
digit concern.

When auxiliary digits are active, ordinary low-level arithmetic such as add,
subtract, and multiply should update them along with value digits. This keeps
the auxiliary residues synchronized with the operation history. However,
ordinary comparison, sign/range interpretation, and output conversion must use
only value digits unless a method explicitly says it is performing an
auxiliary consistency check.

Auxiliary digits must occupy the last digit indexes of a system. This preserves
the historical RNS digit order for value digits, keeps mixed-radix conversion
stable, and makes the displayed RNS word unambiguous. Raw residue display
should show auxiliary digits as a parenthesized suffix, for example
`3 0 (1, 4)`, so they are visible but not mistaken for range-defining value
digits. Header-style display should use the same parenthesized grouping.

Several future operations can benefit from auxiliary digits as side-effect
checks. Base extension and mixed-radix comparison already perform digit-by-digit
reduction. When auxiliary checking is explicitly enabled, these operations may
carry redundant or overflow-check digits through the same reduction and inspect
their terminal state. In the expected simple overflow/error-detection model,
the relevant auxiliary residues reduce to zero when the value is consistent;
non-zero terminal residues indicate overflow, corruption, or invalid auxiliary
state. These checks must be documented and opt-in until their exact algorithms
are verified.

The first explicit prototype check is simple unsigned overflow detection using
one or more trailing `overflow_check` digits. Ordinary add/subtract/multiply
update the overflow-check digits along with value digits. A later call compares
those check residues against the residues implied by the value digits alone. If
they differ, the value digits have wrapped relative to the tracked auxiliary
state. This does not force overflow checking on ordinary arithmetic; it is an
explicit diagnostic method that benefits from residues already maintained by
the arithmetic operation.

Top-level operations should return fully normalized values in the intended
public system. Internal operations may use derived partial powers, skipped
digits, mixed-radix streams, or temporary larger systems, but the public result
should not accidentally retain a larger internal range or precision. Auxiliary
digits, when present, should either be rebuilt from the final value or checked
by a documented consistency routine.

For the first implementation slice, auxiliary digit plumbing is metadata and
dispatch discipline: all existing systems default to `value` digits, dynamic
range is computed from value digits only, and ordinary conversion/comparison
ignore auxiliary digits. Error-correction, overflow-checking, and cross-system
conversion algorithms will be added as explicit feature slices.

### TOML system definitions

Normalized RNS systems shall be reproducible through human-readable TOML definition files. Loading a file is optional: the same `RNSNumberSystem` may also be constructed programmatically. Both paths must run the same validation and produce the same system identity.

The version 1 file shape is:

```toml
schema_version = 1

[system]
name = "example-system"
moduli = [11, 5, 13, 3, 2]
powers = [2, 3, 2, 5, 8]
fractional_digits = 2

[arithmetic]
modular_reduction = "division"
modular_inverse = "extended_euclidean"
```

The `[system]` table is the portable mathematical definition. Its required fields are `moduli`, `powers`, and `fractional_digits`; `name` is descriptive and optional. `schema_version` is required so future formats can be rejected or migrated deliberately.

The `[arithmetic]` table is optional runtime policy. It may reproduce a preferred implementation strategy but does not affect value compatibility or the system fingerprint. Unknown strategy names must fail clearly rather than silently falling back.

Definition files store only normalized systems. Partial powers, skipped digits, and derived working formats belong to live numeric values and must not be written back as new normalized system definitions by ordinary serialization.

Loading a definition must validate at least:

- a supported schema version;
- non-empty modulus and power lists of equal length;
- integer base moduli greater than one;
- positive integer normalized powers;
- a fractional digit count from zero through the total digit count; and
- pairwise coprimality of the resulting full digit moduli.

The same validation applies when constructing `RNSNumberSystem`
programmatically. The primitive helper `are_coprime(a, b)` checks two values,
and `is_pairwise_coprime(values)` checks an entire modulus set. These helpers
operate on effective full digit moduli, such as `base ** power`, because those
are the actual RNS channels that must be pairwise coprime.

System identity is determined canonically from the ordered normalized moduli, powers, and fractional digit count. The descriptive name and arithmetic policy are excluded. A stable fingerprint may be exposed for diagnostics and reproducibility, but equality must not rely only on a hash value.

Generated systems can be added later, but their generated result should be materialized as the same explicit `[system]` form when it is shared or reproduced. TOML files are data and must never execute Python code.

### System capabilities

The chosen modulus set also determines which algorithms are directly available or efficient. This is not analogous to a fixed-radix integer type whose radix and conversion machinery are always known.

For example, the current C++ decimal extraction routine requires channels based on both 2 and 5; it repeatedly obtains a decimal digit and divides the working RNS value by 10 through those factors. Its hexadecimal routine requires a 2-based channel with at least four valid powers so it can extract base-16 digits. Other modulus sets may require a more general conversion or print routine, potentially based on mixed-radix conversion or integer division.

The general RNS integer-division family requires at least one base-2 modulus. It is more effective on the preferred RNS-APAL geometry: power-based digits with small prime bases, especially including 2 and 5. These are algorithm capabilities and performance preferences, not universal validity requirements for an RNS system.

RNS-PYPAL should derive explicit capabilities from the system definition, such as the presence, position, and usable power of particular prime-base factors. Public conversion operations can then select a specialized routine when its preconditions hold and a general fallback otherwise. An unsupported specialized path must never be entered merely because the system is otherwise valid.

The expected preference order is operation-specific. For decimal output, for example, a direct RNS extraction using 2- and 5-based digits is preferred when available; an RNS integer-division conversion may be used when its weaker preconditions hold; and a general mixed-radix or fixed-radix conversion fallback may be used for systems lacking those capabilities. Capability-based dispatch should be added alongside each concrete routine rather than predicted through one coarse `power_based` flag.

### Arithmetic implementation strategies

The mathematical digit operations should not be coupled permanently to one software implementation. RNS-PYPAL will use a small internal arithmetic-policy seam for costly primitives, not a general plugin framework.

The initial required primitives are:

- modular reduction, `value mod modulus`; and
- modular inverse, `value ** -1 mod modulus`, when the inverse exists.

The safe default reduction strategy is ordinary Python integer remainder (`%`) with a validated positive modulus. This corresponds to division-based modular reduction and works for arbitrary Python integer sizes. A future optimized reducer can replace this primitive without changing `PPMDigit` semantics.

The safe default inverse strategy is the extended Euclidean algorithm, matching the flexible default in the later C++ code. It requires no precomputed table and works for any invertible operand/modulus pair. Failure of the coprimality precondition must raise a clear error rather than return a plausible residue.

The initial Python helper for this primitive is
`multiplicative_inverse(value, modulus)`. It returns the residue `inverse` such
that `(value * inverse) % modulus == 1`, and raises an error when the inverse
does not exist. This is the core per-channel operation used by mixed-radix
conversion after one radix has been consumed.

The higher-level helper
`divide_residue_by_coprime_factor(residue, divisor, modulus)` applies that
inverse multiplication and returns the residue-channel quotient. Its result
`q` satisfies `(q * divisor) % modulus == residue % modulus`. For example,
dividing residue `6` by divisor `5` in modulus `7` returns `4`, because
`(4 * 5) % 7 == 6`.

In mixed-radix conversion, after consuming a digit `d` at modulus `m`, a
remaining channel with modulus `n` is updated as:

```python
new_residue = divide_residue_by_coprime_factor(
    old_residue - d,
    m,
    n,
)
```

`PPMDigit` division has two mathematically different cases that a strategy layer must not blur:

- Dividing by a factor of the digit's own current modulus consumes one or more powers, reduces `PowerValid`, and may skip the digit. This changes the derived format.
- Dividing by a value coprime to the digit modulus preserves the format and multiplies the residue by that value's modular inverse.

The first Python implementation of this rule is `PPM.mod_div(divisor)`, backed
by `PPMDigit.mod_div(divisor)`. It is intentionally a low-level RNS operation,
not a Python-integer division shortcut. For a power-based digit, if `divisor`
is an exact power of that digit's base modulus and divides the current effective
modulus, the digit is divided directly and `PowerValid` is reduced. If
`PowerValid` reaches zero, the digit is marked skipped. All other active
channels divide by the same divisor through inverse modular multiplication.

For example, in a system whose first digit is base `2` with normalized power
`4`, the first effective modulus is `2 ** 4 == 16`. If an RNS value known to be
even is divided by `2`, that digit's effective modulus becomes `2 ** 3 == 8`.
The residue in that channel is directly halved; the other pairwise-coprime
channels multiply by `2`'s modular inverse with respect to their own current
moduli. The result is no longer in the normalized full system: it is a derived
format and must only be combined with values in an identical effective format.

The exact divide precondition is enforced on matching base-power channels. If
the base-`2` channel currently holds an odd residue, `mod_div(2)` raises an
error and leaves the value unchanged. This models the required a priori
knowledge or test that the value is divisible by the consumed base factor.

Base extension and normalization are the inverse side of mixed-radix
decomposition. A derived value is first decomposed using its current effective
radices, including partial-power and skipped positions. The resulting
mixed-radix digits are then reconstructed by processing the retained digits from
most-significant to least-significant:

1. seed a normalized accumulator with the highest retained mixed-radix digit;
2. multiply the accumulator by the next lower mixed radix; and
3. add the next lower mixed-radix digit.

This mirrors the C++ `MRN::Convert` method and is the opposite motion of
mixed-radix conversion: decomposition subtracts and divides, while
reconstruction multiplies and adds.

The target format determines which operation is being performed:

- `PPM.extend_to_current_power()` reconstructs the value to each digit's current
  `Power`. This corresponds to the C++ `ExtendPart2Norm()` idea: skipped and
  partial valid powers are recovered up to the value's current derived format,
  but `Power` is not reset to `NormalPower`.
- `PPM.normalize()` reconstructs the value all the way back to the original
  `NormalPower` format. This corresponds to C++ `Normalize()`.
- `PPM.derived_format_copy()` mirrors the C++ derived-value pattern in which
  each active digit's `PowerValid` becomes its current `Power`, while
  `NormalPower` is preserved so full normalization remains possible.
- `PPM.normalized_copy()` and `PPM.extended_to_current_power_copy()` provide
  non-mutating forms.

`MRN.to_ppm()` performs reconstruction into a full normalized `PPM`. The `PPM`
methods above choose the appropriate target format and then reconstruct through
RNS multiply/add operations.

Critical invariant failures should raise `RNSCriticalError`. The exception
message begins with `CRITICAL ERROR`, includes the call location and function
name, and appends relevant values such as digit index, residue, divisor,
effective modulus, and raw PPM state. If unhandled, Python and Jupyter stop
execution and display the normal traceback in addition to this context. Ordinary
API validation errors, such as an unsupported print radix, may remain ordinary
`ValueError`s.

The C++ implementation offers both a compact inverse lookup table and a much larger brute-force division table. The compact table is indexed by digit position, valid power, and divisor; the brute-force table also accounts for every digit value and grows much faster. Neither table is required for the first Python port.

Future strategies may include Python's built-in modular inverse, lazily cached inverses, compact precomputed inverse tables, or specialized reduction. Tables must be owned or cached by system and strategy, never by each numeric value. They should be generated only on request, enforce explicit memory limits, and be tested against the default algorithms. Large lookup data does not belong in the TOML definition itself.

The initial strategy names are reserved as follows:

- `modular_reduction = "division"`
- `modular_inverse = "extended_euclidean"`

Additional names will be specified only when their implementations are introduced. Arithmetic policy must be selected outside hot digit loops where practical, so future optimization does not require redesigning the public numeric classes.

### RNS-native computation rule

The defining purpose of RNS-PYPAL is general-purpose computation performed in RNS. Every arithmetic operation on an RNS numeric object must execute through residue-domain operations and permitted intermediate RNS or mixed-radix state.

An implementation must not reconstruct an operand as a Python integer, binary integer, decimal value, CRT value, or floating-point value, perform the requested mathematical operation there, and convert the result back to residues. This prohibition applies even when Python's arbitrary-precision integers would make that shortcut easy and produce numerically correct examples.

In particular, comparison, signed arithmetic, integer and fractional multiplication, division, scaling, square root, and later mathematical functions may not use convert-operate-reencode implementations. Mixed-radix digits may be streamed to make ordering, truncation, sign, or rounding decisions because that conversion is itself performed through RNS subtraction and inverse modular multiplication; it must not become a disguised reconstruction shortcut.

Fixed-radix arithmetic has legitimate, limited roles:

- accepting a Python integer or string and encoding it into residue digits;
- returning an explicitly requested binary, decimal, hexadecimal, Python-integer, or textual representation;
- implementing a documented conversion fallback when the modulus geometry lacks a specialized RNS conversion capability;
- providing preliminary fixed-point `SPMF` I/O by scaling through Python
  arbitrary-precision integers or strings at the boundary layer;
- calculating system metadata and constants during initialization; and
- serving as an independent test oracle, never as the implementation under test.

Arbitrary-length binary and decimal helpers may therefore be added to the conversion layer when needed for flexible input and output. They must be visibly separated from the RNS arithmetic layer. Public documentation and tests should make it possible to tell whether a conversion selected a specialized RNS path, an RNS integer-division path, a mixed-radix path, or a fixed-radix fallback.

Scalar helper operations do not violate this rule when the scalar is independently reduced into each residue channel and the requested arithmetic remains digit-wise. The forbidden pattern is reconstructing an existing RNS operand into a positional value in order to compute the result.

### Explicit validation and side-effect checks

Basic arithmetic is performance-first. Primitive operations such as add,
subtract, multiply, and low-level modular scaling update the residue state
required by the operation, but they must not automatically perform expensive
validation features such as overflow detection, error detection, redundant-digit
correction, sign recovery, or sign validation.

Validation features are explicit unless the operation already computes the
necessary information as an unavoidable byproduct. In those cases, the operation
may update or validate cached metadata as a documented side effect. A side-effect
check is acceptable only when it does not change the numerical semantics of the
operation and does not impose a new hidden performance cost on ordinary
arithmetic.

Examples:

- `PPM.add`, `PPM.sub`, and `PPM.mult` may maintain overflow-check or redundant
  residues because those digits are part of the active arithmetic state, but
  they do not automatically call overflow or error-checking routines.
- `PPM.has_overflow()` is an explicit diagnostic check.
- Future `SPPM` operations may maintain a sign cache, but basic arithmetic must
  clearly document whether that cache is preserved, invalidated, or recomputed.
- Future `SPMF` algorithms may validate sign or range state as a free side
  effect when the required mixed-radix or comparison information is already
  produced by the intended operation.

The result of a validation feature must not silently alter the mathematical
result. If a check fails, it should return an explicit status or raise a clear
exception according to the method's documented contract.

RNS-PYPAL diagnostic handling should distinguish at least three severities:

- warning -- report the condition and continue;
- error -- raise a recoverable project exception; and
- critical -- raise a hard-stop `RNSCriticalError`.

The existing `critical_error(...)` helper remains a hard-stop convenience path.
Feature-specific APIs may expose a severity parameter when it is useful to let
applications choose between exploratory diagnostics and strict execution.

### Overflow and range policy

Unless a method explicitly states otherwise, RNS-PYPAL arithmetic follows the
same broad range policy as C/C++ integer and fixed-point arithmetic: operations
do not automatically check for overflow. The caller is responsible for selecting
an RNS system with sufficient dynamic range and for performing any required
range checks before or after an operation.

This policy is especially natural for RNS because ordinary add, subtract, and
multiply operations wrap in the declared residue range. Detecting overflow is
possible, but it is not free. General overflow detection normally requires
additional structure, such as redundant residue digits, comparison, conversion,
or other range-analysis support. Those mechanisms are advanced features and
should be added deliberately rather than imposed on every primitive operation.

Consequently, passing tests for residue arithmetic means the digit-wise modular
result is correct for the declared RNS format; it does not imply that the
mathematical result fit in the intended application range. Future redundant-digit
or range-checking features should be opt-in capability layers, not hidden
default behavior.

### Unsigned integer division

The first Python unsigned integer division target is the stable C++ container
path `PPM::DivStd`, which delegates to the `DivPM7` algorithm. The Python method
is named `PPM.div_std(divisor)`. It follows the C++ mutation style: the dividend
object is replaced by the quotient, and the returned `PPM` object is the
remainder.

This division method must remain RNS-native. It may use residue subtraction,
residue addition, digit-level modular division, mixed-radix comparison,
partial-power formats, divisor incrementing, and base extension/normalization
steps. It must not convert the dividend and divisor to Python integers, perform
integer division there, and convert the quotient and remainder back to RNS.

The algorithm requires an RNS system containing a base modulus of `2`. In
practice, systems with low prime base moduli such as `2`, `3`, and `5` are more
useful because they give the divisor more available factors during the
reduction process. When the working divisor has been reduced to a state where
no current modulus factor is available for another division step, the algorithm
increments the working divisor. This is the important `DivPM7` rule that can
create a usable factor again, especially the base-2 factor.

This method must be able to divide values at the high end of the represented
unsigned range without relying on base extension outside the defined RNS number
system. Like the rest of unsigned `PPM` arithmetic, division does not impose a
general overflow policy beyond the declared residue system and the explicit
algorithm preconditions.

### PPM assignment and `AssignPM`

Python `PPM` construction and assignment should support the important input
forms needed by the original C++ class:

- non-negative Python integers;
- unsigned decimal strings;
- unsigned hexadecimal strings using a `0x` prefix;
- unsigned binary strings using a `0b` prefix when useful; and
- compatible `PPM` values.

String assignment is an input-conversion feature. It should encode the
positional text into residues by repeated residue multiply/add steps so it can
accept strings longer than ordinary machine integers and does not depend on
Python's guarded decimal `int()` parser.

`assign()` follows the C++ `Assign` convention for integer and string inputs:
it resets the destination to the full normalized power-based system before
encoding the new value. Assigning from another `PPM` copies the source's current
valid-power state into the destination value without changing the destination's
normal system definition.

`assign_pm()` corresponds to the C++ `AssignPM` family. It is the partial-power
assignment path: it can assign an integer or string into the current derived
format, or copy/promote another `PPM` value's `PowerValid` structure into the
destination's current working `Power` structure while retaining `NormalPower`
as the route back to full normalization.

Older C++ paths that assume simple modulus-only RNS behavior without
partial-power derived formats should not be ported as first-class RNS-PYPAL
behavior. The Python library assumes power-based `PPM` semantics as the
foundation.

For unsigned `PPM`, the initial general-purpose conversion fallback is:

- `PPM.to_int()` -- converts through mixed-radix digits and returns a Python
  arbitrary-precision integer for output/debugging;
- `PPM.format_value(radix=10, prefix=False)` -- returns the represented unsigned
  value in radix 2, 10, or 16; and
- `PPM.print_value(...)` -- prints that converted value.

`str(ppm)` uses the decimal `format_value()` path. Raw residue display remains
available through `format_native()` / `print_native()`, and modulus-header
display remains available through `format_whdr()` / `print_whdr()`.

Python integers are arbitrary precision and can represent values far beyond the
native CPU word size. Python versions such as 3.12 also protect direct
integer-to-decimal conversion with a maximum digit guard. RNS-PYPAL's fallback
decimal formatter therefore emits decimal strings in small chunks instead of
depending on one direct `str(huge_int)` call. Hexadecimal and binary output are
also supported for converted unsigned `PPM` values.

This fallback is intentionally part of the conversion layer. Arithmetic methods
must not call `to_int()` to compute RNS results.

### Python class mapping

| C++ type | Python type | Responsibility |
| --- | --- | --- |
| `ModTable` plus configuration globals | `RNSNumberSystem` | Immutable or safely shared system definition |
| `PPMDigit` | `PPMDigit` | Residue and per-digit power/validity state |
| `PPM` | `PPM` | Unsigned residue integer operations |
| `SPPM` | `SPPM` | Signed complement-based integer operations |
| `SPMF` | `SPMF` | Signed fixed-point residue operations |
| `MRDigit` and `MRN` | Streaming conversion primitive; optional stored mixed-radix type | Ordered conversion, comparison, truncation, rounding, and reconstruction |

Python method names may follow Python conventions, but their arithmetic semantics, mutation behavior, range rules, and error cases should remain traceable to named C++ methods.

## Porting rules

1. Treat the C++ implementation and observed outputs as reference evidence, not an infallible specification.
2. Port one coherent feature slice at a time.
3. Add equivalence tests before expanding the next slice.
4. Separate deliberate Python upgrades from compatibility fixes.
5. Do not assume an existing Python skeleton is correct merely because a smoke test passes.
6. Prefer stable public/container methods over experimental numbered variants unless a test demonstrates the need for a specific variant.
7. Do not compare raw residue arrays positionally. Ordering operations must use mixed-radix conversion or another proven RNS comparison algorithm with equivalent behavior.
8. Preserve the digit-streaming form of mixed-radix algorithms when the complete converted value is unnecessary.
9. Treat derived formats as temporary algorithm state and normalize public results back into their declared full-power system unless an API explicitly promises a derived result.
10. Select specialized algorithms only when the instantiated modulus set satisfies their structural preconditions.
11. Keep mathematical system identity separate from arithmetic implementation policy.
12. Treat division-based reduction and extended-Euclidean inversion as the reference implementations against which optimized strategies are tested.
13. Never implement an RNS arithmetic operation by converting operands to a positional representation, calculating there, and converting back.
14. Confine arbitrary-precision binary and decimal arithmetic to explicit conversion, initialization, metadata, and test-oracle boundaries.
15. Preserve the numeric inheritance hierarchy where appropriate: `SPPM` derives from `PPM`, and `SPMF` derives from `SPPM`, with higher-level classes overriding only the semantics that actually change.

## Evidence and verification policy

RNS-APAL contains stable library code, historical implementations, dormant prototypes, experimental numbered variants, stale comments, and potentially defective paths. RNS-PYPAL must not reproduce a C++ defect merely for literal source fidelity unless compatibility with that exact behavior is explicitly required.

When sources disagree, use the following evidence in context:

1. established RNS mathematics and invariants;
2. clarified design intent recorded in this specification;
3. behavior of the active stable public path;
4. focused C++ examples and independently calculated expected values;
5. headers and implementation comments; and
6. dormant or explicitly experimental routines.

This ordering is guidance rather than a mechanical rule. Any material conflict must be recorded, reduced to a small test case, and resolved deliberately before the affected Python behavior is accepted.

Simple primitives should be verified first because their domains can often be exhaustively tested for small systems. Later algorithms should be built from those verified primitives and checked through mathematical identities, boundary cases, differential tests against stable C++ paths, and independent Python reference calculations.

Initialization and validation require particular care. The newer runtime `init_RNS_APAL(...)` API expresses an intended move away from compile-time geometry, but the current C++ implementation retains legacy global-array dependencies. Its modulus validator also contains constraints and loop bounds that must be validated independently rather than copied blindly. Python TOML and programmatic construction replace both initialization styles with one explicit, tested path.

## Current implementation status

The repository currently contains:

- an `rns_pypal` package with tested `RNSNumberSystem`, `PPMDigit`, `PPM`,
  mixed-radix, and `MRN` foundations;
- unsigned `PPM` assignment, conversion, add/subtract/multiply, comparison,
  partial-power reduction, normalization/current-power extension, and the first
  `DivStd`/`DivPM7` unsigned division port;
- auxiliary digit role metadata for value, redundant, and overflow-check digits,
  including explicit prototype overflow checking;
- an initial complement-based `SPPM` slice for signed assignment, sign recovery,
  sign-cache disposition, signed comparison, negation/absolute value, and
  signed add/subtract/multiply;
- an initial `SPMF` fixed-point slice for fraction-point metadata,
  sliding-point checks, preliminary trap-door I/O, unit-value helpers, and
  same-format add/subtract/compare/multiply; and
- regression tests covering these implemented slices.

Current class-level review documents are maintained separately:

- `docs/PPM_UNSIGNED_REFERENCE.md` -- unsigned reference index
- `docs/PPM_REFERENCE.md`
- `docs/PPMDIGIT_REFERENCE.md`
- `docs/MIXED_RADIX_REFERENCE.md`
- `docs/SPPM_REFERENCE.md`
- `docs/SPMF_REFERENCE.md`

This code is not yet a complete behavioral port of RNS-APAL. In particular,
signed division, signed auxiliary digit handling, mixed-type `SPMF`
operations, `SPMF` division/scaling/normalization, advanced base extension
behavior, Goldschmidt routines, square roots, and other research algorithms
still require systematic comparison with C++ and additional design
clarification.

## Recommended first conversion slice

The first slice should establish the normalized unsigned foundation:

1. Define and validate one known RNS system from the C++ configuration.
2. Define system identity and compatibility from the ordered normalized moduli and powers without building a broad type framework.
3. Verify `PPMDigit` normalized modulus and modular add/subtract/multiply behavior.
4. Verify `PPM` integer assignment and residue layout.
5. Implement and verify the streaming mixed-radix decomposition primitive, including its inverse-modular division step.
6. Verify conversion back to a Python integer across the full unsigned system range.
7. Replace raw-residue comparison with streaming mixed-radix comparison and test cases where residue ordering disagrees with numeric ordering.
8. Verify `PPM` add, subtract, multiply, equality, and zero behavior, including wraparound.

The first slice should record the normalized parent system and leave clean seams for derived formats, but full partial-power mutation and base extension remain a later, separately tested slice.

Partial powers, signed values, and fractions should build on that verified base rather than being corrected simultaneously.

## Later work

- Further partial-power/base-extension equivalence work
- Signed division and SPPM auxiliary-digit policies
- `SPMF` normalization, scaling, division, and additional mixed-format casting
- Advanced division, Goldschmidt routines, square roots, and other research algorithms
- Python conveniences such as operators and non-mutating APIs, added only after reference behavior is covered

## License

The original RNS-APAL source is licensed under CC BY-NC-SA. RNS-PYPAL inherits the applicable license terms for derived work.
