#ifndef MRN_HEADER
#define MRN_HEADER

#include "ppm.h"
#include <vector>
using std::vector;
#include <iostream>
#include <string>

using namespace std;



// -----------------------------------------------------------------
// simple class to store the Mixed Radix digit value and digit power
// -----------------------------------------------------------------
class MRDigit {

public:

	MRDigit(int Digit, int Power, int Skip);

	int Digit;
	int Power;
	int Skip;		// we will track skipped digits since it's easier to read and understand

};

// class that stores a basic Mixed Radix number, in the form of digits and their corresponding power factors
// these numbers are ordered, as previous digit powers are included in the 'implied' power base
// Mixed radix numbers of this class have dynamic power structure (may include skipped digit positions and different sets of powers)
class MRN {

public:

	MRN(PPM *ppm, int &clocks);	// create an MRn number class using a Residue input (implicit conversion) wiith clock counting
	MRN(PPM *ppm);			// create an MRn number class using a Residue input (implicit conversion)
	~MRN();				// destructor to deallocate the memory

	void Print(void);			// print the MRN, including skipped digits
	__int64 Decimal64(void);	// print the value as __int64
	unsigned long long Decimal_ull(void);	// a better conversion routine which returns unsigned long long 64 bit type, otherwise, same as Decimal64


	int Convert(PPM *ppm);		// Convert the MRN to a PPM, via the argument
	int Convert2(PPM *ppm);		// Convert the MRN to a PPM, via the argument
	void Assign(MRN *mrn);		// assign another MRN to this
	void Assign2(PPM *ppm);		// create "assign" conversion to support partial power digit routines

	void ShiftRight(int n);		// simulates a barrel shift right (divide by 2*3*5*7*11 ...)
	void ShiftRight2(int n);	// older version simulates a barrel shift right using MRN vector splice (simulates barrel shift) (divide by ...*5*7*11*...*N)

	vector<MRDigit *> MRn;		// Mixed Radix vector number, contains a list of MRpairs

	void ConvertAdd(PPM *ppm, MRN *mrn);	// better do this first, if its possible, multiply should be as well
	void ConvertMult(PPM *ppm, MRN *mrn);	// test for proposed method of multiplication during conversion  (doesn't work)

	int NumDigs(void);			// returns the number of non-skipped digits of the MRN

	// routnes in support of crypt.cpp
	int get_mrn_n(PPM *n, int digcnt);		// get the mixed radix number starting at digcnt

};




#endif