# Notebook test hierarchy

The focused class-level verification notebooks are organized in directories
named for the primary class they exercise:

1. `modular_helpers_tests.ipynb` — modular inverse and residue-channel division.
2. `MRN/mixed_radix_mrn_tests.ipynb` — mixed-radix decomposition and `MRN`.
3. `PPM/ppm_integer_tests.ipynb` — unsigned `PPM` integer behavior.
4. `SPPM/sppm_integer_tests.ipynb` — signed `SPPM` integer behavior.
5. `SPMF/spmf_fixed_point_tests.ipynb` — fixed-point `SPMF` behavior.

`RNSNumberSystem` and `PPMDigit` have dedicated automated coverage in
`tests/test_02_number_system.py` and `tests/test_03_ppm_digit.py`. They remain
prerequisites for the higher-level notebook experiments rather than separate
interactive notebooks.

`rns_pypal_scratch.ipynb` remains an exploratory scratch notebook and is not part
of the clean-kernel verification sequence above.

Supplemental runnable demonstrations:

- `ppm_assign_rnd_demo.ipynb` — seeded `PPM.assign_rnd()` using an explicitly
  declared six-digit power-based RNS system with bases `2, 3, 5, 7, 11, 13`.
