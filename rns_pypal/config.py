"""Bundled normalized RNS system definitions.

The fractional geometry of the historical Rez-9 example is intentionally not
declared here yet: active C++ sources disagree on whether it uses seven or
eight fractional digits.  Tests use explicit small systems until that conflict
is resolved.
"""

from .modtable import RNSNumberSystem


REZ9_INTEGER_SYSTEM = RNSNumberSystem(
    name="rez9-18-integer",
    moduli=(11, 5, 13, 3, 2, 17, 7, 19, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509),
    powers=(2, 3, 2, 5, 8, 2, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
    fractional_digits=0,
)

# Compatibility name for early callers.  This is explicitly the integer
# geometry and must not be assumed to be the eventual default SPMF geometry.
DEFAULT_SYSTEM = REZ9_INTEGER_SYSTEM
