# RNS-PyPAL Agent Instructions

## Required session startup

At the start of every session, before planning, reviewing, or modifying the
repository:

1. Read this `AGENTS.md` file completely. Codex loads it automatically from the
   repository root, but the active agent remains responsible for following it.
2. Read [ARCHITECTURE.md](ARCHITECTURE.md) completely. It is the canonical
   technical and mathematical specification, but it is a referenced project
   document rather than an automatically discovered instruction file.
3. Read [PROJECT_STATUS.md](PROJECT_STATUS.md) completely. It records the
  current bounded task queue and unresolved implementation work.
4. Inspect the current Git status and preserve unrelated user changes.
5. For the requested task, revisit the architecture sections governing the
   affected classes, algorithms, representations, and invariants.

Do not skip the architecture review because a change appears small. Small
changes to conversion, display, or tests can still encode mathematical
assumptions.

## Project context and evidence

- [ARCHITECTURE.md](ARCHITECTURE.md) — canonical architecture, mathematical
  rules, implementation status, and validation policy.
- [PROJECT_STATUS.md](PROJECT_STATUS.md) — current task queue and deferred
  implementation work; verify every item against the architecture and source.
- [HUMAN_CONTEXT.md](HUMAN_CONTEXT.md) — personal project re-entry notes and
  historical context; verify potentially stale claims against the repository.
- [notebooks/README.md](notebooks/README.md) — focused notebook hierarchy and
  clean-kernel verification scope.
- `docs/*_REFERENCE.md` — class-level C++ and Python reference evidence.
- `cpp_ref/` — original C++ RNS-APAL reference source.
- `tests/` — executable behavioral and regression evidence, ordered by the
  numeric class hierarchy described in `tests/README.md`.

When sources disagree, apply the evidence and verification policy in
`ARCHITECTURE.md`. Record material conflicts and reduce them to focused tests
instead of resolving them silently.

## Universal arithmetic rules

- Treat RNS as the primary arithmetic domain.
- Do not implement arithmetic through positional
  convert-operate-reencode shortcuts.
- Preserve exact integer and rational arithmetic where applicable.
- Do not silently reinterpret normalization, signs, ranges, scaling,
  fractional positions, auxiliary digits, or derived formats.
- Preserve the hierarchy `PPMDigit` → `PPM` → `SPPM` → `SPMF`; validate lower
  layers before relying on them in higher layers.
- Treat C++ behavior as reference evidence, not infallible truth.
- Distinguish successful execution, regression coverage, experimental
  agreement, and mathematical validation.

## Development and verification

- Keep changes bounded to the requested task and avoid unrelated cleanup.
- Use the repository virtual environment at `.venv\Scripts\python.exe`.
- Run the narrowest relevant pytest layer first, then the complete suite.
- Run `python -m ruff check rns_pypal tests` after Python changes.
- Execute modified notebooks from a clean kernel and confirm that every
  nonempty code cell ran without an error output.
- Add or update exact tests for behavior changes. Prefer integer, rational, and
  invariant-based expectations over floating-point agreement.
- Do not claim mathematical correctness solely because code or notebooks run.

## Completion criteria

Before reporting completion:

1. Review the final diff and Git status.
2. Report the exact tests, notebook executions, and lint checks performed.
3. Identify any unresolved mathematical, architectural, or reference conflict.
4. Leave unrelated user work untouched.
