//	Project:						RNS-APAL
//	Authors:						Eric B. Olsen, Rakivaea Kvitting
//	Link to repository:				https ://github.com/MaitrixLLC/RNS-APAL
//	Link to CC BY-NC-SA license:	https ://creativecommons.org/licenses/by-nc-sa/4.0/
//
//	RNS-APAL is protected under the Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) License
//	
//	You are free to:
//	Share - copy and redistribute the material in any medium or format.
//	Adapt - remix, transform, and build upon the material.
//
//	The licensor cannot revoke these freedoms as long as you follow the license terms.
//
//	Under the following terms:
//	Attribution - You must give appropriate credit, provide a link to the license, and indicate if changes were made.You may do so in any reasonable manner, but not in any way that suggests 
//				  the licensor endorses you or your use.
//	Non Commercial - You may not use the material for commercial purposes.
//	Share Alike - If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.
//	No additional restrictions - You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.
//
//	Original Release:	V0.10:	June 5, 2016, Eric B. Olsen 
//	
//						V0.101:	June 6, Added license text
//						V0.102: June 7, 2016, Fixed SPPM::Prints() method to properly check sign flags same as Print() method
//						V0.103: June 8, 2016, Fixed SPMF::Prints() method and fixed Sqrt() method to pass back error code
//						V0.104: July 17, 2016, Added return codes for certain Assign functions, added void PPM constructor,
//												and modified demo routine to provide better operation and explanation, including adding product summation ex.
//						V0.105: October 22, 2017, Added full MOD to PPMDigit::Sub function, helps stabilize library for software testing
//												re-organized config.h and config.cpp for easier user adjustment of configuration with much less crashes,
//												added PPM::Div6 and PPM::Div7 as experiemental divide routines that exhibit less cycles,
//												added extended Euclidean routine as default option for inverse multiplication,  
//												continues to support LUT methods for inverse multiplication as user settable option
//						V0.106:	October 25, 2017, Added a define to set the default console width.  Located in "config.h", good for Win10 console which is wider, and much better.
//						V0.107: December 31, 2019, Modified print demo routine for future expansion.
//						V0.200: January 31, 2020, Added an init function, console width function, print remainder function, and a function to validate the modulus


#if defined(__linux__) && !defined(_WIN32)
#include <inttypes.h>
typedef int64_t __int64;
#else

#endif
#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "utilities.h"
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

#include "ppm.h"
//#include "pmf.h"
#include "mrn.h"
#include "sppm.h"


// constructor should also call base class constructor with the argument
// NO RANGE CHECK AT THIS TIME
SPPM::SPPM(__int64 x) : PPM(x)
{

// if x is within valid ppm range, and also within positive SPPM range
// then create a signed integer given the range x and the sign "sign"

//	cout << this->PPM::Prints() << endl;

	if(x < 0) {
		SignFlag = 1;
		this->Complement();
	}
	else {
		SignFlag = 0;
	}
		
	SignValid = 1;

}

// trying this as a way to solve the assignment of negative values to variables with partial power modulus
SPPM::SPPM(SPPM* copyval, __int64 x)
{
	int power, digval;
	int negflag = 0;

	if (x < 0) {
		x = -x;		// need absolute value to support signed SPPM2 as derived
		negflag = 1;
	}


	NumDigits = copyval->NumDigits;
	PowerBased = copyval->PowerBased;					// start with assumption that class is power based

	Mod2_index = copyval->Mod2_index;
	Mod5_index = copyval->Mod5_index;

	Rn.resize(NumDigits);				// make the Rn array as large as the base class instance

	for (int i = 0; i < copyval->NumDigits; i++) {


		digval = copyval->Rn[i]->Digit;
		power = copyval->Rn[i]->Power;
		Rn[i] = new PPMDigit(digval, i, power);

		Rn[i]->CopyDigit(copyval->Rn[i]);			// copy the whole arg digit into the new digit
		Rn[i]->Power = copyval->Rn[i]->PowerValid;	// derive the new partial power type


		Rn[i]->Digit = x % Rn[i]->Power;


	}

	if (negflag == 1) {
		this->Complement();
		SignFlag = 1;
	}
	else {
		SignFlag = 0;
	}

	SignValid = 1;


	Clear_Counters();
	ena_dplytrc = 0;

}

// destructor
SPPM::~SPPM(void)
{



}

// create a print that also prints the state of the sign flags
// this version, prints raw values with decimal point position
// the version does not alter the values sign state
string SPPM::Prints(int radix)
{
SPPM *temp = new SPPM(0);
int sign;

	string output;

	std::stringstream ss;	// get a stringstream 

	temp->Assign(this);		// use a copy to not affect the state of the class

	if(temp->GetSignValid()) {
		ss << "(V)";

	}
	else {
		ss << "(U)";
	}

	sign = temp->CalcSign();		// calculate the sign by measuring the magnitude

	if(temp->GetSignFlag() != sign) {
		ss << "(E)";
	}

	if(sign) {		// if sign is negative
		ss << "-";
	}
	else {
		ss << "+";
	}


	ss << temp->PPM::Prints(radix);
	output.append(ss.str());

	delete temp;
	return(output);

}

// a print10 that handles signed integers
// always prints as magnitude and sign, i.e., as normal decimal
// routine DOES NOT perform any sign correction, BUT does detect faulty sign flags using the (E) signal
// nor does it modify any sign flag status
// (future version could detect overflow if the abs_range is less than the full range)
string SPPM::Print10(void)
{
string output;
SPPM *temp = new SPPM(0);
SPPM *temp2 = new SPPM(0);
int calc_sign, sign_valid;

	std::stringstream ss;	// get a stringstream 

	temp->Assign(this);			// may want to consider using AssignPM for more generality with variable modulus numbers.
	
	if((sign_valid=temp->GetSignValid()) == SIGN_VALID) {		// print the status of sign validity
		ss << "(V)";
	}
	else {
		ss << "(I)";
	}

	if((calc_sign=temp->CalcSign()) == NEGATIVE) {		// force sign if invalid, if negative ...
		if(sign_valid) {
			if(SignFlag == calc_sign) {
				ss << "-";
			}
			else {
				ss << "(E)-";			// denotes error in sign flag system
			}
		}
		else {
			ss << "-";
		}

		temp2->PPM::Sub(temp);				// perform complement to print magnitude, (not negate)
		temp->PPM::Assign(temp2);			// directly write the value
	}
	else {
		if(sign_valid) {
			if(SignFlag == calc_sign) {
				ss << "+";
			}
			else {
				ss << "(E)+";			// denomtes error in sign flag system
			}
		}
		else {
			ss << "+";
		}
	}

	ss << temp->PPM::Print10();			// print the magnitude value
	output.append(ss.str());

	delete temp2;
	delete temp;
	return(output);

}

// a print(int radix) that handles signed integers
// always prints as magnitude and sign, i.e., as normal decimal
// routine DOES NOT perform any sign correction, BUT does detect faulty sign flags using the (E) signal
// nor does it modify any sign flag status
// (future version could detect overflow if the abs_range is less than the full range)
string SPPM::Print(int radix)
{
string output;
SPPM *temp = new SPPM(0);
int calc_sign, sign_valid;

	std::stringstream ss;	// get a stringstream 

	temp->Assign(this);			// may want to consider using AssignPM for more generality with variable modulus numbers.
	
	if((sign_valid=temp->GetSignValid()) == SIGN_VALID) {		// print the status of sign validity
		ss << "(V)";
	}
	else {
		ss << "(I)";
	}

	if((calc_sign=temp->CalcSign()) == NEGATIVE) {		// force sign if invalid, if negative ...
		if(sign_valid) {
			if(SignFlag == calc_sign) {
				ss << "-";
			}
			else {
				ss << "(E)-";			// denotes error in sign flag system
			}
		}
		else {
			ss << "-";
		}

		temp->PPM::Complement();		// perform complement to print magnitude, (not negate)

	}
	else {
		if(sign_valid) {
			if(SignFlag == calc_sign) {
				ss << "+";
			}
			else {
				ss << "(E)+";			// denomtes error in sign flag system
			}
		}
		else {
			ss << "+";
		}
	}

	ss << temp->PPM::Print(radix);			// print the magnitude value
	output.append(ss.str());

	delete temp;
	return(output);

}

// recently modified to support a more robust approach
// the signed value is checked for true sign of magnitude, and then a conversion to 64 bit signed integer is performed
__int64 SPPM::Decimal64(void)
{
__int64 val;
PPM *temp = new PPM(0);

	temp->Assign(this);

	int calc_sign = this->CalcSign();			// get true sign of magnitude of the value

	if(calc_sign == NEGATIVE) {
						
		temp->Complement();
		val = -temp->Convert();

	}
	else {
		val = temp->Convert();
	}

	delete temp;

	return(val);

}

// SPPM assign function, simply calls PPM::Assign, but also copies flags
// (there should not be a SPMM2::AssignPM method, since it is assumed partial power values are positive only)
void SPPM::Assign(SPPM *sppm)
{

	this->SignFlag = sppm->SignFlag;
	this->SignValid = sppm->SignValid;

	PPM::Assign(sppm);

}

// signed integer Assign helper function
void SPPM::Assign(__int64 x)
{
SPPM *temp = new SPPM(0);

	if(x < 0) {

		this->PPM::Assign(-x);			// must assign a positive value, so negate the argument!
		this->Complement();
		this->SignFlag = NEGATIVE;
		this->SignValid = SIGN_VALID;

	}
	else {

		this->PPM::Assign(x);
		this->SignFlag = POSITIVE;
		this->SignValid = SIGN_VALID;
	}

delete temp;
}

// this routine assigns a value represented in an ascii string of variable length
// the string can include a sign (+ or -), and can include a decimal format (all numbers)
// or a hexadecimal format which is preceded by a "0x"
// Resets the current PPM modulus format to that of the derived format (Power Value)
void SPPM::Assign(string sval)
{
int base = 10;			// assume a base of ten
int start = 0;
int sign = 1;
std::vector<char> s(sval.size()+1);

	std::copy(sval.begin(), sval.end(), s.begin());			// copy string into vector for easier processing

	this->Assign((__int64)0);				// init sppm to zero first

	if(s[0] == '+') {
		start = 1;		// allow a single plus sign at start, no minus
	}
	else if(s[0] == '-') {
		sign = -1;
		start = 1;
	}
	
	if((s[start] == '0') && (s[start+1] == 'x')) {
		base = 16;
		start += 2;
	}

	for(int i=start; i<sval.size(); i++) {		// make sure that all characters are legal

		this->Mult(base);			// each time through, multiply by the base

		if((s[i] >= '0') && (s[i] <= '9')) {
			this->Add(s[i] - '0');			
			}
		else if((s[i] >= 'a') && (s[i] <= 'f')) {
			if(base == 16) {
				this->Add((s[i]-'a')+10);
			}
			else {
				cout << "Error in string format, returning" << endl;
				this->Assign((__int64)0);
				return;
			}
		}
		else if((s[i] >= 'A') && (s[i] <= 'F')) {
			if(base == 16) {
				this->Add((s[i]-'A')+10);
			}
			else {
				cout << "Error in string format, returning" << endl;
				this->Assign((__int64)0);
				return;
			}
		}
		else {
			cout << "Error in string format, returning" << endl;
			this->Assign((__int64)0);
			return;
		}

	}

	this->Mult(sign);

}


// modified to support sign processing
void SPPM::Increment(void)
{

	this->Add((__int64)1);


}

// modified to support sign processing
void SPPM::Decrement(void)
{

	this->Sub((__int64)1);

}


// Add operation of new signed integer type does not normally sign extend to simulate the hardware
// the p's-complement Add works regardless of sign bit;
// If signs bits are valid, and are same, there is no change, and sign remains valid
// if signs are valid, and they are different, then the result sign is invalid, or not known
// if any sign is invalid, then the result sign is invalid
// (now properly handles the result sign for when an argument is zero)
void SPPM::Add(SPPM *arg)
{
int zero;

	zero = this->Zero();
	PPM::Add(arg);		// do the add on magnitude value

	if(this->GetSignValid()) {
		if(arg->GetSignValid()) {

			if(arg->Zero()) {
			
			}
			else if(zero) {
				this->SignFlag = arg->SignFlag;
			}
			else if(this->GetSignFlag() != arg->GetSignFlag()) {
				this->SignValid = 0;		// invalidate the sign, adding different signs
			}

		}
		else {		// the arg is not known, so invalidate
			this->SignValid = 0;		// invalidate the sign, adding unknown signs results in an unknown sign
		}
	}	


	this->ZeroClean();		// clean up zeros
}

// Add a signed integer to an SPMM2
void SPPM::Add(__int64 x)
{
SPPM *sppm_x = new SPPM(0);

	sppm_x->Assign(x);
	this->Add(sppm_x);

	delete sppm_x;

}

// Sub is performed in one step, provided signs are VALID
// it is assumed the user application is responsible for detecting overflow if required
// If the signs are different, the sign flags are invalidated at result
void SPPM::Sub(SPPM *arg)
{
int zero;

	zero = this->Zero();
	PPM::Sub(arg);

	if(this->GetSignValid()) {
	
		if(arg->GetSignValid()) {

			if(arg->Zero()) {
				
			}
			else if(zero) {
				this->SignFlag = !arg->SignFlag;
			}
			else if(this->GetSignFlag() == arg->GetSignFlag()) {
				this->SignValid = 0;		// invalidate the sign, adding different signs
			}

		}
		else {		// the arg is not known, so invalidate
			this->SignValid = 0;		// invalidate the sign, adding unknown signs
		}
	}	

	this->ZeroClean();
}


// Add a signed integer to an SPMM2
void SPPM::Sub(__int64 x)
{
SPPM *sppm_x = new SPPM(0);

	sppm_x->Assign(x);
	this->Sub(sppm_x);

	delete sppm_x;

}

// New signed integer multiply uses p's-complement plus validated sign magnitude
// First: Multiply the integers regardless
// If the signs are known beforehand, then calculate sign and return, else
//   version A: if any one sign not known, sign extend result and set signs appropriately
//   version B: if any one sign not known, sign extend during multiply - emulates hardware more closely

// SHOULD JUST SIMPLY SET SIGN TO INVALID IF UNKNOWN, DO NOT SIGN EXTEND HERE!
void SPPM::Mult(SPPM *arg)
{

	this->PPM::Mult(arg);			// complement math works, do it!
									// (need to build a version that automatically sign extends)
									// (to simulate the hardware)
	if(this->GetSignValid()) {
		if(arg->GetSignValid()) {

			if(this->GetSignFlag() == arg->GetSignFlag()) {
				this->SignFlag = POSITIVE;		// positive value, signs are same
			}
			else {
				this->SignFlag = NEGATIVE;
			}
			
		}
		else {
			this->SignValid = SIGN_INVALID;
		}
	}
	else {
		this->SignValid = SIGN_INVALID;		// ok, this is redundnat
	}

	this->ZeroClean();

}



// This is a a multiply by a signed integer - Helper Function
void SPPM::Mult(__int64 x)
{
SPPM* sppm_x = new SPPM(this, (__int64)0);			// recently made this change to fix GoldDiv, EO 1/2020

	sppm_x->Assign(x);
	this->Mult(sppm_x);

	delete sppm_x;

}


// Divide the numbers using sign flags if they are valid
// Complement negative numbers before the divide,
// if result should be negative, then negate result
// if any sign flags are invalid, then perform CalcSign
// remainder will be returned as same sign as quotient
int SPPM::Div(SPPM *arg, SPPM *rem)
{
int x_sign = POSITIVE;
int y_sign = POSITIVE;
int error;


	SPPM *x = new SPPM(0);
	x->Assign(this);

	SPPM *y = new SPPM(0);
	y->Assign(arg);


	if(this->GetSignValid()) {           // If sign is valid, use it
		if((x_sign = this->GetSignFlag()) == NEGATIVE) {
			x->Complement();
		}
	}
	else {
		if((x_sign = this->CalcSign()) == NEGATIVE) {            // calclate sign if sign is not valid
			x->Complement();
		}
	}

	if(arg->GetSignValid()) {            // if sign is valid, use it
		if((y_sign = arg->GetSignFlag()) == NEGATIVE) {
			y->Complement();
		}
	}
	else {
		if((y_sign = arg->CalcSign()) == NEGATIVE) {      // calculate the sign if sign not valid
			y->Complement();
		}
	}

	error = x->DivStd(y, rem);              // Integer divide is rudimentarily a method on positive integers only

	if(x_sign != y_sign) {         // if signs differ, and answer not zero, then answer is negative
		if(!x->Zero()) {
			x->Complement();              // must remember to complement the result magnitude if negative and not zero
			this->SignFlag = NEGATIVE;
		}
		else {
			this->SignFlag = POSITIVE;
		}
	}
	else {
		this->SignFlag = POSITIVE;
	}

	if(x_sign == NEGATIVE) {            // always make remainder the same sign as dividend
		if(!rem->Zero()) {
			rem->Complement();
			rem->SignFlag = NEGATIVE;
		}
		else {
			rem->SignFlag = POSITIVE;
		}
	}
	else {
		rem->SignFlag = POSITIVE;
	}
	
		
	this->SignValid = 1;		// make sure to set the sign as valid
	rem->SignValid = 1;
	this->PPM::Assign(x);		// make sure to assign the magnitude answer to this after possible complement()

	delete x;
	delete y;

	return(error);
	
}

// Negate the number, and service sign flags if valid
// therefore, if sign flag is not valid, then the operation is basically a complement
// if the negated value is zero, then the value is correctly signed
void SPPM::Negate(void)
{

	if(!this->Zero()) {		// if not zero, then Negate (zero assumed to always be positive)
		
		this->Complement();	// complement() if not zero()
		
		if(SignValid) {

			if(SignFlag == NEGATIVE) {
				SignFlag = POSITIVE;
			}
			else {
				SignFlag = NEGATIVE;
			}
		}
	}
	else {
		SignFlag = POSITIVE;
		SignValid = SIGN_VALID;
	}


}

// perform absolute value using complement function if SignValid IS true
// or perform a sign extend function if SignValid is NOT true
// this algorithm is close approximation of hardare Rez-9 design
// this routine sets a negative number to a positive number, and sign extends if neccesary
void SPPM::Abs(void)
{

	if(!SignValid) {
		CalcSetSign();		// calculate and set sign if the sign is not valid
	}

	if(SignFlag == NEGATIVE) {
		SignFlag = POSITIVE;
		this->Complement();
	}
		
}

// If this is greater than arg, then return TRUE (1)
// This routine will trust the sign valid flags, and use them to make sign comparison similar to hardware
// if any sign valid flag is NOT valid, then the routine will determine actual sign by magnitude
int SPPM::Compare(SPPM *sppm)
{
int this_sign, arg_sign;
int this_valid, arg_valid;

	
	this_valid = this->GetSignValid();
	arg_valid = sppm->GetSignValid();

	if((this_valid == SIGN_VALID) && (arg_valid == SIGN_VALID)) {
		
		this_sign = this->GetSignFlag();		// we need the sign flags, get them
		arg_sign = sppm->GetSignFlag();

		if((this_sign == POSITIVE) && (arg_sign == NEGATIVE))  {
			return(1);
		}
		else if((this_sign == NEGATIVE) && (arg_sign == POSITIVE))  {
			return(0);
		}
		else {
			return(this->PPM::Compare(sppm));      // values are positive, return straight PPM compare
		}
	}
	
	if(!this_valid) {
		this_sign = this->CalcSetSign();		// get the magnitude based sign flags, this version does auto-extend arguments
	}
	else {
		this_sign = this->GetSignFlag();
	}
	
	if(!arg_valid) {
		arg_sign = sppm->CalcSetSign();
	}
	else {
		arg_sign = sppm->GetSignFlag();
	}

	if((this_sign == POSITIVE) && (arg_sign == NEGATIVE))  {
		return(1);
	}
	else if((this_sign == NEGATIVE) && (arg_sign == POSITIVE))  {
		return(0);
	}
	else {
		return(this->PPM::Compare(sppm));      // values are positive, return straight PPM compare
	}

}


// gets the last value in positive range and assigned to the PPM argument
// now modified to return a range of half the ABS_SIGNED_RANGE, where ABS_SIGNED_RANGE = N is a product of first N number of modulus
// MODIFIED TO USE MODDIV AND BASE EXTENSION AND POWER BASED MODULUS, PROVIDES FOR MAXIMUM RANGE (ALL MODULUS USED)
// RIGHT NOW, WORKS WITH ODD AND EVEN NUMBER SYSTEMS, AND WORKS WITH SYSTEMS HAVING REDUNDANT DIGITS
int SPPM::GetAbsRange(PPM *ppm)
{
int error = 0;
PPM *two = new PPM(2);
PPM *rem = new PPM(0);

	if(ModTable::returnNumDigits() == ABS_SIGNED_RANGE) {
		ppm->Assign((__int64)0);								// case where extended digits is not supported
	}
	else {
		error = PPM::GetRange(ppm, ABS_SIGNED_RANGE);		// get the machine number range when extended digits are supported
	}
	ppm->Decrement();									// assumed two's modulus exist, range is always even, max value always odd
														// CHANGE! support both cases, even and odd ranges
//	cout << "abs_signed_range = " << ppm->Print10() << endl;
//	cout << "ppm: " << ppm->Prints() << endl;

	if(ppm->Rn[Mod2_index]->PowerValid) {

		if(ppm->Rn[Mod2_index]->Digit & 0x1) {     // for EVEN number systems, should always be true
			ppm->Decrement();
			ppm->ModDiv(2);					// divide value by two  (multiply by multiplicative inverse of two)
//			ppm->PrintDemoPM();
			ppm->ExtendPart2Norm();			// base extend with partial power digits (DOES NOT WORK WITH NO EXTENDED DIGITS)
		}
		else {                              // the total range of number system is ODD
			ppm->ModDiv(2);					// divide value by two  (multiply by multiplicative inverse of two)
//			ppm->PrintDemoPM();
			ppm->ExtendPart2Norm();				// base extend with partial power digits (DOES NOT WORK WITH NO EXTENDED DIGITS)	
			ppm->Decrement();				// need to adjust for ODD number systems for conventional positive range
		}

	}
	else {
		cout << "ERROR IN GetAbsRange(), power 2 not valid" << endl;
		wait_key();
	}

	delete rem;
	delete two;
	return(error);

}



// return true if zero, and set sign flag positive, set sign valid flag true
int SPPM::ZeroClean(void)
{
int zeroflag = 0;

	if(zeroflag = this->PPM::Zero()) {
		this->SignFlag = POSITIVE;
		this->SignValid = SIGN_VALID;
	}

	return(zeroflag);
}


// calcualte the sign, and returns the true sign of an SPPM based on its value's range only
// Negative returns 1, else Positive returns 0
int SPPM::CalcSign(void)			
{	
	
	PPM *range = new PPM(0);
	GetAbsRange(range);

	int sign = this->PPM::Compare(range);	// return true if: this > (range)


	delete range;
	return(sign);

}

// calcualte the sign of an SPPM based on its value's range only, then sets sign flag and sign valid flag and returns the true sign 
// 
int SPPM::CalcSetSign(void)			
{	
	
	PPM *range = new PPM(0);
	GetAbsRange(range);

	int sign = this->PPM::Compare(range);	// return true if: this > (range), true means it's negative


	if((SignValid) && (SignFlag != sign)) {                 // Code for sign error detection option
			cout << "ERROR: sign flag set incorrectly in CalcSetSign" << endl;
			cout << "Hit any key to correct and continue" << endl;
			wait_key();
	}


	this->SignFlag = sign;	// sign the value
	this->SignValid = 1;


	delete range;
	return(sign);

}

// returns the value of the sign flag,
// True if Negative, False if Positive, and -1 for INVALID
int SPPM::GetSignFlag(void)			// returns the value of the sign flag
{

//	if(SignValid) {
		return(SignFlag);
//	}
//	else {
//		return(-1);
//	}

}

// returns the value of the SignValid flag
int SPPM::GetSignValid(void)
{
	return(SignValid);
}

