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
#include "mrn.h"
using namespace std;

// ----------------------------------------------------------------------
// Mixed Radix Class member functions
// This class is least significant digit first
// ----------------------------------------------------------------------

MRDigit::MRDigit(int digit, int power, int skip)
{

	Digit = digit;
	Power = power;
	Skip = skip;

}

// converts ppm to a mixed radix value using
// normal power modulus only; use "assign2" to
// create a MRN using valid modulus powers
MRN::MRN(PPM *ppm, int &clocks)
{
PPM *ppm_tmp = new PPM(0);
int digit;
int power = 0;

	
	ppm->Backup();

	for(int i=0; i<ppm->NumDigits; i++) {

//		ppm->Print();

		if(!ppm->Rn[i]->Skip) {
			if(digit=ppm->Rn[i]->Digit) ppm->Sub(digit);
//			ppm->counter[PPM::DEC_COUNT] += 1;
			clocks += 1;

//			printf("Digit %d val = %d is subtracted\r\n", i, digit);
//			printf("\r\n after subtract:\r\n");
//			ppm->Print();

			power = ppm->Rn[i]->GetFullPowMod();
//			printf("\r\n after Modulus %d ModDiv\r\n", power);

			ppm_tmp->Assign(ppm->Rn[i]->GetFullPowMod());
			ppm->ModDiv(ppm_tmp);
//			ppm->counter[PPM::DIV_COUNT] += 1;
			clocks += 1;

////		ppm->Rn[i]->SkipDigit();

			MRn.push_back(new MRDigit(digit, power, 0));	// push a real MRN digit
		}
		else {

			MRn.push_back(new MRDigit(0, power, 1));		// push a skipped digit so we can print, etc.

		}

//		ppm->Print();

		if(ppm->Zero(i)) {
			
//			printf("zero detected\r\n");		
			break;
		}
		
//		wait_key();
	}

	ppm->Restore();

	delete ppm_tmp;
//	printf("decomposition terminated\r\n");
//	printf("number of MR digits: %d\r\n", MRn.size());
//	this->Print();
//	printf("\r\n");

}

// used to support partial power digit testing and support
// implicit conversion from PPM to MRN, supporting skip and partial power digits
void MRN::Assign2(PPM *ppm)
{

PPM *ppm_tmp = new PPM(0);
int digit;
int power = 0;

	
	ppm->Backup();

	if(MRn.size() > 0) {
		for(unsigned int i=0; i<MRn.size(); i++) {
			delete(MRn[i]);
		}
	}

	MRn.clear();

// --------------

	for(int i=0; i<ppm->NumDigits; i++) {

//		ppm->Print();
		power = ppm->Rn[i]->GetPowMod2();		// get powervalid power
		
		if(!ppm->Rn[i]->Skip) {

//			if(digit=ppm->Rn[i]->Digit) ppm->Sub(digit);
			if(digit=ppm->Rn[i]->GetDigit()) ppm->Sub(digit);
			
//			ppm->counter[PPM::DEC_COUNT] += 1;
//			clocks += 1;

//			printf("Digit %d val = %d is subtracted\r\n", i, digit);
//			printf("\r\n after subtract:\r\n");
//			ppm->Print();

			
//			printf("\r\n after Modulus %d ModDiv\r\n", power);

			ppm_tmp->Assign(power);
			ppm->ModDiv(ppm_tmp);
//			ppm->counter[PPM::DIV_COUNT] += 1;
//			clocks += 1;

////		ppm->Rn[i]->SkipDigit();

			MRn.push_back(new MRDigit(digit, power, 0));	// push a real MRN digit
		}
		else {

			MRn.push_back(new MRDigit(0, power, 1));		// push a skipped digit so we can print, etc.

		}

//		ppm->Print();

		if(ppm->Zero(i)) {
			
//			printf("zero detected\r\n");		
			break;
		}
		
//		wait_key();
	}

	ppm->Restore();

	delete ppm_tmp;
//	printf("decomposition terminated\r\n");
//	printf("number of MR digits: %d\r\n", MRn.size());
//	this->Print();
//	printf("\r\n");

}

// return the number of non-skipped digits
int MRN::NumDigs(void)
{
int size, i;
int skips = 0;


	if((size=MRn.size()) == 0) return 0;

	for(i=0; i<size; i++) {

		if(MRn[i]->Skip) skips++;

	}

	return(size-skips);

}

// this routine creates a mixed radix number after skipping the
// first digcnt number of digits.
// used to develope constants for comparison during montMult3
// assume n is passed completely extended
int MRN::get_mrn_n(PPM *n, int digcnt)
{
int i;

	
	if(digcnt >= ModTable::returnNumDigits()) {		// not enough digits for digcnt
		return(-1);
	}

	PPM *ppm_n = new PPM(0);
	ppm_n->Assign(n);
	
	for(i=0; i<digcnt; i++) {		// skip first modulus r worth of digits
		ppm_n->Rn[i]->Skip = 1;
	}

		
	this->Assign2(ppm_n);

	delete ppm_n;
	return(0);

}



// basic conversion without clock counting
// added protection to the argument so it is not corrupted
MRN::MRN(PPM *ppm_arg)
{
PPM *ppm_tmp = new PPM(0);
PPM *ppm_copy = new PPM(0);
int digit;
int power = 0;

//	cout << "ppm at enter: " << ppm->Prints() << endl;
//	ppm->Backup();

	ppm_copy->Assign(ppm_arg);

	for(int i=0; i<ppm_copy->NumDigits; i++) {

		if(!ppm_copy->Rn[i]->Skip) {
			if(digit=ppm_copy->Rn[i]->Digit) ppm_copy->Sub(digit);

//			power = ppm->Rn[i]->GetPowMod();	// ebo, 4/12/2016
			power = ppm_copy->Rn[i]->GetPowMod2();

			ppm_tmp->Assign(power);
//			cout << ppm_tmp->Prints() << endl;
			ppm_copy->ModDiv(ppm_tmp);
//			cout << ppm->Prints() << endl;

			MRn.push_back(new MRDigit(digit, power, 0));	// push a real MRN digit
		}
		else {

			MRn.push_back(new MRDigit(0, power, 1));		// push a skipped digit so we can print, etc.
		}

		if(ppm_copy->Zero(i)) {		// leave asap to eliminate leading zeros and wasted time.
					
			break;
		}
	}

//	ppm->Restore();
	delete ppm_copy;
	delete ppm_tmp;

}

MRN::~MRN(void)
{

	for(unsigned int i=0; i<MRn.size(); i++) {			// delete the Mixed Radix Digits in Vector

//		printf("deleting MR digit %d\r\n", MRn[i]->Digit);
		delete(MRn[i]);

	}

}

void MRN::Print(void)
{

//	for(int i=0; i<MRn.size(); i++) {
	for(int i=MRn.size()-1; i>=0; i--) {

		if(MRn[i]->Skip) {
			if (i > 0) {
				cout << "*, ";
			}
			else {
				cout << "*";
			}
		}
		else {
			//printf("%x ", MRn[i]->Digit);
			if (i > 0) {
				cout << hex << nouppercase << MRn[i]->Digit << ", ";
			}
			else {
				cout << hex << nouppercase << MRn[i]->Digit;
			}
		}

	}

}

__int64 MRN::Decimal64(void)
{
int seeded = 0;
int size = MRn.size();
__int64 val=0;


	for(int i=size-1; i>=0; i--) {
		
		if(!MRn[i]->Skip) {
			if(!seeded) {
				val = MRn[i]->Digit;
				seeded = 1;
			}
			else {
				val *= MRn[i]->Power;
				val += MRn[i]->Digit;
			}
		}
	}
	
	return(val);

}

unsigned long long MRN::Decimal_ull(void)
{
int seeded = 0;
int size = MRn.size();
unsigned long long val=0;


	for(int i=size-1; i>=0; i--) {
		
		if(!MRn[i]->Skip) {
			if(!seeded) {
				val = MRn[i]->Digit;
				seeded = 1;
			}
			else {
				val *= MRn[i]->Power;
				val += MRn[i]->Digit;
			}
		}
	}
	
	return(val);

}

// Converts the MRN into a PPM and assigned to ppm argument
// returns total clocks, services PPM counters
int MRN::Convert(PPM *ppm)
{
int size = MRn.size();
int seeded = 0;
int clocks = 0;

	ppm->Assign(0);		// clear the number

	for(int i=size-1; i>=0; i--) {

		if(!MRn[i]->Skip) {
			if(!seeded) {
//				val = mrn->MRn[i]->Digit;
				ppm->Assign(MRn[i]->Digit);			// assignment is treated as a simple add to zero.
//				ppm->counter[PPM::ADD_COUNT] += 1;
				clocks += 1;
				seeded = 1;
			}
			else {
//				val *= mrn->MRn[i]->Power;
//				val += mrn->MRn[i]->Digit;

				ppm->Mult(MRn[i]->Power);
//				ppm->counter[PPM::MULT_COUNT] += 1;

				ppm->Add(MRn[i]->Digit);
//				ppm->counter[PPM::ADD_COUNT] += 1;

				clocks += 2;

			}

		}

	}

	return(clocks);
}

// Converts the PPM into an MRN, debug version
// returns total clocks, services PPM counters
int MRN::Convert2(PPM *ppm)
{
int size = MRn.size();
int seeded = 0;
int clocks = 0;

	ppm->Assign(0);		// clear the PPM argument

	for(int i=size-1; i>=0; i--) {

		if(!MRn[i]->Skip) {
			if(!seeded) {
//				val = mrn->MRn[i]->Digit;
				ppm->Assign(MRn[i]->Digit);			// assignment is treated as a simple add to zero.
				cout << "Start " << ppm->Print(DEC) << endl;
				ppm->Print();
				//printf("\n");
				cout << endl;

//				ppm->counter[PPM::ADD_COUNT] += 1;
				clocks += 1;
				seeded = 1;
			}
			else {
//				val *= mrn->MRn[i]->Power;
//				val += mrn->MRn[i]->Digit;

				ppm->Mult(MRn[i]->Power);
//				ppm->counter[PPM::MULT_COUNT] += 1;

				cout << "Mult by " << MRn[i]->Power << ": " << ppm->Print(DEC) << endl;
				
				ppm->Print();
				//printf("\n");
				cout << endl;

				ppm->Add(MRn[i]->Digit);
//				ppm->counter[PPM::ADD_COUNT] += 1;

				cout << "Add by  " << MRn[i]->Digit << ": " << ppm->Print(DEC) << endl;

				ppm->Print();
				//printf("\n");
				cout << endl;

				clocks += 2;

			}

		}

	}

	return(clocks);
}


// currently experiemental
// add this to mrn while converting to PPM
// this does work
void MRN::ConvertAdd(PPM *ppm, MRN *mrn)
{
int size;
int size1 = MRn.size();
int size2 = mrn->MRn.size();
int seeded = 0;
int clocks = 0;


	ppm->Assign(0);		// clear the ppm, prepare for return value

	if(size1 > size2) {
		size = size1;
	}
	else {
		size = size2;
	}


	for(int i=size-1; i>=0; i--) {

//		if(!MRn[i]->Skip) {
			if(!seeded) {

				if(i < size1) {
					ppm->Assign(MRn[i]->Digit);			// assignment is treated as a simple add to zero.
				}

				if(i < size2) {
					ppm->Add(mrn->MRn[i]->Digit);		// try direct add at this point
				}
				seeded = 1;

			}
			else {

				if(size1 > size2) {
					ppm->Mult(MRn[i]->Power);
				}
				else {
					ppm->Mult(mrn->MRn[i]->Power);
				}

				if(i < size1) {
					ppm->Add(MRn[i]->Digit);			
				}

				if(i < size2) {
					ppm->Add(mrn->MRn[i]->Digit);		// try direct add
				}
			}

//		}

	}


//	return(clocks);


}

// don't think this works because of binomial theorem, (correct multiply expansion)
void MRN::ConvertMult(PPM *ppm, MRN *mrn)
{
int size;
int size1 = MRn.size();
int size2 = mrn->MRn.size();
int seeded = 0;
int clocks = 0;
int digit;


	ppm->Assign(0);		// clear the ppm, prepare for return value

	if(size1 > size2) {
		size = size1;
	}
	else {
		size = size2;
	}


	if(size1 == size2) {

		for(int i=size-1; i>=0; i--) {

//		if(!MRn[i]->Skip) {
			if(!seeded) {
				ppm->Assign(MRn[i]->Digit * mrn->MRn[i]->Digit);			// assignment is treated as a simple add to zero.
//				ppm->Mult(mrn->MRn[i]->Digit);		// try direct add at this point
				seeded = 1;

			}
			else {

				digit = MRn[i]->Digit;

				
				ppm->Mult(MRn[i]->Power * MRn[i]->Power);
//				ppm->Mult(mrn->MRn[i]->Power);
				
				ppm->Add(mrn->MRn[i]->Digit * digit);

//				ppm->Add(digit);
//				if(mrn->MRn[i]->Digit != 0) {
					
//				}

//				if(mrn->MRn[i]->Digit != 0) {
					
//				}
//				if(size1 > size2) {

//					if(i < size2) {
//						ppm->Add(MRn[i]->Digit);		// try direct mult
//						if(mrn->MRn[i]->Digit) {
//						digit = MRn[i]->Digit * mrn->MRn[i]->Digit;
//						digit = digit % ModTable::primes[i];
//						ppm->Add(digit);
						
//						}
//						else {
//							ppm->Add(MRn[i]->Digit);
//						}
//					}
//					else {
//						ppm->Add(MRn[i]->Digit);			// this is longer, add it
//					}
//				}
//				else {
					
//					if(i < size1) {
//						ppm->Add(mrn->MRn[i]->Digit);		// try direct mult
//						if(MRn[i]->Digit) {
//							ppm->Mult(MRn[i]->Digit);
//						}
//					}
//					else {
//						ppm->Add(mrn->MRn[i]->Digit);			// mrn is longer
//					}
//				}
//			}

			}

		}

	}
	else {
		//printf("sizes not the same\r\n");
		cout << "sizes not the same" << endl;
	}
//	return(clocks);



}


void MRN::Assign(MRN *mrn)
{

//	vector<FRDigit *>::iterator it;			// create my first iterator

	if(MRn.size() > 0) {
		for(unsigned int i=0; i<MRn.size(); i++) {
			delete(MRn[i]);
		}
	}

	MRn.clear();
//	it = FRn.end();

	for(unsigned int i=0; i<mrn->MRn.size(); i++) {		// create the digits and copy the value
		MRn.push_back(new MRDigit(mrn->MRn[i]->Digit, mrn->MRn[i]->Power, 0));
	}


}

// Shift is equivalent to skipping positions in MR format
void MRN::ShiftRight(int n)
{
PPM *ppm = new PPM(0);
MRN *mrn = new MRN(ppm);


	if(n < MRn.size()) {
		for(int i=0; i<n; i++) {
			MRn[i]->Skip = 1;
		}
	}
	else {
		this->Assign(mrn);		// assign a zero
	}

	delete mrn;
	delete ppm;

}

// experimental barrel shift for MRN & PPM
// this version performs a splice of the MRN string itself
void MRN::ShiftRight2(int n)
{
PPM *ppm = new PPM(0);
MRN *mrn = new MRN(ppm);

//	vector<FRDigit *>::iterator it;			// create my first iterator
	
	if(n < this->MRn.size()) {

		for(unsigned int i=n; i<this->MRn.size(); i++) {

			if(i == n) {
				mrn->MRn[0]->Digit = this->MRn[i]->Digit;
				mrn->MRn[0]->Power = this->MRn[i]->Power;
			}
			else {
				mrn->MRn.push_back(new MRDigit(MRn[i]->Digit, MRn[i]->Power, 0));
			}

		}

		this->Assign(mrn);

	}
	else {

		this->Assign(mrn);
	}

	delete mrn;
	delete ppm;

}



