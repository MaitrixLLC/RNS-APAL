#ifndef CONFIG_HEADER
#define CONFIG_HEADER

// User configurations for adjusting the RNS number system used by RNS-APAL

// **********************  SET THE TOTAL NUMBER OF DIGITS TO BE USED IN THE RNS SYSTEM BY SETTING NUM_PPM_DIGS ******************************//
//				(This will define the total working register size, and must be large enough to include fractional, whole and extended digits )

//#define NUM_PPM_DIGS  8		// number of RNS TPU digits
//#define NUM_PPM_DIGS	14		// Max Setting for use with __int64 conversions, about 60 bits
//#define NUM_PPM_DIGS	15		// Max Setting for use with __int64 conversions, about 60 bits
//#define NUM_PPM_DIGS	18		// Maximum working size for digit width=6 bits (binary range=64), about 79 bits, 24 decimal digits
//#define NUM_PPM_DIGS	31		// Maximum working size for digit width=7 bits (binary range=128), about 162 bits, 49 decimal digits
//#define NUM_PPM_DIGS	54		// Maximum working size for digit width=8 bits (binary range=256), about 335 bits, 101 decimal digits
//#define NUM_PPM_DIGS	97		// Maximum working size for digit width=9 bits (binary range=512), about 703 bits, 212 decimal digits
//#define NUM_PPM_DIGS	172		// Maximum working size for digit width=10 bits (binary range=1024), about 1420 bits, 428 decimal digits

// **********************  SET THE NUMBER OF FRACTIONAL DIGITS USED IN THE RNS FRACTIONAL NUMBER SYSTEM *******************************//

//#define SPMF_FRACTION_DIGS	8		// I am keeping this for now because SPMF2_FRACTION_DIGS uses SPMF_FRACTION_DIGS. However, I see that SPMF2_FRACTION_DIGS is not used anywhere,
									// so they can both be removed safely.

									// In general, this value should be about one-fourth the total number of digits: NUM_PPM_DIGS
									// this provides a roughly equal fractional range to whole range, and provides a roughly equal range of extended digits to non-extended digits.
									// however, as long as the range is not exceeded, other digit ratios may be used
									// fractional digits are ALWAYS the first digits of the RNS digit array  (PPM->Rn[] array)
									// This value is not important if only using integer arithmetic

// **************************** Set following appropriately to control how RNS Modulus are defined ************************************************* //

//#define CUSTOM_MODULUS  1			// define as "0" for auto-generated RNS number format, otherwise define as "1" for user defined modulus
									// for user defined modulus, Modulus values must be manually added to "Modulus[]" array in "config.cpp"
									// for user defined modulus, make sure to define the custom ModPowers[] array in "config.cpp", and set the NUM_MODULUS_POWERS defined below

//#define NO_POWERS					// define NO_POWERS will produce a non-power based RNS system, otherwise, RNS system is power based
									// (this setting only has effect if CUSTOM_MODULUS is "0")

//#define CUSTOM_POWERS  CUSTOM_MODULUS	// DO NOT CHANGE THIS !!
										// new rules: if user wants custom modulus, then user must also define custom powers;
										// the problem solved was that many combinations of these settings was failing.

//#define NUM_MODULUS_POWERS	NUM_PPM_DIGS		// NEED TO SET APPROPRIATELY IF CUSTOM_MODULUS = 1, otherwise it is ignored, used to declare size of the ModPowers array

// **************************** Set following to control how to perform method of Inverse Multiplication  ********************************** //
//								(New default is to use extended Euclidean, i.e., no LUTs)

//#define USE_MODDIV_LUT		// define this if user wishes to use LUT for inverse multiplication of RNS digits, slightly faster operation;
								// otherwise, the code uses an extended Euclidean routine.  This is a bit slower, but does not require huge memory

//#define USE_BRUTE_LUT			// define this if you want a brute force LUT approach, which takes about maxdigit^3 amount of shorts!
								// To use brute force LUT, USE_MODDIV_LUT must also be defined above. (brute force for research, otherwise, not recommended)

// ***************************  Set to adjust properties of the fractional print routine, See user guide for more info  ********************************************  //

//#define PRINT_REMAINDER		// if defined, SPMF::print(int r) will attempt to place two digits of significance that are estimated based on remainders of conversion
							// otherwise, if not defined, the routine will print a '+' sign at end to indicate a remainder, and will only print the significant digits.

//#define CONSOLE_WIDTH 80		// Sets the default console width;  set this to 80 for Win 7 & 8;  Set this to 128 for Win10.  

// *********************************************************************************************************************************** //

// Do not alter the following defines ...

#define ABS_SIGNED_RANGE	ModTable::returnNumDigits() 		// this is for SPPM2, set less than NUM_PPM_DIGS to maintain a redundant digit, else set equal to NUM_PPM_DIGS
#define UNSIGNED_RANGE		ABS_SIGNED_RANGE
#define SPMF2_FRACTION_DIGS SPMF_FRACTION_DIGS
#define NUM_PPM_COUNTERS	12									// this is the number of counters, it is not changeable.


extern int Modulus[18];
extern const int ModPowers[18];	

extern int modulus_1[18];
extern const int ModPowers_1[18];

extern int modulus_2[8];
extern const int ModPowers_2[8];

extern int modulus_3[8];
extern const int ModPowers_3[8];

extern int modulus_4[8];
extern const int ModPowers_4[8];

extern int modulus_5[8];
extern const int ModPowers_5[8];

extern int modulus_6[8];
extern const int ModPowers_6[8];

#endif

