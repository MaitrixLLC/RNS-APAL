# Test hierarchy

The pytest files are numbered to show the implementation hierarchy:

1. `test_01_modular_helpers.py` — exact modular helper functions.
2. `test_02_number_system.py` — `RNSNumberSystem` and digit roles.
3. `test_03_ppm_digit.py` — individual `PPMDigit` behavior.
4. `test_04_mixed_radix.py` — `MixedRadixDecomposer` and `MRN`.
5. `test_05_ppm.py` — unsigned `PPM` integers.
6. `test_06_sppm.py` — signed `SPPM` integers built on `PPM`.
7. `test_07_spmf.py` — fixed-point `SPMF` values built on `SPPM`.

Every test file is independently collectable. The numbering documents conceptual
prerequisites; tests do not depend on execution state from an earlier file.

Run the complete hierarchy from the repository root:

```powershell
.venv\Scripts\python.exe -m pytest
```
