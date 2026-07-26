# RNS-PyPAL Human Context

## Purpose of This File

This file is my personal re-entry guide for RNS-PyPAL.

It is intended to help me quickly remember:

* what the project is;
* how the development environment is configured;
* where I left off;
* which files and notebooks matter;
* what is currently verified;
* what I should do next.

This file may contain informal notes, reminders, uncertainties, and personal observations that do not belong in the public README.

VS Code agents may help maintain this file, but it is written primarily for me to read.

---

## What RNS-PyPAL Is

RNS-PyPAL is the Python implementation and experimentation environment derived from the RNS-APAL arbitrary-precision residue number system work.

The project is intended to:

* preserve and modernize the original RNS-APAL algorithms;
* provide readable Python implementations;
* support integer and fractional RNS experimentation;
* provide notebooks for exploration and demonstrations;
* create repeatable automated tests;
* compare RNS results against exact rational and arbitrary-precision references;
* support future papers, demonstrations, and hardware development.

Mathematical correctness is more important than code brevity or stylistic modernization.

---

## Development Environment

### Repository Location

```text
D:\Projects\RNS-APAL-REPO\RNS-APAL
```

### Primary Editors

Python development and debugging:

```text
PyCharm
```

Agent-assisted repository work, documentation, Git, and notebooks:

```text
Visual Studio Code
```

This is intentionally a hybrid development environment.

### Python Virtual Environment

Virtual environment:

```text
.venv
```

Interpreter:

```text
D:\Projects\RNS-APAL-REPO\RNS-APAL\.venv\Scripts\python.exe
```

Python version last verified:

```text
Python 3.12.4
```

### Activate from Git Bash

```bash
source .venv/Scripts/activate
```

The Git Bash prompt may display the virtual environment on a separate line:

```text
(.venv)
eric@DSR-WS-1 MINGW64 /d/Projects/RNS-APAL-REPO/RNS-APAL (rns_pypal)
```

Meaning:

```text
(.venv)       Active Python virtual environment
(rns_pypal)   Current Git branch
```

### Verify the Python Interpreter

```bash
python -c "import sys; print(sys.executable)"
```

Expected result:

```text
D:\Projects\RNS-APAL-REPO\RNS-APAL\.venv\Scripts\python.exe
```

Also verify:

```bash
python --version
```

### VS Code Interpreter

VS Code should use:

```text
D:\Projects\RNS-APAL-REPO\RNS-APAL\.venv\Scripts\python.exe
```

Select it using:

```text
Ctrl+Shift+P
Python: Select Interpreter
```

### Jupyter Kernel

Jupyter notebooks in VS Code should use the same `.venv` interpreter.

Select it from the notebook kernel selector:

```text
Select Another Kernel
Python Environments
RNS-APAL\.venv\Scripts\python.exe
```

Verify from inside a notebook:

```python
import sys
print(sys.executable)
```

Expected result:

```text
D:\Projects\RNS-APAL-REPO\RNS-APAL\.venv\Scripts\python.exe
```

---

## Jupyter Recovery Notes

A notebook previously became stuck while attempting to connect to a missing or incorrectly configured kernel.

The obsolete VS Code setting was:

```json
"python.defaultInterpreterPath": ".venv/Scripts/python.exe"
```

The actual environment existed, but VS Code initially failed to resolve it correctly.

Recovery steps:

1. Restart VS Code if the kernel remains permanently busy.
2. Confirm that `.venv` exists.
3. Activate `.venv` from Git Bash.
4. Verify the interpreter with `sys.executable`.
5. Select the same interpreter in VS Code.
6. Select the same environment as the notebook kernel.
7. Run a simple notebook cell to verify the interpreter.

Do not create a replacement environment until the existing `.venv` has been inspected.

---

## Current Repository Observations

Jupyter is now working from within VS Code.

An older workbook or notebook still uses the name:

```text
print_demo
```

This appears to be an older API or demonstration name.

The notebook may still contain useful work, but it should not automatically be treated as current.

Before modifying or renaming it, determine:

* what `print_demo` originally did;
* what the current equivalent is;
* whether the notebook is historical, active, or superseded;
* whether its useful results should become automated tests.

---

## Important Working Principles

* Do not assume that a notebook is current merely because it runs.
* Do not assume that code is mathematically correct merely because it executes.
* Preserve exact integer and rational arithmetic.
* Do not substitute floating-point approximations without explicit review.
* Do not casually change normalization, sign, range, scaling, or fractional interpretation.
* Preserve correspondence with the original RNS-APAL implementation.
* Important notebook results should eventually become tests, examples, or documented findings.
* Agents should report ambiguity rather than invent mathematical behavior.

---

## How to Resume This Project

1. Open the repository in PyCharm.
2. Open the same repository in VS Code.
3. Open Git Bash in VS Code.
4. Activate `.venv`.
5. Verify `sys.executable`.
6. Confirm the VS Code Python interpreter.
7. Confirm the Jupyter kernel.
8. Check Git status.
9. Read this file.
10. Read `PROJECT_STATUS.md`.
11. Read `notebooks/README.md`.
12. Run the existing tests.
13. Select one bounded next task.

---

## Current Immediate Objective

Create a conservative agent-assisted development structure.

Initial tasks:

1. Inventory the repository.
2. Inventory all notebooks.
3. Identify current and obsolete names.
4. Determine which notebooks run from a clean kernel.
5. Determine what tests already exist.
6. Create `PROJECT_STATUS.md`.
7. Create `AGENTS.md`.
8. Create `notebooks/README.md`.

No files should be deleted, moved, renamed, or substantially refactored during the initial inventory.

---

## Current Next Task

Ask the VS Code agent to inspect the repository and notebooks without editing anything.

Suggested prompt:

> Inspect the complete RNS-PyPAL repository, including Python source files, tests, notebooks, examples, documentation, configuration files, and available Git history.
>
> Produce a factual current-state assessment containing:
>
> * implemented capabilities;
> * partially implemented capabilities;
> * existing tests;
> * notebook inventory;
> * notebooks that appear outdated;
> * obsolete class, function, or demonstration names;
> * notebooks that should be run from a clean kernel;
> * environment and dependency requirements;
> * undocumented mathematical assumptions;
> * the three most logical next tasks.
>
> Do not edit, rename, move, or delete any files.
>
> Do not infer mathematical correctness merely because code runs.
>
> Clearly distinguish observed facts from inferences.

---

## Session Log

### 2026-07-26

What I did:

* Recovered the RNS-PyPAL Python virtual environment.
* Confirmed that `.venv` uses Python 3.12.4.
* Confirmed the interpreter location.
* Connected VS Code Jupyter to the `.venv` kernel.
* Identified an older notebook using the name `print_demo`.

What worked:

* Git Bash activation of `.venv`.
* VS Code interpreter selection.
* Jupyter kernel connection.

What remains uncertain:

* Whether `print_demo` was renamed or replaced.
* Which notebooks are current.
* Which notebook results have automated tests.
* The exact overall implementation status.

Exact next step:

Perform a read-only repository and notebook inventory using the VS Code agent.
