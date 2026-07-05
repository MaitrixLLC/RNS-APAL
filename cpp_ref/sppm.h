#ifndef SPPM_CLASS
#define SPPM_CLASS

#include <vector>
using std::vector;
#include <iostream>
#include <string>

#include "ppm.h"

using namespace std;


// sign flag defines
#define NEGATIVE	1		// sign flag is set when value is negative
#define POSITIVE	0		// sign flag is cleared when value is positive or zero
#define SIGN_VALID		1
#define SIGN_INVALID	0


// Class object for Signed RNS integer representation
// Derived class from base class PPM
class SPPM : public PPM {

protected: 


public:

	SPPM(__int64 x);		// constructor, creates a signed integer using a magnitude and sign flag
	SPPM(SPPM* copyval, __int64 x);		// constructor, creates a signed integer using a modulus structure copied from SPPM* copyval
	~SPPM(void);
	
	int SignValid;			// flag to signify if the sign flag is valid or not
	int SignFlag;			// actual sign flag, for signed magnitude approach (which will be combined with P's complement


	string Prints(int radix);	// prints the raw SPPM, supporting signed numbers & overflow detection
	string Print(int radix);
	string Print10(void);	// prints the signed integer SPPM in decimal, printing sign & and overflow/underflow

	__int64 Decimal64(void);	// converts a residue number into a signed 64 bit integer 

	void Assign(SPPM *sppm);	// Assign an SPPM number, (i.e., need to service the sign flags)
	void Assign(__int64 x);		// assign a signed number directly for testing purposes
	void Assign(string sval);	// assign a signed number from string input

	void Increment(void);		// basic SPPM increment	
	void Decrement(void);		// basic SPPM decrement

	void Add(__int64 x);		// Add a signed int64 integer to the SPPM
	void Add(SPPM *sppm);		// Add signed numbers

	void Sub(__int64 x);		// Subtract a signed int64 integer from the SPPM
	void Sub(SPPM *sppm);		// Subtract signed numbers 

	void Mult(__int64 x);		// Multiply a signed PPM integer by a signed int x, Helper Function
	void Mult(SPPM *sppm);		// Multiply signed integers, experiment with error reversal algorithm
								// Multiply will handle magnitude based sign detection automatically
								
	int Div(SPPM *sppm, SPPM *rem);		// Clearly, sign detection will be performed first, then unsigned division.
								// (sign flags could be a benefit in this case)
//	int Sqrt(void);				// return the integer square root of the number, return error if number is negative (Do we really need, or just call PPM::sqrt()?)

	void Negate(void);			// calculate the arithmetic complement (negate), change sign if valid  "M's complement"
	void Abs(void);				// take the absolute value of the SPPM integer
//	int IsEqual(SPPM *sppm);	// (NOT USED IN RE-WRITE, NO DEFINITION FOR THIS) is the SPPM equal to this?  (also considers sign and zero, of course)
//	void Complement(void);		// calculates the arithmetic complement, do not change sign flag status
//	int Extend(void);			// extend a signed number? SHOULD NOT NEED!!

	int CalcSign(void);			// calculates the sign based on range, and returns the true sign based on range only, does NOT set or affect sign or sign valid flags
	int CalcSetSign(void);		// calculates the sign based on range, and sets the sign flag and sign valid flag  appropriately (this is SignExtend())
	int GetSignFlag(void);		// returns the Sign flag
	int GetSignValid(void);		// returns the Sign Valid flag
	int GetAbsRange(PPM *ppm);	// returns absolute range of the signed value, and return it into a PPM argument

	int Compare(SPPM *sppm);	// Compare two signed integers, and Sign if required (STILL NEED)

private:

	int ZeroClean(void);		// if the value is zero, return 1, and make sure it is positive and signvalid = 1
	
};


#endif
