
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

### `SPMF`

`SPMF` derives from `SPPM` and interprets part of the residue system as fixed-point fractional range. It tracks:

- `NumFractDigits` -- current radix-point position
- `NormalFractDigits` -- normalized radix-point position

Fractional digits are the first digits in `Rn`. `SPMF` adds decimal and ratio assignment, fixed-point printing, fractional multiplication, scaling, division, inverse, normalization, and square-root routines. The C++ header contains multiple research versions of several algorithms; application-facing container methods such as `MultStd` should guide which implementation is treated as canonical.

## Python architecture

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
- calculating system metadata and constants during initialization; and
- serving as an independent test oracle, never as the implementation under test.

Arbitrary-length binary and decimal helpers may therefore be added to the conversion layer when needed for flexible input and output. They must be visibly separated from the RNS arithmetic layer. Public documentation and tests should make it possible to tell whether a conversion selected a specialized RNS path, an RNS integer-division path, a mixed-radix path, or a fixed-radix fallback.

Scalar helper operations do not violate this rule when the scalar is independently reduced into each residue channel and the requested arithmetic remains digit-wise. The forbidden pattern is reconstructing an existing RNS operand into a positional value in order to compute the result.

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

- an `rns_pypal` package scaffold
- initial `RNSNumberSystem` and `PPMDigit` implementations
- initial `PPM`, `SPPM`, and `SPMF` implementations
- a small smoke-test suite covering assignment, basic arithmetic, zero, comparison, and elementary sign behavior

This code has not yet been established as behaviorally equivalent to RNS-APAL. In particular, partial-power semantics, base extension, normalization, overflow/range behavior, conversion, signed-state transitions, and fixed-point arithmetic still require systematic comparison with C++.

The current Python `PPM.compare()` lexicographically compares raw residue digits in reverse array order. That is not a valid general comparison for a non-positional RNS representation and must be replaced with the digit-by-digit mixed-radix process before Python comparison tests can be considered meaningful.

The current Python `SPPM` skeleton treats its sign-magnitude flag as the operative representation rather than maintaining complement-based residue arithmetic. The current `SPMF` skeleton also contains positional integer-division shortcuts and incomplete expressions. These modules are unverified placeholders and must not guide the port of RNS arithmetic.

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

- Partial-power digits, skip state, derived formats, normalization, and base extension
- Stable integer division through the `DivStd` behavior
- `SPPM` sign validity, sign recovery, overflow rules, and signed arithmetic
- `SPMF` unit scaling, decimal/ratio assignment, normalization, and `MultStd`
- Advanced division, Goldschmidt routines, square roots, and other research algorithms
- Python conveniences such as operators and non-mutating APIs, added only after reference behavior is covered

## License

The original RNS-APAL source is licensed under CC BY-NC-SA. RNS-PYPAL inherits the applicable license terms for derived work.
