# C++ RNS-APAL reference source

This directory contains the original C++ RNS-APAL source tree and Visual Studio
project files used as reference evidence for the Python RNS-PYPAL conversion.

Important files:

- `ppm.cpp` / `ppm.h` — unsigned `PPM` implementation and declarations.
- `mrn.cpp` / `mrn.h` — mixed-radix number support.
- `sppm.cpp` / `sppm.h` — signed integer residue support.
- `spmf.cpp` / `spmf.h` — signed fixed-point residue support.
- `config.cpp` / `config.h` and `init.cpp` / `init.h` — original system
  configuration and initialization paths.
- `utilities.cpp` / `utilities.h` — shared C++ helpers.
- `User_Guide/` — original RNS-APAL PDF documentation.

The C++ code is reference material, not automatically authoritative. Some files
contain dormant prototypes, historical variants, stale comments, or bugs. The
Python port should preserve stable mathematical behavior and documented
invariants, with tests added for each converted feature slice.

