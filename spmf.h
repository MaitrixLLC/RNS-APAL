#ifndef SPMF_CLASS
#define SPMF_CLASS
#include "ppm.h"
#include "sppm.h"

#include <vector>
using std::vector;
#include <iostream>
#include <string>

//#define MAX_PMF_DIGS		NUM_PPM_DIGS		// (make = 21 if fract=8 for R^3 system)		
												// (make = 32 if fract=16 for R^2 + 4.95e+11
//#define SPMF_FRACTION_DIGS	12
//#define PMF_FRACT_DIGS	15

//
// Unsigned Fractional (fixed point) RNS representation class
//  (might get sumped in lieu of a Signed version
//
class SPMF : public SPPM {

private:
	int NumFractDigits;				// this is essentially the decimal point position
	int NormalFractDigits;			// the normal number of fractional digits

public:
//	PMF::PMF(int denom);			// create a PMF fraction by inverting an integer
	SPMF(__int64 x); 
	~SPMF(void);

	string Prints(void);			// print native SPMF with fractional point!
	string Prints(int radix);		// print native SPMF with fractional point using radix argument!
	double PrintFPM_old(void);		// returns a standard floating point number, calculated using std. FP, supports partial modulus
	double PrintFPM(void);			// returns a standard floating point number, same as above, but revised by chatGPT
	string Print(int radix);		// standard string output fractional print
	string PrintAbsPM(int radix);	// cleaned up version of PrintPM2(int r);
	string PrintPM(int radix);      // signed-aware partial-modulus fractional print

	void AssignInverse(__int64 x);		// assigns the inverse of x, i.e., 1/x
	
	void GetUnit(PPM *ppm);			// returns the unit value via the ppm argument
	void GetUnitPM(PPM *ppm);		// returns the unit value via the ppm argument, supports partial power modulus
	void AssignUnitPM(void);		// assigns the SPMF a unit value (1.0)  (shortcut routine)
	int AssignFP(string fval);		// Assignment takes arbitrary string as input with a single decimal point and optional +/- prefix, returns true if successful
	void AssignRatio(__int64 x, __int64 y);  // Assignment of a fixed fraction as a ratio of two long long integers.  x value times fractional range cannot exceed underlying PPM number system


									// NOTE that Add(PPM) and Mult(PPM) are still defined in base class
	void Add1(__int64 x);				// NEED TO DERIVE THESE BASE CLASSES THE RIGHT WAY! add a whole unit to the FRN
	void Add(SPMF *arg);			// NEED TO DERIVE RIGHT - add a signed fractional, do not sign extend, but try to preserve sign flag if possible

	void Sub1(__int64 x);			// NEED TO DERIVE RIGHT - subtract a whole unit to the FRN
	void Sub(SPMF *arg);			// NEED TO DERIVE RIGHT - subtract a signed fractional, do not sign extend, just preserve the sign if possible

	void Mult(__int64 x);			// NEED TO REVIEW DERIVED VERSION - Integer multiply with Sign Extend - fraction times unit - FRN * x
	void Mult(SPPM *sval);			// multiply a signed integer to the fractional residue type

	void MultStd(SPMF *arg);		// Calls the preferred version, use this call for applications.
	void Mult(SPMF *arg);			// fractional multiply FRN * FRN, using MRC
	void Mult2(SPMF *arg);			// fractional multiply FRN * FRN, using integrated word processing, doesn't work yet, doesn't look to be faster either
	void Mult3(SPMF *arg);
	void Mult4(SPMF *arg);			// simple stable method using mixed-radix conversion and truncation, measures against magnitude for sign, uses complement logic
	void Mult4b(SPMF *arg);			// method using mixed-radix conversion and truncation, measures against magnitude for sign detect, uses correction constant for sign correction
	void Mult5(SPMF *arg);			// multiply using fast "power based" reconversion
	void Mult5PM(SPMF *arg);
	void Mult5_demo(SPMF *arg);		// Demo multiply for patent write up
	void Mult6(SPMF *arg);			// (PROBLEM WITh LARGE NUMBER OF DIGITS, OR LARGE WHOLE) - Dual accumulator, fast recombination method with round-up
	void FracSep(SPMF *frac, PPM *whole);		// fractional value seperator
	void Mult_frac(SPMF *frac1, SPMF *frac2);	// routine to multiply two fractional values only

	void I2N_Convert(void);
	void MAC(SPMF *arg1, SPMF *acc);	// Multiply and Accumulate function
	void ProdSum(SPMF *arg1, SPMF *arg2, SPMF *arg3);		// sum of products test routine, just do sum of two products for test and demo
	
	void Div(SPMF *arg);				// prototype fractional divide by integer
	void DivPM(SPMF *arg);				// (experimental) prototype fractional divide with power modulus by int divide
	void Inverse(void);					// calculate the inverse of the PMF number
	void AssignInversePPM(PPM* denom);  // prototype for creating an SPMF inverse using a PPM integer


	int scale(SPMF *arg);			// test routine to scale a number D, such that 0 < D < 1.0  (experimental)
									// returns 0 if no scale, else -1 for each "left shift", or +1 for each "right shift"
	int scalePM(SPMF *arg, SPMF *pmf2, int &width);	// container method which calls the best scaling routine developed, or version under test
	int scalePM4(SPMF *arg1, SPMF *arg2, int &width); // same as PM3 but cleaned up
	int scalePM5(SPMF *arg1, SPMF *arg2, int &width); // version of PM3 which supports Rez-9 testing
	int scalePM6(SPMF *arg1, SPMF *arg2, int &width); // same as PM5 but used for debugging and testing

	int scale_chk(PPM *ppm);		// (experimental) routine indicates if the two's modulus is the largest digit, or ties with largest, returns true
	int ScaleInt(PPM *n, PPM *d, SPMF *pmf_n, SPMF *pmf_d, int &width);

	void GoldDiv(SPMF *arg);				// first version of a divide by multiplication routine using Goldschmidt routines, make RNS range^2 compared with range 
											// of operand, and don't let the divide resut in a number too large to be contained in range of operand as simple rule.
											// we don't prove the required range requirements here, but suffice to say there is no overflow or underflow detection in this
											// algorithm at this time.  This could be added later as a problem for the student!	 
	void GoldDivAbs(SPMF *arg);			    // prototype Goldschmidt division routine, assumes the divisor is properly scaled for now, only positive numbers
	void GoldDivAbsDebug(SPMF *arg);		// same or similar as above, but with printfs for testing
	void GoldDivAbs2(SPMF *arg);			// preliminary testing of Goldschmidt algorithm variations in persuit of more efficeint routines

	void GoldDiv(PPM *n, PPM *d, PPM *rem);			// prototype Goldschmidt integer division routine, converts integer to smallest avail fraction

	void NormalFract(void);		// return to a normalized fractional format with equivalent value (DON'T CONFUSE with PPM->NORMALIZE)
	void NormalFract1(void);	// same as above, but with printf for testing

	int Sqrt(void);		// take the fractional square root
	
	int GetMaxRootFraction(SPMF *max_val);		// returns the maximum value of a fractional type for which std methods will work, based on your chosen system.
	int GetMaxAbsFraction(SPMF *max_val);		// returns the largest represent able fraction in the SPMF system.
	int GetUmp(SPMF *ump);

	int GetCurFractPos(void);		// returns the fraction point position
	int GetNormFractPos(void);		// returns the normal fraction point position

	// transcedental, exponential and trigonometric functions here

	void AssignTaylor_E(void);		// assigns the value of 'e' using the Taylor expansion

};



#endif

