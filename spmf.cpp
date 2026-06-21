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
#include "sppm.h"
#include "spmf.h"
#include "mrn.h"
//#include "rppm.h"



// constructor should also call base class constructor with the argument
SPMF::SPMF(__int64 x) : SPPM(x)
{

	NumFractDigits = ModTable::returnNumFracDigits();
	NormalFractDigits = NumFractDigits;

	if(NumFractDigits > ModTable::returnNumDigits()) {		// store the number of fractional digits
		printf("ERROR creating the SPMF class\r\n");
		wait_key();
	}

	

}



SPMF::~SPMF(void)
{




}

// returns the frational range of an original power SPMF number (no partial powers)
void SPMF::GetUnit(PPM *ppm)
{

	ppm->Assign(1);
	
	for(int i=0; i<NumFractDigits; i++) {

		ppm->Mult(ppm->Rn[i]->GetFullPowMod());			// multiply NumFract number of modulus

	}

}

// returns the fractional range of a partial power modulus
void SPMF::GetUnitPM(PPM *ppm)
{
	ppm->Assign(1);
	
	for(int i=0; i<NumFractDigits; i++) {

		ppm->Mult(this->Rn[i]->GetPowMod2());		// multiply NumFract number of modulus, modify to use partial power modulus

	}

}

// Assigns the value of 1.0 to the SMPF2 fractional number  (shortcut routine)
void SPMF::AssignUnitPM(void)
{

	this->GetUnitPM(this);

}

// this routine initializes the SPMF variable by assigning the string representation
// the string may be decimnal only, prefixed with a single + or - sign, 
// and may contain a single decimal point
// SUPPORTS POWER PM
int SPMF::AssignFP(string fval)
{
int base = 10;			// always a base of ten
int start = 0;
int sign = 0;			// 0 is positive, one is negative
int dec_flag = 0;		// decimal point detection flag
int frac_cnt = 0;		// number of fractional digits
int i;
std::vector<char> s(fval.size()+1);

PPM *dec_div = new PPM(1);
PPM *rem = new PPM(0);
SPMF *unit = new SPMF(0);


	std::copy(fval.begin(), fval.end(), s.begin());			// copy string into vector for easier processing

	this->PPM::Assign(0);			// init ppm base to zero first

	if(s[0] == '+') {
		start = 1;		// allow a single plus sign at start, no minus
	}
	else if(s[0] == '-') {
		start = 1;
		sign = 1;
	}

	for(unsigned int i=start; i<fval.size(); i++) {		// make sure that all characters are legal

		if((s[i] >= '0') && (s[i] <= '9')) {
			this->PPM::Mult(base);			// each time through, multiply by the base
			this->PPM::Add(s[i] - '0');	
			if(dec_flag) frac_cnt += 1;		// track the number of decimal digits
			}
		else if(s[i] == '.') {			// decimal point detected
			if(!dec_flag) {
				dec_flag = 1;			// decimal flag detected
			}
			else {
//				printf("Error: duplicate decimal point detected during Assign\n");
				return(0);
			}
		}
		else {
//			cout << "Error in string format, returning" << endl;
			this->PPM::Assign(0);
			return(0);
		}

	}

	// OK, now multiply by residue fractional range, and divide by decimal fractional range.

	// create a decimal fractional divisor	
	for(i=0; i<frac_cnt; i++) {
		dec_div->Mult(10);
	}

	unit->GetUnitPM(unit);			// assign a unit value to the unit variable

	this->PPM::Mult(unit);			// multiply by the RNS fractional range

//	cout << "this: " << this->Print10() << " dec_div: " << dec_div->Print10() << endl;

	this->ena_dplytrc = 0;
	this->PPM::DivStd(dec_div, rem);		// divide by the decimal fractional range

	// now determine if a round up is needed

//	cout << "remainder: " << rem->Print10() << endl;
//	cout << "dec_div:   " << dec_div->Print10() << endl;

	rem->Mult(2);					// increase remainder by two for comparison against dec_div
	rem->Increment();				// Used for no need for IsEqual in compare with dec_div
//	if(!dec_div->Compare(rem)) {
	if(rem->Compare(dec_div)) {
		this->PPM::Add(1);			// add one for round-up
//		printf("round up in AssignPM\n");
	}

	if(sign) {
		this->Negate();
	}

//	PPM *dec_div = new PPM(1);
//	PPM *rem = new PPM(0);
//	SPMF *unit = new SPMF(0);

	delete unit;
	delete rem;
	delete dec_div;

	return(1);		// return TRUE for OK

}

// this assign method allows exact ratios to be input into the residue fractional type
// NOT any ratio can be developed, since the RNS numbers used should keep an extra "redundant fractional range" in x value
void SPMF::AssignRatio(__int64 x, __int64 y)
{
SPPM *x_fr = new SPPM(x);
SPPM *y_fr = new SPPM(y);
PPM *rng = new PPM(0);
SPPM *ratio = new SPPM(0);
SPPM *rem = new SPPM(0);
	
	GetUnitPM(rng);		// get the fractional range into a PPM value

	ratio->PPM::Assign(rng);	// don't destroy the range, we need it later for round up
	ratio->SignFlag = POSITIVE;
	ratio->SignValid = SIGN_VALID;

	ratio->Mult(x_fr);	// perform an integer multiply of numerator by fractional range

	ratio->Div(y_fr, rem);

//	cout << "ratio " << ratio->Print(DEC) << endl;
	rem->Abs();
	y_fr->Abs();

//	cout << "rem " << rem->Print(DEC) << endl;
//	cout << "y_fr " << y_fr->Print(DEC) << endl;

	rem->Mult(2);

	if(rem->PPM::Compare(y_fr)) {    // if two times remainder is greater than divisor, then round up
//		printf("round up\n");
		if(ratio->SignFlag == POSITIVE) {
			ratio->PPM::Increment();
		}
		else {
			ratio->PPM::Decrement();
		}
	}

	this->Assign(ratio);		// cast the value into the ratio

//SPPM *x_fr = new SPPM(x);
//SPPM *y_fr = new SPPM(y);
//PPM *rng = new PPM(0);
//SPPM *ratio = new SPPM(0);
//SPPM *rem = new SPPM(0);

	delete rem;
	delete ratio;
	delete rng;
	delete y_fr;
	delete x_fr;
}

/*
// add a whole unit to the SPMF
void SPMF::Add1(int x)
{
int arg_sign = POSITIVE;

	if(x == 0) return;
	if(x < 0) {
		arg_sign = NEGATIVE;
		x = -x;
	}

	PPM *unit = new PPM(this, 0);	// this routine now supports derived partial power numbers for use with Goldschmidt
	GetUnitPM(unit);				// modified to support power based modulus
	unit->Mult(x);
	
	if(arg_sign == POSITIVE) {
		PPM::Add(unit);
	}
	else {
		PPM::Sub(unit);
	}
		
	if(this->Zero()) {
		this->SignFlag = 0;
		this->SignValid = 1;
	}
	else if(this->GetSignValid()) {
		if((this->CalcSign() == POSITIVE)) { 
			if(arg_sign == NEGATIVE) {
				this->SignValid = 0;
			}
		}
		else {
			if(arg_sign == POSITIVE) {
				this->SignValid = 0;
			}
		}
	}

	delete unit;
}
*/

// helper function designed for wide range of power modulus
void SPMF::Add1(__int64 x)
{
SPMF *arg = new SPMF(0);
SPMF *unit = new SPMF(0);

	arg->AssignPM(x, this);		// adopt the power modulus of the calling for this helper function
	unit->AssignPM(0, this);

	GetUnitPM(unit);
	arg->PPM::Mult(unit);

	this->Add(arg);

	delete unit;
	delete arg;
}

// signed fractional add
void SPMF::Add(SPMF *arg)
{

	this->SPPM::Add(arg);		// do a low level signed (derived) add

}

// helper function designed for wide range of power modulus
void SPMF::Sub1(__int64 x)
{
SPMF *arg = new SPMF(0);
SPMF *unit = new SPMF(0);

	arg->AssignPM(x, this);     // adopt the power modulus of the calling for this helper function
	unit->AssignPM(0, this);

	GetUnitPM(unit);
	arg->PPM::Mult(unit);

	this->Sub(arg);

	delete unit;
	delete arg;

}


void SPMF::Sub(SPMF *arg)
{

	this->SPPM::Sub(arg);		// just do a low level signed (derived) add
	
}


string SPMF::Prints(void)
{
SPMF *num = new SPMF(0);

	num->Assign(this);

	string output;			// where does this go after its passed back?
	std::stringstream ss;
	
	if(num->GetSignValid()) {
		ss << "(V)";
	}
	else {
		ss << "(I)";
	}

	if(num->CalcSign()) {		// if negative
		if((num->GetSignValid()) && (!num->SignFlag)) {		// if sign is valid, and not negative, then error
			printf("Sign magnitude error!\r\n");
			wait_key();
		}
		else {
			ss << "-";
		}
		
	}
	else {
		if((num->GetSignValid()) && (num->SignFlag)) {		// if sign is valid, and not negative, then error
			printf("Sign magnitude error!\r\n");
			wait_key();
		}
		else {
			ss << "+";
		}
		
	}	
	
	output.append(ss.str());
	
	for(unsigned int i=0; i<num->Rn.size(); i++) {
		
		std::stringstream ss;
		
		if(i == num->NumFractDigits) {		// print a fractional point
			ss << dec << ". ";
		}		
		if(!num->Rn[i]->Skip) {
			ss << dec << num->Rn[i]->Digit << " ";
		}
		else {
			ss << dec << "* ";
		}

		output.append(ss.str());

	}

	delete num;
	return(output);
}

// print the raw SPMF value, but also detect if sign is valid or not, and flag with (E) code if
// there is a sign flag discrepancy, this makes this method similar to other print methods
string SPMF::Prints(int radix)
{
SPMF *num = new SPMF(0);

	num->Assign(this);

	string output;			
	std::stringstream ss;
	
	if(num->GetSignValid()) {
		ss << "(V)";
	}
	else {
		ss << "(I)";
	}

	if(num->CalcSign() == NEGATIVE) {		// if negative by measurement
		if((num->GetSignValid()) && (!num->SignFlag)) {		// if sign is valid, and not negative, then error
//			printf("Sign magnitude error!\r\n");
//			wait_key();
			ss << "(E)-";
		}
		else {
			ss << "-";
		}
		
	}
	else {                                 // positive by measurement
		if((num->GetSignValid()) && (num->SignFlag)) {		// if sign is valid, and not negative, then error
//			printf("Sign magnitude error!\r\n");
//			wait_key();
			ss << "(E)+";
		}
		else {
			ss << "+";
		}
		
	}	
	
	output.append(ss.str());
	
	if(radix == DEC) {
		for(unsigned int i=0; i<num->Rn.size(); i++) {
		
			std::stringstream ss;
		
			if(i == num->NumFractDigits) {		// print a fractional point
				ss << dec << ". ";
			}		
			if(!num->Rn[i]->Skip) {
				ss << dec << num->Rn[i]->Digit << " ";
			}
			else {
				ss << dec << "* ";
			}

			output.append(ss.str());

		}
	}
	else if(radix == HEX) {
		for(unsigned int i=0; i<num->Rn.size(); i++) {
		
			std::stringstream ss;
		
			if(i == num->NumFractDigits) {		// print a fractional point
				ss << hex << ". ";
			}		
			if(!num->Rn[i]->Skip) {
				ss << hex << num->Rn[i]->Digit << " ";
			}
			else {
				ss << hex << "* ";
			}

			output.append(ss.str());

		}

	}
	else {
		printf("radix not supported\n");
	}

	delete num;
	return(output);
}


// print the SPMF using partial power modulus instead
// this was correct by chatGPT
double SPMF::PrintFPM(void)
{
	double val;
	int is_negative = 0;

	SPPM* num = new SPPM(0);

	//
	// Create intermediate copy of same format.
	//
	// IMPORTANT:
	// Do not normalize/base-extend a negative complement-coded value
	// directly.  First determine the sign, then convert to positive
	// magnitude for decimal/float conversion.
	//
//	num->AssignPM(this);			// always copies to unsigned type
	num->SPPM::Assign(this);

	//
	// Determine sign before normalization.
	// CalcSign() uses the complement/range interpretation.
	//
	if (num->CalcSign())
	{
		is_negative = 1;

		//
		// Optional diagnostic consistency check.
		//
		if (num->GetSignValid() && (num->GetSignFlag() != NEGATIVE))
		{
			printf("(V)Sign magnitude error before PrintFPM magnitude conversion!\r\n");
		}

		//
		// Convert to positive magnitude before Normalize().
		//
		// Prefer Abs() if it is the trusted signed-magnitude operation.
		// If Abs() is not correct for SPMF fractional values, replace
		// this with the correct complement-to-magnitude operation.
		//
		num->Negate();
	}
	else
	{
		is_negative = 0;

		//
		// Optional diagnostic consistency check.
		//
		if (num->GetSignValid() && (num->GetSignFlag() != POSITIVE))
		{
			printf("(V)Sign magnitude error before PrintFPM magnitude conversion!\r\n");
		}
	}

	//
	// Now the value should be non-negative.
	// Normalize/base-extension is safe on positive magnitude.
	//
	num->Normalize();

	if (num->GetSignValid())
	{
		printf("(V)");
	}
	else
	{
		printf("(I)");
	}

	if (is_negative)
	{
//		printf("-");		// printf routine already prints a negative sign
	}
	else
	{
		printf("+");
	}

	PPM* frac = new PPM(0);
	PPM* one = new PPM(0);

	SPMF::GetUnitPM(one);

	num->ena_dplytrc = 0;

	//
	// num is now positive magnitude, so unsigned PPM division is safe.
	//
	num->PPM::DivStd(one, frac);

	val = (double)(frac->Convert()) / (double)(one->Convert());
	val += (double)(num->Convert());

	if (is_negative)
	{
		val = -val;
	}

	delete one;
	delete frac;
	delete num;

	return val;
}

string SPMF::PrintPM(int radix)
{
	SPMF temp(0);

	//
	// SPMF is signed.  Make a signed copy of this value.
	//
	// Use the signed SPPM assignment path so that SignFlag and
	// SignValid are copied along with the residue digits.
	//
	temp.SPPM::Assign((SPPM*)this);

	//
	// Some operations, especially Add/Sub accumulation, may leave
	// the cached sign flag invalid.  Recompute it from the actual
	// complement/range representation before printing.
	//
	temp.CalcSetSign();

	//
	// The decimal conversion path should operate on a positive
	// magnitude.  If the value is negative, convert the temporary
	// copy to its absolute value and prepend '-'.
	//
	if (temp.GetSignFlag() == NEGATIVE)
	{
		temp.Abs();
		temp.CalcSetSign();

		return string("-") + temp.PrintAbsPM(radix);
	}

	//
	// Positive or zero.
	//
	return temp.PrintAbsPM(radix);
}

// print the SPMF using partial power modulus instead
double SPMF::PrintFPM_old(void)
{
double val;


	SPPM *num = new SPPM(0);	

	num->AssignPM(this);		// create a intermediate copy of same format

	num->Normalize();			// then  normalize that value for further processing

//	cout << "integer equivalent: " << num->Print10() << endl;
//	printf("\r\n");

	if(num->GetSignValid()) {
		printf("(V)");
	}
	else {
		printf("(I)");
	}
															// SIGN STUFF NEEDS TO BE LOOKED AT SINCE VPM SUPPORT ADDED
	if(num->CalcSign()) {		// if negative
		if((num->GetSignValid()) && (!num->SignFlag)) {		// if sign is valid, and not negative, then error
			printf("Sign magnitude error!\r\n");
//			wait_key();
		}
		else {
			printf("-");
		}

		num->Complement();
	}
	else {
		if((num->GetSignValid()) && (num->SignFlag)) {		// if sign is valid, and not negative, then error
			printf("Sign magnitude error!\r\n");
//			wait_key();
		}
		else {
			printf("+");
		}

	}	


	PPM *frac = new PPM(0);
	PPM *one = new PPM(0);
	SPMF::GetUnitPM(one);

//	cout << "printFPM one (getunit) equals:" << one->Print10p() << endl;


//	SPMF::GetUnit(one);

	num->ena_dplytrc = 0;
	num->PPM::DivStd(one, frac);		// NOTE: Div must support Power Modulus, in general

	val = (double)(frac->Convert())/(double)(one->Convert());
	val += (double)(num->Convert());
//	printf(" %5.20f ", val);
//	printf("\r\n");

	delete one;
	delete frac;
	delete num;

	return(val);

}

// print signed fractional type to a string object
// supports decimal, hexadecimal and binary format fractions
string SPMF::Print(int r)
{

	std::stringstream ss;
	std::stringstream ss2;
	string output;

	PPM *num = new PPM(0);	
	num->Assign(this);		// this is here		

	PPM *frac = new PPM(0);
	PPM *range = new PPM(0);
	SPMF::GetUnit(range);			// NOTE: GetUnit is strictly for NON partial power based numbers
	
	PPM *radix = new PPM(r);
	PPM *rem1 = new PPM(0);
	PPM *rem2 = new PPM(0);

//	cout << "this: " << this->Print10() << endl;

	SPMF *snum = new SPMF(0);
	snum->Assign(this);

	
	if(snum->GetSignValid()) {
		ss2 << "(V)";
	}
	else {
		ss2 << "(I)";
	}

	if(snum->CalcSign() == NEGATIVE) {		// if negative
		if((snum->GetSignValid()) && (!snum->SignFlag)) {		// if sign is valid, and not negative, then error
			ss2 << "(E)-";
//			printf("Sign magnitude error!\r\n");
//			wait_key();
		}
		else {
			ss2 << "-";
		}
		num->Complement();
	}
	else {                                   // if positive
		if((snum->GetSignValid()) && (snum->SignFlag)) {		// if sign is valid, and not negative, then error
			ss2 << "(E)+";
//			printf("Sign magnitude error!\r\n");
//			wait_key();
		}
		else {
			ss2 << "+";
		}
		
	}	
	
	output.append(ss2.str());

	
	num->ena_dplytrc = 0;
	num->DivStd(range, frac);
	
	ss << num->Print(r);		// print the whole part first, with header formatting

	num->Assign(frac);			// now work on the fractional portion

//#ifdef PRINT_REMAINDER
	if (print_the_remainder) {
	// new code to increase accuracy of the least significant digits
		int j = NumFractDigits;
		for (int i = 0; i < 3; i++) {
			if (j < NumDigits) {
				num->Mult(Rn[j]->GetPowMod2());		// scale up for more accuracy test
				range->Mult(Rn[j]->GetPowMod2());
				j++;
			}
		}
	}
//#endif

	ss << ".";
	while(!num->Zero()) {


		range->ena_dplytrc = 0;
		range->DivStd(radix, rem1);		// divide by radix

//		cout << "range div: " << range->Print10() << endl;
//		cout << "rem1: " << rem1->Print10() << endl;
//		wait_key();

		if(range->Zero() || (!range->Compare(radix))) break;

//		cout << "num before div: " << num->Print10() << endl;
		num->ena_dplytrc = 0;
		num->DivStd(range, rem2);
		
//		cout << "num/range: " << num->Print10() << endl;
//		cout << "rem2: " << rem2->Print10() << endl;

		if(num->IsEqual(radix)) {

			if(r == 10) {
				ss << ("9");				// this should support decimal
			}
			else if(r == 16) {
				ss << ("f");				// this should support hexadecimal
			}
			else if(r == 2) {
				ss << ("1");                // this supports binary
			}
	
			num->Assign(rem2);
			num->Add(range);		
		}
		else {
			
//			printf("(%I64d)", num->Decimal64());
			ss << num->Print_NoHdr(r);
			num->Assign(rem2);		
		}

		range->Add(rem1);		// carry remainder forward (add rem) for the range (does this really matter?)
		
//		wait_key();

	}

	int remflag = 0;
// remainder notice print, format as fraction against the remaining range, or as a plus sign depending on state of PRINT_REMAINDER
	if(!rem2->Zero()) {

		if (print_the_remainder)
			ss << "~";	// indicates that remainder digits are printed
		else
			ss << "+";	// indicates that there is a remainder
//#ifdef PRINT_REMAINDER
//
//		ss << "~";		// indicates that remainder digits are printed
//
//#else
//		ss << "+";		// indicates that there is a remainder
//
//#endif
	}


	output.append(ss.str());


	//PPM *num = new PPM(0);	
	//PPM *frac = new PPM(0);
	//PPM *range = new PPM(0);
	//PPM *radix = new PPM(r);
	//PPM *rem1 = new PPM(0);
	//PPM *rem2 = new PPM(0);
	//PPM *temp = new PPM(0);

	delete snum;
	delete rem2;
	delete rem1;
	delete radix;
	delete range;
	delete frac;
	delete num;

	return(output);

}


// this is the PrintPM2(int r) method below, but with debugging commands removed
// Use only with positive values!  For internal print and debugging of divide and scaling routines
string SPMF::PrintAbsPM(int r)
{

	std::stringstream ss;
	std::stringstream ss2;
	string output;

	PPM *frac = new PPM(this, 0);
	PPM *range = new PPM(this, 0);

	this->GetUnitPM(range);		// NOTE:  Get range of reduced power modulus (this)

	PPM *num = new PPM(0);			// get the spmf value as a ppm class object
	num->Assign(this);				// DON't USE AssignPM!
	num->Normalize();

	PPM *radix = new PPM(r);
	PPM *rem1 = new PPM(0);
	PPM *rem2 = new PPM(0);

	SPMF *snum = new SPMF(0);
	snum->Assign(this);					// this was done for sign detection, don't know why, will need to be modified for variable PM
										// THIS LIKELY DOES NOT WORK, NEED TO INVESTIGATE SIGN DETECTION ON PARTIAL MODULUS
	output.append(ss2.str());

	num->ena_dplytrc = 0;
	num->DivStd(range, frac);		// divide with power based modulus
	
	ss << num->Print(r);		// print the whole part

	num->Assign(frac);

	ss << ".";
	while(!num->Zero()) {

		range->ena_dplytrc = 0;
		range->DivStd(radix, rem1);		// divide by power of ten

		if(range->Zero() || (!range->Compare(radix))) break;

		num->ena_dplytrc = 0;
		num->DivStd(range, rem2);

		if(num->IsEqual(radix)) {

			if(r == 10) {
				ss << ("9");				// this should support decimal
			}
			else if(r == 16) {
				ss << ("f");				// this should support hexadecimal
			}
			else if(r == 2) {
				ss << ("1");                // this supports binary
			}

			num->Assign(rem2);
			num->Add(range);		
		}
		else {

			ss << num->Print_NoHdr(r);
			num->Assign(rem2);		
		}

		range->Add(rem1);		// carry remainder forward (add rem) for the range (why add to range?)

	}

	int remflag = 0;

	if(!rem2->Zero()) {
		if (print_the_remainder)
			ss << "~";	// indicates that there is a remainder
		else
			ss << "+";	// indicates that there is a remainder

//#ifdef PRINT_REMAINDER
//
//		ss << "~";		// indicates that there is a remainder
//
//#else
//		ss << "+";		// indicates that there is a remainder
//
//#endif
	}

	output.append(ss.str());

	delete snum;
	delete rem2;
	delete rem1;
	delete radix;
	delete range;
	delete frac;
	delete num;

	return(output);

}


// calculate basic fractional integer inverse
void SPMF::AssignInverse(__int64 x)
{
int arg_sign = 0;

	if(x < 0) {
		arg_sign = 1;
		x = -x;
	}
	else if(x == 0) {
		printf("Divide by error in inverse\r\n");
		wait_key();
		return;
	}

	PPM *ppm1 = new PPM((__int64) x);
	PPM *rem = new PPM(0);

	PPM *frac_range = new PPM(0);

	GetUnit(frac_range);		// get the unit value in PPM format

	frac_range->ena_dplytrc = 0;
	frac_range->DivStd(ppm1, rem);		// 1/x = R/x -> range/x

	this->PPM::Assign(frac_range);			// commentd out to help compile

	if(arg_sign) {		// its negative
		this->PPM::Complement();
	}
		
	this->SignFlag = arg_sign;
	this->SignValid = 1;

	delete frac_range;
	delete rem;
	delete ppm1;
}


// integer multiply with sign extend
void SPMF::Mult(__int64 x)
{

	SPPM::Mult(x);

}

void SPMF::Mult(SPPM *sval)
{

	SPPM::Mult(sval);

}


// This is stadard version that should be used for application development.
// it calls the preferred version of multiply
void SPMF::MultStd(SPMF *arg)
{

	// UNCOMMENT ONLY ONE OF THESE ROUTINES for the standard version used throughout

//	Mult3(arg);			//basic signed multiply without roundup

//	Mult4(arg);			// simulates dual MRC conversion using a complement scheme, with round up

	Mult4b(arg);		// simulates single MRC conversion scheme using a correction constant, with round-up

//	Mult5PM(arg);		// No sign support (for debugging and internal analysis of Mult6 method)

//	Mult6(arg);			// is there is a bug with this method?

}

// Original verison WITH ROUND-UP
// positive values only
void SPMF::Mult(SPMF *pmf)
{

	SPMF *pmftmp = new SPMF(0);
	SPMF *pmftmp2 = new SPMF(0);
	SPMF *rem = new SPMF(0);
	SPMF *unit = new SPMF(0);
//	SPMF *unitsqr = new SPMF(0);

	GetUnit(unit);				// get unit value

//	cout << "this: " << this->Print10() << endl;
//	cout << "arg: " << SPMF->Print10() << endl;

	PPM::Mult(pmf);		// standard ppm multiply

//	cout << "intermediate product: " << this->Print10() << endl;
//	cout << "intermediate raw: " << this->Prints() << endl;

//	unitsqr->Add1(1);			// for testing MAC function, add a unit squared
//	unitsqr->PPM::Mult(unit);
//	this->PPM::Add(unitsqr);
//	this->Add1(1);

//	cout << "this after add one sqr: " << this->Print10() << endl;

//	wait_key();

	MRN *mrn = new MRN(this);
	MRN *mrn2 = new MRN(this);		//get second copy

//	cout << "mrn: " << mrn->Decimal64() << endl;
//	printf("MRN: ");
//	mrn->Print();
//	printf("\r\n");

	mrn->ShiftRight(NumFractDigits);	// shift correction
	
//	cout << "after shift, mrn: " << mrn->Decimal64() << endl;
//	printf("MRN: ");
//	mrn->Print();
//	printf("\r\n");	

	mrn->Convert(pmftmp);		// convert the shifted value (answer)
//	mrn->Convert(this);
	mrn2->Convert(pmftmp2);		// convert the non-shifted value

	

	rem->Assign(pmftmp2);
	pmftmp2->Assign(pmftmp);
	pmftmp2->PPM::Mult(unit);	// adjust the result value for calc of rem

	rem->Sub(pmftmp2);		// calculate the remainder
	rem->Mult(2);			// compare .500 switch point directly to unit value

//	cout << "rem " << rem->PPM::Print10() << endl;
//	cout << "unit: " << unit->PPM::Print10() << endl;	
	
	if(!unit->PPM::Compare(rem)) {		// if unit not greater than rem,
		pmftmp->PPM::Add(1);		// round up one unit
//		printf("Round Up\r\n");
	}
	
	this->Assign(pmftmp);
	
//	cout << "this: " << this->Print10() << endl;

//	wait_key();
	delete mrn2;
	delete mrn;

	delete unit;
	delete rem;
	delete pmftmp2;
	delete pmftmp;

}


// Multiply using seperate fraction and integer words
// for development purposes only, no sign support
void SPMF::Mult2(SPMF *pmf)
{

	PPM *unit = new PPM(0);

	PPM *temp = new PPM(0);
	SPMF *this_rem = new SPMF(0);		// place to store this fraction portion

	PPM *arg = new PPM(0);
	SPMF *arg_rem = new SPMF(0);		// place to store arg fraction portion

	arg->Assign(pmf);

	this->GetUnit(unit);			// get the unit value

	arg->ena_dplytrc = 0;			// using divide to do splice, change !!!!!!
	arg->DivStd(unit, arg_rem);
//	this->Assign(arg);				// commented out to help compile

	cout << "arg: " << arg->Print10() << endl;
	cout << "arg_rem: " << arg_rem->Prints() << endl;

	this->ena_dplytrc = 0;			// using divide to make splice, change !!!!
	this->PPM::DivStd(unit, this_rem);

	temp->Assign(this);		// assign this (integer) to a temp

	PPM::Mult(arg);			// standard multiplication of Integer portion
	
	temp->Mult(arg_rem);
	this->PPM::Add(temp);

	temp->Assign(arg);
	temp->Mult(this_rem);

	this->PPM::Add(temp);

//	arg_rem->Mult2(this_rem);		// THIS IS A RECURSIVE ERROR.  perform fractional multiply on fractional portion

//	this->Add(arg_rem);			// still need to write this routine


	delete unit;
	delete temp;
	delete this_rem;
	delete arg;
	delete arg_rem;

}


// re-wrote the fraction multiply to handle signed values better
// new version uses method of complements, and a compare against an intermediate positive range
// which simulates the claimed hardware invention better
// this vesion has NO ROUNDING  (truncation only)
void SPMF::Mult3(SPMF *arg)
{
int sign;
//	cout << "this: " << this->Print10() << endl;
//	cout << "arg: " << SPMF->Print10() << endl;

	PPM::Mult(arg);		// standard ppm multiply

//	cout << "this after *: " << this->Print10() << endl;

//	wait_key();

// the following code as disabled since it appears incorrect.
// it was replaced by a comparison against the signed PPM range 
/*
	if((sign=this->GetExtSign()) == NEGATIVE) {		// emaulates the comparison against the intermediate
		this->Complement();
	}
*/
	if((sign=this->SPPM::CalcSign()) == NEGATIVE) {		// emaulates a comparison of the intermediate against the signed integer range
		this->Complement();
	}

	MRN *mrn = new MRN(this);

	cout << "mrn: " << mrn->Decimal64() << endl;
	printf("MRN: ");
	mrn->Print();
	printf("\r\n");

	mrn->ShiftRight(NumFractDigits);
	printf("MRN: ");
	mrn->Print();
	printf("\r\n");	

	mrn->Convert(this);
//	cout << "this: " << this->Print10() << endl;

	if(sign == NEGATIVE) {
		this->Complement();
		this->SignFlag = 1;
	}
	else {

		this->SignFlag = 0;
	}

	this->SignValid = 1;

//	wait_key();
	delete mrn;

}

// version to simulate sign extending multipler (hardware) model
// this version converts both intermediate product and its complement;
// if the original intermediate product is less than the complement, the answer is positive, and the original mixed radix value is re-converted
// if the complement is less than the orignal, the result is negative, and the complement is converted back, and the final rounded result is complemented
// this vesion has ROUNDING
void SPMF::Mult4(SPMF *arg)
{
int sign;

	PPM::Mult(arg);		// standard ppm multiply to obtain intermediate product

	PPM *comp = new PPM(0);
	comp->Assign(this);
	comp->Complement();

	PPM *copy = new PPM(0);

	if(this->PPM::Compare(comp)) {
		MRN *mrnc = new MRN(comp);
		copy->Assign(comp);
		sign = NEGATIVE;
		mrnc->ShiftRight(NumFractDigits);
		mrnc->Convert(this);
		delete mrnc;
	}
	else {
		MRN *mrn = new MRN(this);
		copy->Assign(this);
		sign = POSITIVE;
		mrn->ShiftRight(NumFractDigits);
		mrn->Convert(this);
		delete mrn;
	}
		

	PPM *ru_val = new PPM(0);					// calcualte a round-up compare value constant (need to perform only once, but is done redundantly here)
	GetRange(ru_val, NumFractDigits);
	ru_val->ModDiv(2);
	ru_val->ExtendPart2Norm();
	ru_val->Decrement();

	if(copy->ComparePart(ru_val, NumFractDigits)) {   // if the intermediate product fractional value greater than the round-up value, then increment by one
		this->Increment();
	}

	if(sign == NEGATIVE) {
		this->Complement();
		this->SignFlag = 1;
	}
	else {

		this->SignFlag = 0;
	}

	this->SignValid = 1;

	delete ru_val;
	delete copy;
	delete comp;

}

// version to simulate sign extending multipler model recently introduced
// this version implements a correction factor instead using complement logic;
// in this version, the signed fractional value is directly divided by the fractional range using MRC and digit truncation
// if the value is negative, a correction constant is subtracted, constant = Rw
// rounding is applied as in the positive case for all cases except for truncation quantity = 0.50000
void SPMF::Mult4b(SPMF *arg)
{
	int sign;

	PPM::Mult(arg);		// standard ppm multiply to obtain intermediate product

//	PPM *comp = new PPM(0);		// no need to complement in this version
//	comp->Assign(this);
//	comp->Complement();

	sign = this->SPPM::CalcSign();		// emulates a comparison of the intermediate against the signed integer range

	PPM *cor_const = new PPM(1);										// declare variable for correction constant
	for (int i = NumFractDigits; i < NumDigits; i++) {		// calculate correction constant using current value of power modulus
		cor_const->PPM::Mult(this->PPM::Rn[i]->GetPowMod2());
	}

	PPM *copy = new PPM(0);			// temp variable for copy of original intermediate value


	MRN *mrn = new MRN(this);
	copy->Assign(this);				// keep copy of the original for next stages of calculation

	mrn->ShiftRight(NumFractDigits);
	mrn->Convert(this);
	delete mrn;


	PPM *ru_val = new PPM(0);					// calcualte a round-up compare value constant (need to perform only once, but is done redundantly here)
	GetRange(ru_val, NumFractDigits);
	ru_val->ModDiv(2);							// this assumes there is an even fractional range (two's power modulus mapped to fractional range)
	ru_val->ExtendPart2Norm();
//	ru_val->Decrement();						// we decrement to make the comparison include half range (0.50000)

	if (copy->ComparePart(ru_val, NumFractDigits)) {   // if the intermediate product fractional value greater than the round-up value, then increment by one
		this->Increment();
	}
	else if(copy->IsEqualPart(ru_val, NumFractDigits)) {		// special logic for tie breaking case
		if (sign == POSITIVE) {
			this->Increment();
		}
	}

	if (sign == NEGATIVE) {
		this->PPM::Sub(cor_const);		// this is the new method for correcting incorrectly normalized negative values
		this->SignFlag = 1;
	}
	else {

		this->SignFlag = 0;
	}

	this->SignValid = 1;

	delete ru_val;
	delete copy;

}

// Multiplication using a fast power based recombination.
// for test and development only
// NEED TO ADD ROUND UP!
// Positive Values Only
void SPMF::Mult5(SPMF *spmf2)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *pmpower = new PPM(1);
PPM *acc = new PPM(0);
PPM *temp = new PPM(0);
int digval1 = 0;

	ppm1->Assign(this);
	ppm1->Mult(spmf2);

//	cout << "intermediate prod = " << ppm1->Prints() << endl;
//	printf("iprod:");
//	ppm1->Print();
//	printf("\n\n");

//	cout << "NumFractDigits: = " << NumFractDigits << endl;

	int index = 0;
	int zero1;
	do {

		digval1 = ppm1->Rn[index]->Digit;

//		printf("dig1= %3x\r\n", digval1);
//		cout << index << ": pmpower = " << pmpower->Prints() << endl;
//		printf("power:");
//		pmpower->Print();
//		printf("\n");
		
		ppm1->Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

//		printf("dcomp:");
//		ppm1->Print();
//		printf("\r\n");


		if(index >= NumFractDigits) {      // recompose the digit while decomposing the digit

			temp->Assign(digval1);
			temp->Mult(pmpower);
			
//			printf("mmult:");
//			temp->Print();
//			printf("\n");

			acc->Add(temp);
			
//			printf("acc:  ");
//			acc->Print();
//			printf("\r\n");

//			pmpower->Mult(Modulus[index]);
			pmpower->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}

		index += 1;
		zero1 = ppm1->Zero();

//		printf("\n");

	} while(!zero1);

	this->PPM::Assign(acc);
	
	delete ppm1;					// delete temps
	delete pmpower;
	delete acc;
	delete temp;

}

// Multiplication using a fast power based recombination.
// for test and development only
// NEED TO ADD ROUND UP!
// Used for demo and patent writing ups
void SPMF::Mult5_demo(SPMF *spmf2)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *pmpower = new PPM(1);
PPM *acc = new PPM(0);
PPM *temp = new PPM(0);
int digval1 = 0;

PPM *rup = new PPM(510510/2);
MRN *mrn = new MRN(rup);

	printf("mixed radix rup: \n");
	mrn->Print();
	printf("\n");

	ppm1->Assign(this);
	ppm1->Mult(spmf2);

	cout << "intermediate prod = " << ppm1->Prints() << endl;
	printf("iprod:\n");
	ppm1->PrintDemo();
	printf("\n\n");

	int index = 0;
	int zero1;
	do {

		digval1 = ppm1->Rn[index]->Digit;

		printf("dig1= %3d\r\n", digval1);
		cout << index << ": pmpower = " << pmpower->Print10() << endl;
//		printf("power:");
		pmpower->PrintDemo();
		printf("\n");
		
		ppm1->Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

		printf("dcomp:\n");
		ppm1->PrintDemo();
		printf("\r\n");


		if(index >= NumFractDigits) {      // recompose the digit while decomposing the digit

			temp->Assign(digval1);
			temp->Mult(pmpower);
			
//			printf("mmult:\n");
			cout << "mmult: " << temp->Print10() << endl;
			temp->PrintDemo();
			printf("\n");

			acc->Add(temp);
			
//			printf("acc:  \n");
			cout << "acc: " << acc->Print10() << endl;
			acc->PrintDemo();
			printf("\r\n");

//			pmpower->Mult(Modulus[index]);
			pmpower->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}

		index += 1;
		zero1 = ppm1->Zero();

		printf("\n");

	} while(!zero1);

	this->PPM::Assign(acc);


}

// Multiplication using a fast power based MR to Residue recombination.
// this version supports signed values, and generates a sign flag irregardless of signvalid flags of argument
// This version also supportsd rounding
// NEED TO TEST ROUND UP!
void SPMF::Mult6(SPMF *spmf2)
{
PPM *ppm1 = new PPM(0);		// the first accumulator
PPM *ppm2 = new PPM(0);
PPM *pmpower = new PPM(1);
PPM *acc1 = new PPM(0);
PPM *acc2 = new PPM(0);
PPM *temp = new PPM(0);
int digval1, digval2;
int compare = EQUAL;			// start compare with equal
int ru_digit;
int ru_flag1 = EQUAL;
int ru_flag2 = EQUAL;
int ru_done1 = 0;
int ru_done2 = 0;


	ppm1->Assign(this);		// first accumulator
	ppm1->Mult(spmf2);

	ppm2->Assign(ppm1);		// second accumulator
	ppm2->Complement();

//	cout << "intermediate prod = " << ppm1->Prints() << endl;
//	printf("iprod:");
//	ppm1->Print();
//	printf("\n\n");

	PPM *ru_val = new PPM(0);					// calcualte a round-up compare value constant (need to perform only once at class instantiation, but is done redundantly here)
	GetRange(ru_val, NumFractDigits);
	ru_val->ModDiv(2);
	ru_val->ExtendPart2Norm();

//	cout << "NumFractDigits: = " << NumFractDigits << endl;

	int index = 0;
	int zero1, zero2;
	do {

		digval1 = ppm1->Rn[index]->Digit;
		digval2 = ppm2->Rn[index]->Digit;

		if(digval1 > digval2) {      // digit1 is greater than digit2
			compare = GREATER;
		}
		else if(digval1 < digval2) {   // digit1 is less than digit2, else equal (no change)
			compare = LESSER;
		}



//		printf("dig1= %3x\r\n", digval1);
//		cout << index << ": pmpower = " << pmpower->Prints() << endl;
//		printf("power:");
//		pmpower->Print();
//		printf("\n");
		
		ppm1->Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

		ppm2->Sub(digval2);
		ppm2->ModDiv(Rn[index]->GetPowMod2());
		ppm2->Rn[index]->SkipDigit();

//		printf("dcomp:");
//		ppm1->Print();
//		printf("\r\n");

		ru_digit = ru_val->Rn[index]->Digit;        // we could have stored the rounding constant in MRN format, and avoid the conversion here
		ru_val->Sub(ru_digit);
		ru_val->ModDiv(Rn[index]->GetPowMod2());
		ru_val->Rn[index]->SkipDigit();


		if(index >= NumFractDigits) {      // recompose the digit while decomposing the digit

			temp->Assign(digval1);
			temp->Mult(pmpower);
			acc1->Add(temp);

//			printf("mmult:");
//			temp->Print();
//			printf("\n");

//			printf("acc:  ");
//			acc->Print();
//			printf("\r\n");

			temp->Assign(digval2);
			temp->Mult(pmpower);
			acc2->Add(temp);

//			pmpower->Mult(Modulus[index]);
			pmpower->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}

		index += 1;
		zero1 = ppm1->Zero();
		zero2 = ppm2->Zero();


		if(index < NumFractDigits) {      // calculate round-up value with digit values against fract_range/2 or early termination
			
			if(digval1 > ru_digit) {      // digit1 is greater than ru_digit
				ru_flag1 = GREATER;
			}
			else if(digval1 < ru_digit) {   // digit1 is less than ru_digit, else equal (no change)
				ru_flag1 = LESSER;
			}

			if(digval2 > ru_digit) {      // digit2 is greater than ru_digit
				ru_flag2 = GREATER;
			}
			else if(digval2 < ru_digit) {   // digit2 is less than ru_digit, else equal (no change)
				ru_flag2 = LESSER;
			}

			if(zero1) {
				ru_done1 = 1;
//				printf("ru_done1\n");
			}
			
			if(zero2) {
				ru_done2 = 1;
//				printf("ru_done2\n");
			}

		}

//		printf("\n");

	} while(!zero1 && !zero2);


	if((compare == GREATER) || (!zero1 && zero2)) {
		this->PPM::Assign(acc2);

		if((ru_flag2 == GREATER) && !ru_done2) {
			this->Increment();
//			printf("-round up\n");
		}

		this->Complement();
		this->SignFlag = NEGATIVE;
	}
	else {
		this->PPM::Assign(acc1);

		if((ru_flag1 == GREATER) && !ru_done1) {
			this->Increment();
//			printf("+round up\n");
		}

		this->SignFlag = POSITIVE;
	}

	this->SignValid = 1;
	
	delete ru_val;           // delete all allocated variables
	delete ppm1;	
	delete ppm2;
	delete pmpower;
	delete acc1;
	delete acc2;
	delete temp;

}

// routine to seperate the fractional portion from the whole portion
// fractional portion stored in frac, and whole portion stored in whole upon exit
void SPMF::FracSep(SPMF *frac, PPM *whole)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *pmpower_frac = new PPM(1);
PPM *pmpower_int = new PPM(1);
PPM *acc = new PPM(0);		// temp accumulator for fractional part
PPM *temp = new PPM(0);		
int digval1 = 0;
int index = 0;


	ppm1->Assign(this);		// do MR reduction using the PPM type

	frac->Assign((__int64)(0));		// clear the return arguments
	whole->Assign(0);

//	cout << "starting value = " << ppm1->Prints() << endl;
//	printf("dec:\n");
//	this->Print10();
//	printf("\n\n");


	do {

		digval1 = ppm1->Rn[index]->Digit;

//		printf("dig1= %3d\r\n", digval1);

		
		ppm1->Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

//		printf("dcomp:\n");
//		ppm1->PrintDemo();
//		printf("\r\n");


		if(index < NumFractDigits) {      // recompose the digit while decomposing the digit

//			cout << index << ": pmpower_frac = " << pmpower_frac->Print10() << endl;
//			printf("power:");
//			pmpower_frac->PrintDemo();
//			printf("\n");

			temp->Assign(digval1);
			temp->Mult(pmpower_frac);
			
//			printf("mmult:\n");
//			cout << "mmult: " << temp->Print10() << endl;
//			temp->PrintDemo();
//			printf("\n");

			frac->PPM::Add(temp);
			
//			printf("frac:  \n");
//			cout << "frac: " << frac->Print10() << endl;
//			frac->PrintDemo();
//			printf("\r\n");

			pmpower_frac->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}
		else {

//			cout << index << ": pmpower_int = " << pmpower_int->Print10() << endl;
//			printf("power:");
//			pmpower_int->PrintDemo();
//			printf("\n");

			temp->Assign(digval1);
			temp->Mult(pmpower_int);
			
//			printf("mmult:\n");
//			cout << "mmult: " << temp->Print10() << endl;
//			temp->PrintDemo();
//			printf("\n");

			whole->Add(temp);
			
//			printf("whole:  \n");
//			cout << "whole: " << acc->Print10() << endl;
//			whole->PrintDemo();
//			printf("\r\n");

			pmpower_int->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}

		index += 1;
		
//		printf("\n");

	}  while (!ppm1->Zero());
	

}

// routine to multiply two fractional portions only (no whole portion on either operand)
// used to verify new fractional multiply routine
void SPMF::Mult_frac(SPMF *frac1, SPMF *frac2) 
{
SPMF *full_mult = new SPMF(0);
SPMF *part_mult = new SPMF(0);
PPM *ppm1 = new PPM(0);
PPM *acc = new PPM(0);
PPM *pmpower = new PPM(1);
PPM *temp = new PPM(0);
int zero = 0;
int index = 0;
int digval1, i;
	
SPMF *unit = new SPMF(0);
PPM *ppm_unit = new PPM(0);
PPM *ppm_unit_inv = new PPM(1);
	

	unit->GetUnit(ppm_unit);
	ppm_unit_inv->Assign(ppm_unit);

	full_mult->Assign(frac1);			// process a full word multiply of the two
	full_mult->PPM::Mult(frac2);

	part_mult->Assign(full_mult);		// assign the same product to the part mult cariable

	ppm1->Assign(part_mult);

	cout << "fract parts multiplied: " << ppm1->Print10() << endl;
	ppm1->PrintDemo();


	do {                // reduce the part mult variable

		digval1 = ppm1->Rn[index]->Digit;
//		printf("dig1= %3d\r\n", digval1);

		
		ppm1->Sub(digval1);
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

		temp->Assign(digval1);
		temp->Mult(pmpower);

		acc->Add(temp);

		pmpower->Mult(this->Rn[index]->GetPowMod2());	// advance the power term

		index += 1;


	} while(!zero && (index < NumFractDigits));

	cout << "fract parts extended (acc): " << acc->Print10() << endl;
	acc->PrintDemo();

	full_mult->PPM::Sub(acc);

	cout << "full_mult - acc: " << full_mult->PPM::Print10() << endl;
	full_mult->PPM::PrintDemo();


	// let's try to calculate a multiplicative inverse of the fractional range
	// first, skip all the fractional digits
	ppm_unit_inv->Assign(1);
	for(i=0; i<NumFractDigits; i++) {
		ppm_unit_inv->Rn[i]->Skip = 1;
	}
	
	for(i=0; i<NumFractDigits; i++) {
		digval1 = this->Rn[i]->GetPowMod2();		// get the factor, and calc an inverse
		ppm_unit_inv->ModDiv(digval1);
//		printf("index: %d\n", i);
//		ppm_unit_inv->PrintDemo();

	}

	cout << "ppm_unit: " << ppm_unit->Print10() << endl;
	ppm_unit->PrintDemo();

	cout << "ppm_unit_inv: " << endl;
	ppm_unit_inv->PrintDemo();


	full_mult->PPM::Mult(ppm_unit_inv);		// multiply by multiplicative inverse

	index = NumFractDigits;		// start base extension at the valid digit
	this->Assign((__int64)0);
	pmpower->Assign(1);
	do {

		digval1 = full_mult->Rn[index]->Digit;
//		printf("dig1= %3d\r\n", digval1);

		
		full_mult->PPM::Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		full_mult->ModDiv(Rn[index]->GetPowMod2());
		full_mult->Rn[index]->SkipDigit();

		temp->Assign(digval1);
		temp->Mult(pmpower);

		this->PPM::Add(temp);

		pmpower->Mult(this->Rn[index]->GetPowMod2());	// advance the power term

		index += 1;

	} while(!full_mult->Zero());

	cout << "this: " << this->Print(10) << endl;
	this->PrintDemo();

	wait_key();
}

// Multiplication using a fast power based recombination.
// for test and development only
// NEED TO ADD ROUND UP!
// THIS ASSUMES THAT BOTH ARGS ARE SAME FORMAT
void SPMF::Mult5PM(SPMF *spmf2)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *pmpower = new PPM(1);
PPM *acc = new PPM(0);
PPM *temp = new PPM(0);
int digval1 = 0;


	ppm1->AssignPM(0, this);
	pmpower->AssignPM(1, this);
	acc->AssignPM(0, this);
	temp->AssignPM(0, this);

	ppm1->AssignPM(this);
//	ppm1->Assign(this);
	ppm1->Mult(spmf2);

//	cout << "intermediate prod = " << ppm1->Prints() << endl;
//	printf("iprod:");
//	ppm1->Print();
//	printf("\n\n");

	int index = 0;
	int zero1;
	do {

		digval1 = ppm1->Rn[index]->Digit;

//		printf("dig1= %3x\r\n", digval1);
//		printf("%3x ", digval1);
//		cout << index << ": pmpower = " << pmpower->Prints() << endl;
//		printf("power:");
//		pmpower->Print();
//		printf("\n");
		
		ppm1->Sub(digval1);
//		ppm1->ModDiv(Rn[index]->GetPowMod());
		ppm1->ModDiv(Rn[index]->GetPowMod2());
		ppm1->Rn[index]->SkipDigit();

//		printf("dcomp:");
//		ppm1->Print();
//		printf("\r\n");


		if(index >= NumFractDigits) {      // recompose the digit while decomposing the digit

			temp->Assign(digval1);
			temp->Mult(pmpower);
			
//			printf("mmult:");
//			temp->Print();
//			printf("\n");

			acc->Add(temp);
			
//			printf("acc:  ");
//			acc->Print();
//			printf("\r\n");

//			pmpower->Mult(Modulus[index]);
			pmpower->Mult(this->Rn[index]->GetPowMod2());		// get current power modulus

		}

		index += 1;
		zero1 = ppm1->Zero();

//		printf("\n");

	} while(!zero1);

	this->PPM::Assign(acc);

//	printf("\n");


}

// this normalization routine is sort of a trick, but allows us to use any of the multiply methods called by MultStd
// the method called by MultStd needs to be a sign extending version, so be careful
// when we multiply by ump, we are not changing the intermediate value, since the PPM::Mult() method called
// by a standard fractional multiply method simply leaves the intermediate method unchanged.
void SPMF::I2N_Convert()
{
SPMF *ump = new SPMF(1);

	this->MultStd(ump);

	delete ump;
}


// sum of products test demo
void SPMF::ProdSum(SPMF *pmf1, SPMF *pmf2, SPMF *pmf3)
{

//	cout << "this: " << this->Print10() << endl;
//	cout << "arg: " << SPMF->Print10() << endl;

	PPM::Mult(pmf1);		// standard ppm multiply

//	cout << "intermediate product A: " << this->Print10() << endl;
//	cout << "intermediate raw A:" << this->Prints() <<endl;

	pmf2->PPM::Mult(pmf3);
//	cout << "intermediate product B: " << pmf2->Print10() << endl;
//	cout << "intermediate raw A:" << pmf2->Prints() <<endl;
	
	this->Add(pmf2);
//	cout << "intermediate sum: " << this->Print10() << endl;
//	cout << "intermediate sum: " << this->Prints() << endl;

//	wait_key();

	this->I2N_Convert();			// convert the intermediate result

}


// Fractional Multiply Accumulate function
// this routine has the benefit of sign extending the sequence of fractional multiplication followed by addition
void SPMF::MAC(SPMF *pmf, SPMF *acc)
{
	SPMF *unit = new SPMF(0);
	SPMF *acc_copy = new SPMF(0);
	
	acc_copy->Assign(acc);

	GetUnitPM(unit);				// get unit value
	acc_copy->PPM::Mult(unit);

	PPM::Mult(pmf);		// standard ppm multiply
	
	this->PPM::Add(acc_copy);

	this->I2N_Convert();

	delete acc_copy;
	delete unit;

}


// divide fractions, requires an integer multiply and an integer divide
// performs a round up if the remainder is greater than half the divisor
void SPMF::Div(SPMF *divisor)
{
int sign;

	this->ena_dplytrc = 0;

	SPMF *div = new SPMF(0);		// copy divisor so as not to alter it
	div->AssignPM(divisor);			// we use div in the goldshmidt routine, need the flexibility of partial power modulus

	PPM *rem = new PPM(0);
	PPM *unit = new PPM(0);
	GetUnit(unit);

	if(this->CalcSign() == div->CalcSign()) {
		sign = POSITIVE;
	}
	else {
		sign = NEGATIVE;
	}

	if(this->CalcSign() == NEGATIVE) {    // must complement dividend if negative
		this->Complement();
//		cout << "this: " << this->PrintPM(10) << endl;
	}

	if(div->CalcSign() == NEGATIVE) {     // must complement divisor if negative
		div->Complement();
//		cout << "arg: " << pmf->PrintPM(10) << endl;
	}


	PPM::Mult(unit);			// multiply dividend by fractional range
	PPM::DivStd(div, rem);		// then perform the division by divisor

								// easy method to perform round-up ...
	rem->Mult(2);				// double the remainder for correct range of comparison


	if(rem->Compare(div)) {
		PPM::Add(1);			// round up one click
//		printf("Div rounded up\r\n");
	}

	if(sign == NEGATIVE) {
		if(!this->Zero()) {
			this->Complement();
			this->SignFlag = NEGATIVE;
		}
		else {
			this->SignFlag = POSITIVE;
		}
	}
	else {
		this->SignFlag = POSITIVE;
	}

	this->SignValid = 1;

//	wait_key();

	delete unit;
	delete rem;
	delete div;
}

// divide fractions, requires an integer multiply and an integer divide
// this version supports partial power modulus
void SPMF::DivPM(SPMF *divisor)
{
int sign;

	this->ena_dplytrc = 0;

	SPMF *div = new SPMF(0);		// copy divisor so as not to alter it
	div->Assign(divisor);

	PPM *rem = new PPM(0);
	PPM *unit = new PPM(0);
	GetUnitPM(unit);


	if(this->CalcSign() == div->CalcSign()) {
		sign = POSITIVE;
	}
	else {
		sign = NEGATIVE;
	}

	if(this->CalcSign() == NEGATIVE) {
		this->Complement();
//		cout << "this: " << this->PrintPM(10) << endl;
	}

	if(div->CalcSign() == NEGATIVE) {
		div->Complement();
//		cout << "arg: " << pmf->PrintPM(10) << endl;
	}

//	cout << "this:    " << this->PPM::Print10() << endl;
//	cout << "unit:    " << unit->PPM::Print10() << endl;

	PPM::Mult(unit);			// multiply by fractional range
	PPM::DivStd(div, rem);	// then perform the integer division

								// experimental .... perform round-up?
	rem->Mult(2);				// double remainder for correct range of comparison

//	cout << "rem:     " << rem->PPM::Print10() << endl;
//	cout << "divisor: " << div->PPM::Print10() << endl;	

	if(rem->Compare(div)) {
		PPM::Add(1);		// round up one click
//		printf("Div rounded up\r\n");
	}

	if(sign == NEGATIVE) {
		if(!this->Zero()) {
			this->Complement();
			this->SignFlag = NEGATIVE;
		}
		else {
			this->SignFlag = POSITIVE;
		}
	}
	else {
		this->SignFlag = POSITIVE;
	}

	this->SignValid = 1;

//	wait_key();
	delete div;
	delete unit;
	delete rem;

}

// get the inverse of the SPMF value using integer division method
// DivPM handles negative values, so this routine is easy
void SPMF::Inverse(void)
{

	SPMF *unit = new SPMF(0);
	GetUnitPM(unit);

	unit->DivPM(this);
	
	this->Assign(unit);

	delete unit;

}

// Assign this = round(Rf / denom), interpreted as the SPMF value 1/denom.
//
// This is the PPM-denominator version of AssignRatio(1, y), avoiding
// __int64 limits for large factorial denominators.
//
// Positive denominator only.
void SPMF::AssignInversePPM(PPM* denom)
{
	PPM* range = new PPM(0);
	PPM* rem = new PPM(0);

	if (denom->Zero()) {
		printf("ERROR: AssignInversePPM divide by zero\n");
		wait_key();
		this->Assign(0LL);
		delete rem;
		delete range;
		return;
	}

	// range = Rf
	this->GetUnitPM(range);

	// this = floor(Rf / denom), rem = Rf % denom
	this->PPM::Assign(range);
	this->ena_dplytrc = 0;
	this->PPM::DivStd(denom, rem);

	// Round-to-nearest using the same convention as DivPM:
	// if 2*rem > denom, increment by one fractional click.
	rem->Mult(2);

	if (rem->Compare(denom)) {
		this->PPM::Add(1);
	}

	this->SignFlag = POSITIVE;
	this->SignValid = SIGN_VALID;

	delete rem;
	delete range;
}

/*

	f = 2.0 - d;

	while((i < 100) && (f != last_f)) {

		last_f = f;

		n *= f;
		d *= f;

		f = 2.0 - d;

		printf("n = %5.16f, d = %5.16f, f = %5.16f\r\n", n, d, f);

		i += 1;

	}

*/

// prototype fractional division (by multiplication) routine, which divides "this" by arg 
void SPMF::GoldDiv(SPMF *arg)
{
int width;
int sign = POSITIVE;
SPMF *divisor = new SPMF(0);

	divisor->Assign(arg);

	if(divisor->CalcSign() == NEGATIVE) {
		divisor->Complement();
		divisor->SignFlag = POSITIVE;
		if(this->CalcSign() == NEGATIVE) {
			sign = POSITIVE;
			this->Complement();
		}
		else {
			sign = NEGATIVE;
		}
	}
	else if(this->CalcSign() == NEGATIVE) {
		this->Complement();
		sign = NEGATIVE;
	}

	this->scalePM(divisor, this, width);			// first, scale the divisor (arg) to: 1.0 > divisor >= 0.5, and also scale "this" by same amoount
//	this->GoldDivAbsDebug(divisor);	
	this->GoldDivAbs(divisor);	
	this->NormalFract();

	if(sign == NEGATIVE) {
		if(!this->Zero()) {
			this->SignFlag = NEGATIVE;
			this->Complement();
		}
		else {
			this->SignFlag = POSITIVE;
		}
	}
	else {
		this->SignFlag = POSITIVE;
	}
	this->SignValid = 1;

	delete divisor;
}

// prototype Goldschmidt (absolute) division assumes the divisor is scaled, 0 < D < 1.0
// sign processing has been eliminated out of this primitive routine
// therefore, the routine expects positive operands only (i.e., absolute)
// there is still a side effect going on here regarding maintaining proper PM format, but still works (has to do with divisor)
void SPMF::GoldDivAbs(SPMF *divisor)
{
SPMF *f = new SPMF(0);
SPMF *last_f = new SPMF(0);
SPMF *last_d = new SPMF(0);
int i = 0;
int sign = POSITIVE;


	f->PPM::AssignPM(0, this);			// assign f with the same format as this
	f->NumFractDigits = this->NumFractDigits;		// need an assign method to do this also
	last_f->AssignPM(0, this);
	last_f->NumFractDigits = this->NumFractDigits;
	last_d->AssignPM(0, this);
	last_d->NumFractDigits = this->NumFractDigits;

	SPMF *div = new SPMF(0);			// don't really need to protect divisor (by copy), since the calling routine does this
	div->Assign(divisor);
	div->NumFractDigits = divisor->NumFractDigits;

	f->Add1(2);						// helper funciton now modified to work with power based modulus (needs getunitPM())

	f->Sub(div);	// f = 2.0 - d

	while(!f->IsEqual(last_f)) {		// if divisor is same from last loop, basic termination for now

		last_f->Assign(f);			// store a copy for the check
		last_d->Assign(this);
		
		this->Mult5PM(f);	

		div->Mult5PM(f);

		f->PPM::AssignPM(0, this);		// clear the variable f
		f->Add1(2);
		f->Sub(div);			// f = 2.0 - d

		i += 1;

	}

	this->Assign(last_d);			// this for testing one early iteration candidate

	delete div;
	delete last_f;
	delete last_d;
	delete f;
}

// trying to understand the bad side effect in PM processing, Sub method fails when the divisor is in PM format
void SPMF::GoldDivAbsDebug(SPMF *divisor)
{
SPMF *f = new SPMF(0);
SPMF *last_f = new SPMF(0);
SPMF *last_d = new SPMF(0);
int i = 0;
int sign = POSITIVE;


	f->PPM::AssignPM(0, this);			// assign f with the same format as this
	f->NumFractDigits = this->NumFractDigits;		// need an assign method to do this also
	last_f->AssignPM(0, this);
	last_f->NumFractDigits = this->NumFractDigits;
	last_d->AssignPM(0, this);
	last_d->NumFractDigits = this->NumFractDigits;

	SPMF *div = new SPMF(0);
//	div->AssignPM(0, divisor);
	div->Assign(divisor);
	div->NumFractDigits = divisor->NumFractDigits;
	div->PrintDemo();
//	divisor->PrintDemo();

//	cout << div->NumFractDigits << "   " << divisor->NumFractDigits << endl;

//	wait_key();

	f->Add1(2);						// now modified to work with pwoer based modulus (needs getunitPM())

	f->Sub(div);	// f = 2.0 - d


	while(!f->IsEqual(last_f)) {		// if divisor is same from last loop, basic termination for now

		last_f->Assign(f);			// store a copy for the check
		last_d->Assign(this);
		
//		cout << "f = " << f->PrintPM2(10) << "  " << divisor->PrintPM2(10) << endl;
//		divisor->PrintDemo();

		this->Mult5PM(f);	

		div->Mult5PM(f);

		f->PPM::AssignPM(0, this);		// clear the variable f
		f->Add1(2);
		f->Sub(div);			// f = 2.0 - d

		cout << "n = " << this->PrintAbsPM(10) << endl;
		cout << " d = " << div->PrintAbsPM(10) << endl;
//		wait_key();

		i += 1;

	}

	this->Assign(last_d);			// this for testing one early iteration candidate

	delete div;
	delete last_f;
	delete last_d;
	delete f;
}

// same as above but for continued testing
// now adding some testing and verification of the new Goldschmidt formula variations
void SPMF::GoldDivAbs2(SPMF *divisor)
{
SPMF *f = new SPMF(0);
SPMF *last_f = new SPMF(0);
SPMF *last_d = new SPMF(0);
SPMF *div2 = new SPMF(0);			// experimental value for new goldschmidt method using iterative formula: divisor = 1.0 - (1-f)(1-f)
SPMF *unity = new SPMF(0);
SPMF *temp = new SPMF(0);
int i = 0;
int sign = POSITIVE;


//	cout << "n = " << this->PrintPM(10) << " d = " << divisor->PrintPM(10) << endl;


	f->PPM::AssignPM(0, this);			// assign f with the same format as this
	f->NumFractDigits = this->NumFractDigits;		// need an assign method to do this also
	last_f->AssignPM(0, this);
	last_f->NumFractDigits = this->NumFractDigits;
	last_d->AssignPM(0, this);
	last_d->NumFractDigits = this->NumFractDigits;

	printf("Num fract digs: %d\n", f->NumFractDigits);

	f->Add1(2);						// now modified to work with pwoer based modulus (needs getunitPM())

	f->Sub(divisor);	// f = 2.0 - d

	cout << "divisor = " << divisor->PrintAbsPM(10) << endl;

	unity->AssignPM(0, this);		// get the format
	unity->NumFractDigits = this->NumFractDigits;
	unity->Add1(1);			// make the value one
	div2->AssignPM(0, this);
	div2->NumFractDigits = this->NumFractDigits;
	temp->AssignPM(0, this);
	temp->NumFractDigits = this->NumFractDigits;

	// (in future, do termination compare on intermediate product)
	while(!f->IsEqual(last_f)) {		// if divisor is same from last loop

		last_f->Assign(f);			// store a copy for the check
		last_d->Assign(this);
		
		this->Mult5PM(f);	

//		cout << "f = " << f->PrintPM(10) << "  " << divisor->PrintPM(10) << endl;
//		divisor->PrintDemo();
		div2->Assign(unity);			// new technique for processing divisor
		div2->PPM::Sub(divisor);
		div2->Mult5PM(div2);
		temp->Assign(unity);
		temp->PPM::Sub(div2);


		divisor->Mult5PM(f);			// normal method for processing divisor


//		cout << "result= " << divisor->PrintPM(10) << endl;
		divisor->PrintDemo();
		temp->PrintDemo();
//		printf("\n");

		cout << "n = " << this->PrintAbsPM(10) << " d = " << divisor->PrintAbsPM(10) << endl;
//		cout << "div2 = " << temp->PrintPM2(10) << endl;
//		this->PrintDemo();
//		divisor->PrintDemo();

		f->PPM::AssignPM(0, this);		// clear the variable f
		f->Add1(2);
		f->Sub(divisor);			// f = 2.0 - d

//		cout << "n = " << this->PrintPM(10) << " d = " << divisor->PrintPM(10) << endl;

		wait_key();

		i += 1;

	}

//	this->Assign(last_d);
/*
	if(sign == NEGATIVE) {
		this->SignFlag = NEGATIVE;
		this->Complement();
	}
	else {
		this->SignFlag = POSITIVE;
	}
*/

	this->SignValid = 1;


	delete last_f;
	delete last_d;
	delete f;

}

// INTEGER VERSION
// Experimental routine to develop integer divide using Goldschmidt and simple scaling of integers to simple fractions
void SPMF::GoldDiv(PPM *n_arg, PPM *d_arg, PPM *rem)
{
int width;
SPMF *spmf1 = new SPMF(0);
SPMF *spmf2 = new SPMF(0);
SPMF *spmf3 = new SPMF(0);
SPMF *spmf_rem = new SPMF(0);
PPM *ppm_tmp = new PPM(0);
PPM *n_result = new PPM(0);
PPM *n = new PPM(0);
PPM *d = new PPM(0);


	n->Assign(n_arg);
	d->Assign(d_arg);

	spmf3->ScaleInt(n, d, spmf1, spmf2, width);

	printf("scaled num:\n");
	cout << spmf1->Print(10) << endl;
	spmf1->PPM::PrintDemo();
	printf("fraction point: %d\n", spmf1->NumFractDigits);
	printf("\n");

	printf("scaled divisor:\n");
	cout << spmf2->Print(10) << endl;
	spmf2->PPM::PrintDemo();
	printf("fraction point: %d\n", spmf2->NumFractDigits);
	printf("\n");

	spmf1->GoldDivAbsDebug(spmf2);			// using std golddiv for scaled integer values


	printf("smpf1 after division\n");
	spmf1->PrintDemo();
	printf("fraction point: %d\n", spmf1->NumFractDigits);
	printf("%3.15Lf\n", spmf1->PrintFPM());
	cout << spmf1->Print(10) << endl;
	printf("\n");
	
	spmf1->FracSep(spmf_rem, n_result);		// separate the fraction from the whole  NOT ENOOUGH, NEED ITERATION STEP
	cout << "whole: " << n_result->Print10() << endl;


	ppm_tmp->Assign(n_result);


							// would need this this if using d directly
//	d->Extend2();			// base extend, and normalize the power digits

	ppm_tmp->Mult(d_arg);

	cout << "calc dividend: " << ppm_tmp->Print10() << endl;

	rem->Assign(n);
	rem->Sub(ppm_tmp);

	cout << "rem: " << rem->Print10() << endl;

	delete spmf1;
	delete spmf2;
	delete spmf3;
	delete spmf_rem;
	delete ppm_tmp;
	delete n_result;
	delete n;
	delete d;

}


// Same as PPM::Sqrt(), except negative numbers return error code
// Uses Newtons method for performing Square Root
// always returns a positive result
int SPMF::Sqrt(void)
{
SPMF *last_x = new SPMF(0);
SPMF *y = new SPMF(0);
SPMF *x = new SPMF(0);
SPMF *two = new SPMF(0);


	if(this->CalcSign() == NEGATIVE) {
		delete two;
		delete x;
		delete y;
		delete last_x;
		return(SQRT_OF_NEGATIVE);
	}

	last_x->Assign(this);
	x->Add1(2);

	two->Add1(2);

	while(1) {

		y->Assign(this);
		y->Div(x);
		y->Add(x);
		y->Div(two);			// this could be done a lot more efficiently using ModDiv and extend()

		if(last_x->IsEqual(y)) break;

		last_x->Assign(x);
		x->Assign(y);

	}

	this->Assign(y);

	delete two;
	delete x;
	delete y;
	delete last_x;

	return(0);		// return no error code if parameter is positive

}

// returns a maximum value in the SPMF argument for which std. multiply will work
int SPMF::GetMaxRootFraction(SPMF *max_val)
{
	
	GetMaxAbsFraction(max_val);
	max_val->Sqrt();

	return(0);
}

int SPMF::GetMaxAbsFraction(SPMF *max_val)
{
PPM *range = new PPM(0);
	
	this->GetAbsRange(range);		// get maximum value of the underlying residue system

	max_val->PPM::Assign(range);

	max_val->SignFlag = POSITIVE;		// square root of range ensures the value is positive by definition
	max_val->SignValid = SIGN_VALID;

	delete range;

	return(0);
}

int SPMF::GetUmp(SPMF *spmf)
{
PPM *ump = new PPM(1);

	spmf->PPM::Assign(ump);

	delete ump;

	return(0);

}

// returns the current fraction point position
int SPMF::GetCurFractPos(void)
{

	return(NumFractDigits);

}

// returns the current normal fraction point position
int SPMF::GetNormFractPos(void)
{

	return(NormalFractDigits);

}

// return a non-normal fractional value to an equivalent normal fractional format
// to be used after GoldDiv if needed
void SPMF::NormalFract(void)
{
int i, dig_dif, pwr_dif, digval;
unsigned int twos_pwr;
PPM *ppm1 = new PPM(0);

//	cout << "current NumFractDigs: " << this->NumFractDigits << "  " << "Normal NumFractDigs: " << this->NormalFractDigits << endl;

	dig_dif = this->NumFractDigits - this->NormalFractDigits;	// should always be zero or positive if scaling to NumFractDigits is performed as a minimum

//	printf("Fractional point position dif: %d\n", dig_dif);

//	wait_key();

//	cout << "current twos PowerValid: " << this->Rn[Mod2_index]->PowerValid << endl; 
//	cout << "current twos power modulus: " << this->Rn[Mod2_index]->Power << "  normal two's power:" << this->Rn[Mod2_index]->NormalPower << endl;

	pwr_dif = this->Rn[Mod2_index]->NormalPower - this->Rn[Mod2_index]->PowerValid;		// get the power difference
//	printf("Two's power dif: %d\n", pwr_dif);
//	wait_key();

	this->Normalize();		// increase the two's modulus back to its normal setting
//	cout << " this after normalize: " << this->PrintPM2(10) << endl;

	twos_pwr = power(2, pwr_dif);		// get the power of the 
	this->PPM::Mult((int) twos_pwr);			// scale value to compensate for normal two's power

	for(i=this->NormalFractDigits; i<this->NormalFractDigits+dig_dif; i++) {     // for each digit position needing alignment ...

		digval = this->Rn[i]->Digit;

//		printf("digval: %d\r\n", digval);

		this->PPM::Sub(digval);
		this->ModDiv(Rn[i]->GetFullPowMod());
		this->Rn[i]->SkipDigit();

//		cout << this->Prints() << endl;
//		wait_key();

	}

//	printf("base extending\n");
//	cout << " this: " << this->Prints() << endl;
//	this->PrintDemoPM(10);
	this->ExtendNorm();

//	cout << this->Prints() << endl;
//	wait_key();


	this->NumFractDigits = this->NormalFractDigits;
//	printf("restoring numfractdigits: %d\n", this->NumFractDigits);

}

// return a non-normal fractional value to an equivalent normal fractional format
// to be used after GoldDiv if needed
void SPMF::NormalFract1(void)
{
int i, dig_dif, pwr_dif, digval;
unsigned int twos_pwr;
PPM *ppm1 = new PPM(0);

	cout << "current NumFractDigs: " << this->NumFractDigits << "  " << "Normal NumFractDigs: " << this->NormalFractDigits << endl;

	dig_dif = this->NumFractDigits - this->NormalFractDigits;	// should always be zero or positive if scaling to NumFractDigits is performed as a minimum

	printf("Fractional point position dif: %d\n", dig_dif);

	wait_key();

	cout << "current twos PowerValid: " << this->Rn[Mod2_index]->PowerValid << endl; 
	cout << "current twos power modulus: " << this->Rn[Mod2_index]->Power << "  normal two's power:" << this->Rn[Mod2_index]->NormalPower << endl;

	pwr_dif = this->Rn[Mod2_index]->NormalPower - this->Rn[Mod2_index]->PowerValid;		// get the power difference
	printf("Two's power dif: %d\n", pwr_dif);
	wait_key();

	this->Normalize();		// increase the two's modulus back to its normal setting

	twos_pwr = power(2, pwr_dif);		// get the power of the 
	this->PPM::Mult((int) twos_pwr);			// scale value to compensate for normal two's power

	for(i=this->NormalFractDigits; i<this->NormalFractDigits+dig_dif; i++) {     // for each digit position needing alignment ...

		digval = this->Rn[i]->Digit;

		printf("digval: %d\r\n", digval);

		this->PPM::Sub(digval);
		this->ModDiv(Rn[i]->GetFullPowMod());
		this->Rn[i]->SkipDigit();

		cout << this->Prints() << endl;
		wait_key();

	}

	printf("base extending\n");
	this->ExtendNorm();

	cout << this->Prints() << endl;
	wait_key();


	this->NumFractDigits = this->NormalFractDigits;
	printf("restoring numfractdigits: %d\n", this->NumFractDigits);

}



// this is the container for the best version of the scaling routine
// this routine scales pmf to a value between 0.5 and 1.0, and then scales 
// the second argument pmf2 to the same degree and format
int SPMF::scalePM(SPMF *pmf, SPMF *pmf2, int &width)
{
int index;

//	scalePM4(pmf, pmf2, width);		// this version supports two's modulus in first position only
	index = scalePM5(pmf, pmf2, width);		// this version should support two's modulus in any position
//	index = scalePM6(pmf, pmf2, width);		// this version includes debug statemetns for analysis of the scaling routine

	return(index);
	
}


// prototype scaling routine for sliding point scaling testing
// this version works for all numbers as long as two's modulus is in first index(?)
int SPMF::scalePM4(SPMF *pmf, SPMF *pmf2, int &width)
{
PPM *ppm = new PPM(0);
PPM *ppm_tmp = new PPM(0);
int digit2;
int end;
__int64 BinaryPower;

	ppm->AssignPM(pmf);

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
	int index = 0;
	int index2;
	int zero1;		
	int digval1 = 0;
	end = 0;
	do {

		digit2 = ppm->Rn[Mod2_index]->Digit;		// store the two's digit

		if(index != Mod2_index) {

			digval1=ppm->Rn[index]->Digit;		
			ppm->Sub(digval1);
			
			if(ppm->Zero()) {
				break;
			}

			ppm->ModDiv(Rn[index]->GetPowMod2());
			ppm->Rn[index]->SkipDigit();

		}

		zero1 = ppm->Zero();

		index += 1;

	} while(!zero1);


	width = BinaryWidth((__int64)(digit2), BinaryPower);		// get the digit width of the last non-zero two's power modulus

	pmf->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf->Rn[Mod2_index]->Digit %= power(pmf->Rn[Mod2_index]->Modulus, pmf->Rn[Mod2_index]->PowerValid);

	pmf2->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf2->Rn[Mod2_index]->Digit %= power(pmf2->Rn[Mod2_index]->Modulus, pmf2->Rn[Mod2_index]->PowerValid);

	if(index < NumFractDigits) {		// if digit conversion < NumFracDigs

		index2 = index;
		int primemod;
		while((NumFractDigits - index2) > 1) {      // do all remaining digits except the last one before the normal fraction point.

			pmf->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale main arg up
			pmf2->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale the piggyback arg same amount

			index2 += 1;
		}

		index = index2;			// for this case, the normal fraction point is used

	}
	else {

		pmf->NumFractDigits = index;			// adjust the fraction point up
		pmf2->NumFractDigits = index;		// adjust the fraction point up

	}
		
	delete(ppm_tmp);
	delete(ppm);
	
	return(index);
		
}

// prototype scaling routine for sliding point scaling testing
// this version works for all numbers
// don't forget it needs two's power modulus as largest modulus
// this is a variation for testing Rez-9, where the two's modulus is not neccesarily in the first digit index
int SPMF::scalePM5(SPMF *pmf, SPMF *pmf2, int &width)
{
PPM *ppm = new PPM(0);
PPM *ppm_tmp = new PPM(0);
int digit2;
int end;
__int64 BinaryPower;

	ppm->AssignPM(pmf);

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
	int index = 0;
	int index2;
	int zero1;		
	int digval1 = 0;
	int limit = 1;


	end = 0;
	do {

		digit2 = ppm->Rn[Mod2_index]->Digit;		// store the two's digit

		if(index != Mod2_index) {

			digval1=ppm->Rn[index]->Digit;		
			ppm->Sub(digval1);
			
			if(ppm->Zero()) {
				break;
			}

			ppm->ModDiv(Rn[index]->GetPowMod2());
			ppm->Rn[index]->SkipDigit();

		}
		else {
			limit = 0;
		}

		zero1 = ppm->Zero();

		index += 1;

	} while(!zero1);


	width = BinaryWidth((__int64)(digit2), BinaryPower);		// get the digit width of the last non-zero two's power modulus

	pmf->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf->Rn[Mod2_index]->Digit %= power(pmf->Rn[Mod2_index]->Modulus, pmf->Rn[Mod2_index]->PowerValid);

	pmf2->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf2->Rn[Mod2_index]->Digit %= power(pmf2->Rn[Mod2_index]->Modulus, pmf2->Rn[Mod2_index]->PowerValid);

	if(index < NumFractDigits) {		// if digit conversion < NumFracDigs

		index2 = index;
		int primemod;
		while((NumFractDigits - index2) > limit) {      // do all remaining digits except the last one before the normal fraction point.

			if(index != Mod2_index) {
				pmf->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale main arg up
				pmf2->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale the piggyback arg same amount
			}
			index2 += 1;
		}

		index = index2;			// for this case, the normal fraction point is used

	}
	else {

		pmf->NumFractDigits = index;			// adjust the fraction point up
		pmf2->NumFractDigits = index;		// adjust the fraction point up

	}
		
	delete(ppm_tmp);
	delete(ppm);
	
	return(index);
		
}


// same as scalePM5, but for debugging hardware
int SPMF::scalePM6(SPMF *pmf, SPMF *pmf2, int &width)
{
PPM *ppm = new PPM(0);
PPM *ppm_tmp = new PPM(0);
int digit2;
int end;
__int64 BinaryPower;

	ppm->AssignPM(pmf);

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
	int index = 0;
	int index2;
	int zero1;		
	int digval1 = 0;
	int limit = 1;


	end = 0;
	do {

		digit2 = ppm->Rn[Mod2_index]->Digit;		// store the two's digit
		printf("digit2 = %x\n", digit2);

		if(index != Mod2_index) {

			digval1=ppm->Rn[index]->Digit;		
			ppm->Sub(digval1);

			printf("subtracting by digit %x at index %d\n", digval1, index);
			ppm->PrintDemo();
			
			if(ppm->Zero()) {
				break;
			}

			ppm->ModDiv(Rn[index]->GetPowMod2());
			ppm->Rn[index]->SkipDigit();

			printf("dividing by modulus %d\n", Rn[index]->GetPowMod2());
			ppm->PrintDemo();

		}
		else {
			limit = 0;
		}

		zero1 = ppm->Zero();

		index += 1;

		wait_key();

	} while(!zero1);


	width = BinaryWidth((__int64)(digit2), BinaryPower);		// get the digit width of the last non-zero two's power modulus

	printf("digit (=%d) width of 2s power: %d\n", digit2, width);

	pmf->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf->Rn[Mod2_index]->Digit %= power(pmf->Rn[Mod2_index]->Modulus, pmf->Rn[Mod2_index]->PowerValid);

	pmf->PrintDemo();
	cout << "int value: " << pmf->PPM::Print10() << endl;
	wait_key();

	pmf2->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	pmf2->Rn[Mod2_index]->Digit %= power(pmf2->Rn[Mod2_index]->Modulus, pmf2->Rn[Mod2_index]->PowerValid);

	if(index < (NumFractDigits)) {		// if digit conversion < NumFracDigs

		printf("scaling up is needed\n");

		index2 = index;
		int primemod;
		while(((NumFractDigits) - index2) > limit) {      // do all remaining digits except the last one before the normal fraction point.

			if(index != Mod2_index) {
				pmf->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale main arg up
				pmf2->Mult(primemod=pmf->Rn[index2]->GetPowMod2());		// scale the piggyback arg same amount
			}
			index2 += 1;
		}

		index = index2;			// for this case, the normal fraction point is used

		pmf->NumFractDigits = index;			// adjust the fraction point up
		pmf2->NumFractDigits = index;		// adjust the fraction point up

	}
	else {

		printf("scaling up NOT needed\n");

		pmf->NumFractDigits = index;			// adjust the fraction point up
		pmf2->NumFractDigits = index;		// adjust the fraction point up

	}

	cout << "fractional point position: " << this->NumFractDigits << endl;
//	cout << "scaled pmf: " << pmf->PrintAbsPM(10) << endl;
	pmf->PrintDemoPM(10);		

	wait_key();
	
	delete(ppm_tmp);
	delete(ppm);
	
	return(index);
		
}

// checks to see if the two's digit is the largest, or tied with largest digit
int SPMF::scale_chk(PPM *ppm)
{
int i;
int twos_digit = ppm->Rn[0]->Digit;

	for(i=1; i<ModTable::returnNumDigits(); i++) {
		if(!ppm->Rn[i]->Skip) {
			if(ppm->Rn[i]->Digit != twos_digit) {
				return(0);
			}
		}
	}

	return(1);
}


#define MIN_FRACTION_DIGS  6
// prototype scaling routine for converting integer to simple fraction for use by GoldDivI
// under development
int SPMF::ScaleInt(PPM *n, PPM *d, SPMF *pmf_n, SPMF *pmf_d, int &width)
{
PPM *ppm_tmp = new PPM(0);
int digit2;
int first_index, end, min_frac_digs;
__int64 BinaryPower;

	
	int index = 0;
	int index2;
	int zero1;		
	int digval1 = 0;
	int limit = 1;

	ppm_tmp->AssignPM(d);

	end = 0;
	do {

		digit2 = d->Rn[Mod2_index]->Digit;		// store the two's digit

		if(index != Mod2_index) {

			digval1 = d->Rn[index]->Digit;		
			d->Sub(digval1);

			printf("digval: %d\r\n", digval1);

			if(d->Zero()) {
				break;
			}

			d->ModDiv(Rn[index]->GetPowMod2());
			d->Rn[index]->SkipDigit();
			cout << "d: " << d->Prints() << endl;

			wait_key();
		}
		else {
			limit = 0;
		}

		zero1 = d->Zero();

		index += 1;

	} while(!zero1);


	width = BinaryWidth((__int64)(digit2), BinaryPower);		// get the digit width of the last non-zero two's power modulus

	printf("width: %d, Numfractdigs: %d\n", width, index);

	d->Assign(ppm_tmp);		//restore the divisor value

	d->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	d->Rn[Mod2_index]->Digit %= power(d->Rn[Mod2_index]->Modulus, d->Rn[Mod2_index]->PowerValid);

	n->Rn[Mod2_index]->PowerValid = width;			// adjust the two's power valid
	n->Rn[Mod2_index]->Digit %= power(n->Rn[Mod2_index]->Modulus, n->Rn[Mod2_index]->PowerValid);

	pmf_d->PPM::AssignPM(d);
	pmf_n->PPM::AssignPM(n);

	first_index = index;
	min_frac_digs = index+4;
//	if(index < MIN_FRACTION_DIGS) {		// if digit conversion < NumFracDigs
	if(index < (min_frac_digs)) {

		index2 = index;
		int primemod;
//		while((MIN_FRACTION_DIGS - index2) > limit) {      // do all remaining digits except the last one before the normal fraction point.
		while((min_frac_digs - index2) > limit) {      // do all remaining digits except the last one before the normal fraction point.

			if(index != Mod2_index) {
				pmf_d->Mult(primemod=pmf_d->Rn[index2]->GetPowMod2());		// scale main arg up
				pmf_n->Mult(primemod=pmf_n->Rn[index2]->GetPowMod2());		// scale the piggyback arg same amount
			}
			index2 += 1;
		}

		index = index2;			// for this case, the normal fraction point is used

	}


	pmf_d->NumFractDigits = index;			// adjust the fraction point up
	pmf_n->NumFractDigits = index;			// adjust the fraction point up


	delete(ppm_tmp);
	
	return(index);
		
}

void SPMF::AssignTaylor_E()
{
	SPMF* sum = new SPMF(0);
	SPMF* term = new SPMF(0);

	PPM* fact = new PPM(1);

	int n = 0;

	sum->Assign(0LL);

	while (1) {

		if (n == 0 || n == 1) {
			fact->Assign(1);
		}
		else {
			fact->Mult(n);
		}

		term->AssignInversePPM(fact);

		printf("n = %2d  ", n);
		printf("n! = ");
		cout << fact->Print10() << "  ";
		cout << "term = " << term->PrintAbsPM(10) << endl;

		if (term->Zero()) {
			cout << "Rounded term reached zero at n = " << n << endl;
			cout << "Last added nonzero term was n = " << (n - 1) << endl;
			break;
		}

		sum->Add(term);

		n += 1;
	}

	this->Assign(sum);

	delete fact;
	delete term;
	delete sum;
}
