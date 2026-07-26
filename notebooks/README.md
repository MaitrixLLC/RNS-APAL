# Notebook test hierarchy

The focused notebooks are organized by the implementation layer they exercise:

1. `modular_helpers_tests.ipynb` — modular inverse and residue-channel division.
2. `mixed_radix_mrn_tests.ipynb` — mixed-radix decomposition and `MRN`.
3. `ppm_integer_tests.ipynb` — unsigned `PPM` integer behavior.
4. `sppm_integer_tests.ipynb` — signed `SPPM` integer behavior.
5. `spmf_fixed_point_tests.ipynb` — fixed-point `SPMF` behavior.

`RNSNumberSystem` and `PPMDigit` have dedicated automated coverage in
`tests/test_02_number_system.py` and `tests/test_03_ppm_digit.py`. They remain
prerequisites for the higher-level notebook experiments rather than separate
interactive notebooks.

`rns_pypal_scratch.ipynb` remains an exploratory scratch notebook and is not part
of the clean-kernel verification sequence above.
