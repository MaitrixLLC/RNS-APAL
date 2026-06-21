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
#include <algorithm>
#include <inttypes.h>
typedef  int64_t __int64;

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
#include "mrn.h"
using namespace std;



int ModTable::ModTableInit = 0;
//int PPM::initialized = 0;			// static member is outside the class
int *ModTable::primes = NULL;
//int PPM::NumDigits = 0;		// number of digits of the PPM object

// try out some global constants, like calculated ranges
PPM *PPM::Mult_range = NULL;		// maximum value that can be reliably multplied by any value equal to or less


vector<vector<vector<short>>> ModTable::arrayDivTbl;	// this is a brute force divide lookup table, created using vectors
vector<vector<vector<short>>> ModTable::ModDivTbl;		// this is a more efficient version of ModDiv LUT, array[p][pwr][P], where p = number of modulus, pwr = powers of modulus, P = largest digit modulus 
														// the multiplicative inverse for modulus p with respect to any modulus m --> array[p][pwr][m]
int ModTable::q;
int ModTable::req_num_digs;
int ModTable::req_num_frac_digs;
int ModTable::req_mode;
int ModTable::req_routine;

// Init the Prime numbers and other LUT static members here
ModTable::ModTable(int mode, int routine, int num_digs, int num_frac_digs, int *modulus, const int *num_powers) 
{
int i, pwr;
long long maxdigit;
	
	req_mode = mode;

	req_routine = routine;
	req_num_digs = num_digs;
	req_num_frac_digs = num_frac_digs;
	req_modulus.resize(req_num_digs);
	req_powers.resize(req_num_digs);	// NOTE: this array is now forced to be same size as modulus array

	if ((mode & CUSTOM) && (mode & NO_POWERS)) {
		for (i = 0; i < req_num_digs; i++) {

			req_modulus[i] = Modulus[i];

		}
	}
	else if (mode & CUSTOM) {
		for (i = 0; i < req_num_digs; i++) {
			req_modulus[i] = Modulus[i];
			req_powers[i] = num_powers[i];
		}
	}
	

//	wait_key();

	if(!this->ModTableInit) {				// prime array is first!
		primes = new int[req_num_digs];
		if(!(req_mode & CUSTOM)) {                             // If NOT using CUSTOM_MODULUS, then generate the primes in sequence
			init_prime(primes, req_num_digs);
		}
		else if(req_mode & CUSTOM) {                        // else, if using CUSTOM_MODULUS, then generate the primes using "Modulus[]" array
			for(i=0; i< req_num_digs; i++) {
				primes[i] = Modulus[i];
			}
		}
	}
	
//
// This section creates the static tables for LUT based ModDiv() method
//
	if(!this->ModTableInit) {

//		wait_key();

		this->ModTableInit = 1;
		BaseClass = 1;						// this is the base class, the class that instantiated static configuration variables

		q = GetMaxDigWidth(maxdigit, req_num_digs);			// assign the q value of the RNS system (largest digit width in bits)
		unsigned int m = maxdigit;
		
		cout << "Q = " << q << ", LUT Adr Range = " << maxdigit;

		if (ModTable::returnRoutine() == USE_MODDIV_LUT) {
			long long size = 0;

			ModDivTbl.resize(ModTable::returnNumDigits());
			for (int i = 0; i < ModDivTbl.size(); i++) {
				GetDigitMod(i, pwr, q);
				ModDivTbl[i].resize(pwr);
				size += pwr * m;
				for (int j = 0; j < pwr; j++) {

					ModDivTbl[i][j].resize(m);

				}
			}

			//printf("Smaller LUT memory allocation: %lu bytes\n", size / sizeof(short));
			cout << "Smaller LUT memory allocation: " << size / sizeof(short) << " bytes" << endl;
			for (int i = 0; i < ModDivTbl.size(); i++) {
				for (int j = 0; j < ModDivTbl[i].size(); j++) {
					for (int k = 0; k < ModDivTbl[i][j].size(); k++) {

						ModDivTbl[i][j][k] = -1;			// invalidate all entries first
					}
				}
			}

			for (int i = 0; i < ModDivTbl.size(); i++) {        // there are m entries in p index, but we only fill the primes right now
				for (int p = 0; p < ModDivTbl[i].size(); p++) {
					int pmod = power(primes[i], p + 1);
					for (int j = 0; j < ModDivTbl[i][p].size(); j++) {				// this is the modulus that is "with respect to" (ALL POWERS)

						int resp_mod = j;
						int trial = -1;
						int k = 0;
						for (k = 0; k < m; k++) {

							trial = (resp_mod * k) % pmod;
							if (trial == 1) break;

						}

						if (trial == 1) {
							ModDivTbl[i][p][resp_mod] = k;
							//					printf("inverse of mod %d, power %d, with respect to %d is %d\n", pmod, p, resp_mod, k);
							//					wait_key();
						}
					}
				}
			}

		}
		else if (ModTable::returnRoutine() & USE_BRUTE_LUT) {
			long long size = 0;
			// allocate storage for massive three dimensional array
			arrayDivTbl.resize(ModTable::returnNumDigits());
			for (int i = 0; i < ModTable::returnNumDigits(); i++) {
				arrayDivTbl[i].resize(m);

				for (int j = 0; j < m; j++) {
					arrayDivTbl[i][j].resize(m);
					size += m * ModTable::returnNumDigits();
				}
			}

			cout << "Brute force LUT memory allocation: " << size / sizeof(short) << " bytes" << endl;

			for (int i = 0; i < arrayDivTbl.size(); i++) {
				for (int j = 0; j < arrayDivTbl[i].size(); j++) {
					for (int k = 0; k < arrayDivTbl[i][j].size(); k++) {

						arrayDivTbl[i][j][k] = -1;		// invalidate all entries first

					}
				}
			}


			for (int i = 0; i < arrayDivTbl.size(); i++) {

				for (int j = 0; j < GetDigitMod(i, pwr, q); j++) {
					for (int k = 0; k < GetDigitMod(i, pwr, q); k++) {

						if (int temp = (j * k) % GetDigitMod(i, pwr, q)) {		// if non zero ...

							arrayDivTbl[i][temp][j] = k;
						}
						else {								// zero cases... 
							if (j) {

								arrayDivTbl[i][temp][j] = 0;
							}
							else {

								arrayDivTbl[i][temp][j] = 0;
							}
						}
					}
				}
			}
		}
		else {				// Use Extended Euclidean
			cout << "No LUT memory allocation: using extended Euclidean" << endl;
		}

	}
//	else {
//		BaseClass = 0;									// this is not the base class
//	}

// Post static intialization initialzations!
	
}

// the GetDigitMod function returns the digit modulus for each Rn[], and also the Powers value during ModTable init
// one of three versions is called depending on the value assigned to #define CUSTOM_POWERS and the #define NO_POWERS
int ModTable::GetDigitMod(int index, int &pwr, int q)
{
int mode = ModTable::returnMode();

	if((mode & CUSTOM) && !(mode & NO_POWERS)) {
		return(GetCustDigitMod(index, pwr));
	}
	else {
		
		if (mode & NO_POWERS) { 
			pwr = 1;
			return(primes[index]);
		}
		else
			return(GetAutoDigitMod(index, pwr, q));
		
//#ifdef NO_POWERS
//		pwr = 1;
//		return(primes[index]);
//#else
//		return(GetAutoDigitMod(index, pwr, q));
//#endif
		
	}

}

// Return the modulus and power of each Rn[] modulus when using #define CUSTOM_MODULUS 1
// first array element of ModPowers[] defines power for first modulus, and so on
// make sure that the ModPowers array is adequatly deined prior to using the custom modulus feature
int ModTable::GetCustDigitMod(int index, int &pwr)
{

	int size = sizeof(ModPowers)/sizeof(int);		// find number of elements in the array
	if(index < size) {								// if array has an power for this modulus, use it
		pwr = ModPowers[index];
		return(power(primes[index], pwr));
	}
	else {
		pwr = 1;
		return(primes[index]);
	}
	
}

// Return the modulus and power of each Rn[] modulus when using #define CUSTOM_MODULUS 0
// this provides the auto_power function if using #define Custom_Powers 0
int ModTable::GetAutoDigitMod(int index, int &pwr, int q)
{
__int64 binary_power;
int temp_width;

	pwr = 1;
	while(1) {

		temp_width = BinaryWidth(power(primes[index], pwr+1), binary_power);

		if(primes[index] == 2) temp_width -= 1;     // two's modulus is special case, the width is one less than returned

		if(temp_width <= q) {
			pwr++;
		}
		else {
			return(power(primes[index], pwr));
		}
	
	}

	return(0);		// should never get here

}

// This function returns the correct maximum width (Q) of the power based RNS system
// and it's power of two via the maxdigit argument
int ModTable::GetMaxDigWidth(long long &maxdigit, int req_num_digs)
{
int i, mod, pwr, binwidth;
__int64 digwidth;
int mode = ModTable::returnMode();

	maxdigit = 0;
	binwidth = 0;

	if(mode & CUSTOM) {
		for(i=0; i< req_num_digs; i++) {
		
			mod = GetCustDigitMod(i, pwr);
			binwidth = BinaryWidth(mod, digwidth);

			if(digwidth > maxdigit) {
				maxdigit = digwidth;
			}

		}
	}
	else {
		for(i=0; i< req_num_digs; i++) {
		
			mod = primes[i];
			binwidth = BinaryWidth(mod, digwidth);

			if(digwidth > maxdigit) {
				maxdigit = digwidth;
			}

		}
	}

	return(binwidth);

}

int ModTable::returnNumDigits(void)
{

	return req_num_digs;

}
int ModTable::returnNumFracDigits(void)
{

	return req_num_frac_digs;

}

int ModTable::returnMode(void)
{

	return req_mode;

}


int ModTable::returnRoutine(void)
{

	return req_routine;

}

//int ModTable::returnPrintRemainder(void)
//{
//
//	return req_print_remainder;
//
//}
// ***************************************  PPM CLASS BELOW  **********************************

// this is the primary constructor for the PPM class
PPM::PPM(__int64 x)
{
int power;
	
	if(x < 0) x = -x;		// need absolute value to support signed SPPM2 as derived

	NumDigits = ModTable::returnNumDigits();	
	PowerBased = 0;					// start with assumption that class is NOT power based
	
	Mod2_index = INVALID_MODULUS;		// start with "invalid modulus" for 2 and 5 modulus index
	Mod5_index = INVALID_MODULUS;

	Rn.resize(NumDigits);				// make the Rn array as large as requested
	for(int i=0; i<NumDigits; i++) {

		int digval = (int)(x % ModTable::GetDigitMod(i, power, ModTable::q));
		if(power > 1) PowerBased = 1;								// if even one digit is power based, the class is power based, therefore, set the powerbased flag true
	
		if(!(ModTable::primes[i] % 2)) Mod2_index = i;				// search for the 2 and 5 modulus digit positions for print(10) and print(16) radix methods
		if(!(ModTable::primes[i] % 5)) Mod5_index = i;

		Rn[i] = new PPMDigit(digval, i, power);			
		Rn[i]->Skip = 0;

	}
	
	Clear_Counters();
	ena_dplytrc = 0;			// disable divide trace by default

}

// inits the PPM and automatically copies the modulus format properties of the ppm argument
// the __int64 x argument is to stop the compiler from confusing the PPM arg from a __int64 arg
// CONSIDER HAVING A BETTER VERSION BY SUPPORTING PPM(THIS, __INT64), AND ALSO SUPPORTING PPM(THIS, THIS);
// BOTH VERSIONS COPY THE FIRST PPM ARGUMENT FORMAT, THEN EITHER ASSIGN VALUE, OR ASSIGN THIS
PPM::PPM(PPM *copyval, __int64 x)
{
int power, digval;

//	if(x < 0) x = -x;		// need absolute value to support signed SPPM2 as derived

	NumDigits = copyval->NumDigits;	
	PowerBased = copyval->PowerBased;					// start with assumption that class is power based
	
	Mod2_index = copyval->Mod2_index;
	Mod5_index = copyval->Mod5_index;

	Rn.resize(NumDigits);				// make the Rn array as large as the base class instance

	for(int i=0; i<copyval->NumDigits; i++) {


		digval = copyval->Rn[i]->Digit;
		power = copyval->Rn[i]->Power;
		Rn[i] = new PPMDigit(digval, i, power);	

		Rn[i]->CopyDigit(copyval->Rn[i]);			// copy the whole arg digit into the new digit
		Rn[i]->Power = copyval->Rn[i]->PowerValid;	// derive the new partial power type
		
		if(x >= 0) {                                // redo th digit value depending on parameter x, if negative, then  use copyval, else use x
			Rn[i]->Digit = x % Rn[i]->Power;
		}
		else {
			Rn[i]->Digit = copyval->Rn[i]->Digit;
		}
	
	}

	Clear_Counters();
	ena_dplytrc = 0;

}

// PPM constructor without any arguments, init to zero
PPM::PPM(void)
{


	int power, x;
	x = 0;
	
//	if(x < 0) x = -x;		// need absolute value to support signed SPPM2 as derived

	NumDigits = ModTable::returnNumDigits();	
	PowerBased = 0;					// start with assumption that class is NOT power based
	
	Mod2_index = INVALID_MODULUS;		// start with "invalid modulus" for 2 and 5 modulus index
	Mod5_index = INVALID_MODULUS;

	Rn.resize(NumDigits);				// make the Rn array as large as requested
	for(int i=0; i<NumDigits; i++) {

		int digval = (int)(x % ModTable::GetDigitMod(i, power, ModTable::q));
		if(power > 1) PowerBased = 1;								// if even one digit is power based, the class is power based, therefore, set the powerbased flag true
	
		if(!(ModTable::primes[i] % 2)) Mod2_index = i;				// search for the 2 and 5 modulus digit positions for print(10) and print(16) radix methods
		if(!(ModTable::primes[i] % 5)) Mod5_index = i;

		Rn[i] = new PPMDigit(digval, i, power);			
		Rn[i]->Skip = 0;

	}
	
	Clear_Counters();
	ena_dplytrc = 0;			// disable divide trace by default

}

PPM::~PPM(void)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		delete(Rn[i]);

	}

	Rn.clear();		// clear the vector
	

}

// Print now handles the skip digit by printing as an asterick
void PPM::Print(void)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		if(!Rn[i]->Skip) {
			printf("%2d ", Rn[i]->Digit);
		}
		else {
			cout << "*** ";
		}
	}

}

// print raw PPM value in a BCFR power based format
// never finished, obviously
void PPM::PrintPM(void)
{
int colcnt = 0;
int j;

	for(unsigned int i=0; i<Rn.size(); i++) {

		if(!Rn[i]->Skip) {
//			
			colcnt += PrintDigPM(i);
		}
		else {
			for(j=0; j<Rn[i]->Power; j++) {
				cout << "*";
				colcnt += 1;
			}
		}
		cout << " ";
	}

	cout << endl;

}

// prints the digit with leading zero for that particular digit, and returns the digit count for formatting on output
int PPM::PrintDigPM(unsigned int digit)
{
int cnt = 0;
int i, dig1, dig2;

	digit = 0;
	if(digit < NumDigits) {

		dig1 = Rn[digit]->Digit;			// get the digit value
		
		for(i=Rn[digit]->Power; i>0; i--) {

			dig2 = dig1 / (power(Rn[digit]->Modulus, i-1));

			cout << dig2;

			dig1 = dig1 - (dig2 * power(Rn[digit]->Modulus, i-1));
			cnt += 1;
		}

	}

	return(cnt);

}
/*
// mostly the same as PrintDemoPM below, but supports seperate radix for hdr and value
void PPM::PrintDemo(void)
{
unsigned int done = 0;
unsigned int i = 0;
unsigned int len = 0;
unsigned int temp_len, maxwidth, size, k, last_i;
char hdr[500];
int digsize[NUM_PPM_DIGS];
char digit[10];

int radix = 10;				// CHANGE THE RADIX VALUES HERE
int hdr_radix = 10;

	digit[0] = 0;
	hdr[0] = 0;
	last_i = 0;

	size = NUM_PPM_DIGS-1;							// find the width of the largest possible digit, assumes last digit is at least size of largest digit in digit width
	itoa(Rn[size]->GetFullPowMod(), digit, radix); 
	maxwidth = strlen(digit) + 1;

	while(!done) {

		while((i< Rn.size()) && (len < (CONSOLE_WIDTH-maxwidth))) {           // print modulus header
	
			if(Rn[i]->PowerValid == 0) {
				itoa(Rn[i]->GetFullPowMod(), digit, hdr_radix);				// testing the print radix feature using active modulus
			}
			else {
				itoa(Rn[i]->GetPowMod2(), digit, hdr_radix);
			}

			temp_len = strlen(digit);
			strcat(hdr, digit);
			strcat(hdr, " ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}
		

		printf("%s\r\n", hdr);			// print header border
		for(k=last_i; k<i; k++) {
			size = digsize[k];
			while(size) {
				printf("-");
				size -= 1;
			}
			printf(" ");
		}
		printf("\r\n");

		for(k=last_i; k<i; k++) {           // print the digit value itself

			if(!Rn[k]->Skip) {
				itoa(Rn[k]->Digit, digit, radix); 
			}
			else {
				digit[0] = 0;
				strcpy(digit, "*");
			}

			size = strlen(digit);
			if(size > digsize[k]) {     // there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while(size) {
					printf("?");
					size -= 1;
				}
				printf(" ");
			}
			else {
				printf("%s", digit);                 // digit is smaller than modulus size, so pad with blanks
				while(digsize[k] - size) {
					printf(" ");
					size += 1;
					}
				printf(" ");
			}
		}
		printf("\r\n");
		

		hdr[0] = 0;
		len = 0;
		last_i = i;
//		wait_key();

		if(i >= Rn.size()) break;

	}

	printf("\n");
}
*/

// mostly the same as PrintDemoPM below, but supports seperate radix for hdr and value
void PPM::PrintDemo(void)
{
	unsigned int done = 0;
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int temp_len, maxwidth, size, k, last_i;
	string hdr = "";
	vector<int> digsize(NumDigits);
	//int digsize[NUM_PPM_DIGS]; 1/9/20 replaced with a vector in order to initialize digsize with size NumDigits

	int radix = 10;																// CHANGE THE RADIX VALUES HERE
	int hdr_radix = 10;

	last_i = 0;

	size = NumDigits - 1;													// find the width of the largest possible digit, assumes last digit is at least size of largest digit in digit width
	string digit = to_string(Rn[size]->GetFullPowMod());						// we assume last digit is largest modulus

	maxwidth = digit.length() + 1; 

	while (!done) {

		while ((i < Rn.size()) && (len < (console_width - maxwidth))) {          // print modulus header

			if (Rn[i]->PowerValid == 0) {										// testing the print radix feature using active modulus
				digit = to_string(Rn[i]->GetFullPowMod());
			}
			else {
				digit = to_string(Rn[i]->GetPowMod2());
			}

			temp_len = digit.length();
			hdr.append(digit);
			hdr.append(" ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}


		cout << hdr << endl; 													// print header border
		for (k = last_i; k < i; k++) {
			size = digsize[k];
			while (size) {
				cout << "-"; 
				size -= 1;
			}
			cout << " ";  
		}
		cout << endl;

		for (k = last_i; k < i; k++) {										 // print the digit value itself

			if (!Rn[k]->Skip) {
				digit = to_string(Rn[k]->Digit);
			}
			else {
				digit = "*";
			}

			size = digit.length(); 
			if (size > digsize[k]) {										// there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while (size) {
					cout << "?";  
					size -= 1;
				}
				cout << " "; 
			}
			else {
				cout << digit;											 // digit is smaller than modulus size, so pad with blanks
				while (digsize[k] - size) {
					cout << " ";  
					size += 1;
				}
				cout << " "; 
			}
		}
		cout << endl;

		hdr = "";
		len = 0;
		last_i = i;
		//wait_key();

		if (i >= Rn.size()) break;

	}

	cout << endl;
}

// mostly the same as PrintDemoPM below, but supports seperate radix for hdr and value
/*void PPM::PrintDemo(int radix)
{
int done = 0;
unsigned int i = 0;
unsigned int len = 0;
unsigned int temp_len, maxwidth, size, k, last_i;
char hdr[500];
int digsize[NUM_PPM_DIGS];
char digit[10];

	int hdr_radix = radix;

	digit[0] = 0;
	hdr[0] = 0;
	last_i = 0;

	size = NUM_PPM_DIGS-1;							// find the width of the largest possible digit, assumes last digit is at least size of largest digit in digit width
	itoa(Rn[size]->GetFullPowMod(), digit, radix); 
	maxwidth = strlen(digit) + 1;

	while(!done) {

		while((i < Rn.size()) && (len < (CONSOLE_WIDTH-maxwidth))) {           // print modulus header
	
			if(Rn[i]->PowerValid == 0) {
				itoa(Rn[i]->GetFullPowMod(), digit, hdr_radix);			// testing the print radix feature using active modulus
			}
			else {
				itoa(Rn[i]->GetPowMod2(), digit, hdr_radix);
			}

			temp_len = strlen(digit);
			strcat(hdr, digit);
			strcat(hdr, " ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}
		

		printf("%s\r\n", hdr);			// print header border
		for(k=last_i; k<i; k++) {
			size = digsize[k];
			while(size) {
				printf("-");
				size -= 1;
			}
			printf(" ");
		}
		printf("\r\n");

		for(k=last_i; k<i; k++) {           // print the digit value itself

			if(!Rn[k]->Skip) {
				if(k == last_i) {
					itoa(Rn[k]->Digit, digit, radix);				// testing the print radix feature
				}
				else {
					itoa(Rn[k]->Digit, digit, radix); 
				}
			}
			else {
				digit[0] = 0;
				strcpy(digit, "*");
			}

			size = strlen(digit);
			if(size > digsize[k]) {     // there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while(size) {
					printf("?");
					size -= 1;
				}
				printf(" ");
			}
			else {
				printf("%s", digit);                 // digit is smaller than modulus size, so pad with blanks
				while(digsize[k] - size) {
					printf(" ");
					size += 1;
					}
				printf(" ");
			}
		}
		printf("\r\n");
		

		hdr[0] = 0;
		len = 0;
		last_i = i;
//		wait_key();

		if(i >= Rn.size()) break;

	}

	printf("\n");
}*/

string my_to_string(int val, unsigned int radix)
{
	char c= NULL;
	string s;
	bool flag = TRUE;
	if (val < 0)
	{
		flag = FALSE;
		val = -val;
	}
	if (val == 0)
	{
		s = '0';
	}
	else
	{
		while (val)
		{
			if (val % radix < 10)
			{
				c = (val % radix) + '0';     //s.append(to_string(val % radix));
			}
			else
			{
				c = (val % radix) + 'A' - 10;
			}
			s.push_back(c);
			val = val / radix;
		}
		if (flag == FALSE)
		{
			string t;
			t[0] = '-';
			for (int i = 0; i < s.length(); i++)
			{
				t[i + 1] = s[i];
			}
			s = t;
		}
		reverse(s.begin(), s.end());
	}
	return s;
}

void PPM::PrintDemo(int radix)
{
	int done = 0;
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int temp_len, maxwidth, size, k, last_i;
	string hdr = "";

	vector<int> digsize(NumDigits);
	//int digsize[NUM_PPM_DIGS]; 1/9/20 replaced with a vector in order to initialize digsize with size NumDigits

	string digit = ""; 

	int hdr_radix = radix;

	last_i = 0;

	size = NumDigits - 1;							// find the width of the largest possible digit, assumes last digit is at least size of largest digit in digit width
	digit = my_to_string(Rn[size]->GetFullPowMod(), hdr_radix);
	maxwidth = digit.length() + 1; 

	while (!done) {

		while ((i < Rn.size()) && (len < (console_width - maxwidth))) {           // print modulus header

			if (Rn[i]->PowerValid == 0) {
				digit = my_to_string(Rn[i]->GetFullPowMod(), hdr_radix); // testing the print radix feature using active modulus
			}
			else {
				digit = my_to_string(Rn[i]->GetPowMod2(), hdr_radix);
			}

			temp_len = digit.length();
			hdr.append(digit); 
			hdr.append(" ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}


		cout << hdr << endl;	// print header border
		for (k = last_i; k < i; k++) {
			size = digsize[k];
			while (size) {
				cout << "-"; // printf("-");
				size -= 1;
			}
			cout << " ";// printf(" ");
		}
		cout << endl;  

		for (k = last_i; k < i; k++) {           // print the digit value itself

			if (!Rn[k]->Skip)
				digit = my_to_string(Rn[k]->Digit, hdr_radix);
			else 
			{
				digit = "*";
			}

			size = digit.length();
			if (size > digsize[k]) {     // there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while (size) {
					cout << "?"; 
					size -= 1;
				}
				cout << " ";
			}
			else {
				cout << digit;             // digit is smaller than modulus size, so pad with blanks
				while (digsize[k] - size) {
					cout << " ";
					size += 1;
				}
				cout << " ";
			}
		}
		cout << endl;


		hdr[0] = 0;
		len = 0;
		last_i = i;
		//wait_key();

		if (i >= Rn.size()) break;

	}

	cout << endl; 
}

/*
void PPM::PrintDemoPM(int radix)
{
int done = 0;
unsigned int i = 0;
unsigned int len = 0;
unsigned int temp_len, maxwidth, size, k, last_i;
char hdr[500];
int digsize[NUM_PPM_DIGS];
char digit[10];

	digit[0] = 0;
	hdr[0] = 0;
	last_i = 0;

	size = NUM_PPM_DIGS-1;							// find the width of the largest possible digit
	itoa(Rn[size]->GetFullPowMod(), digit, radix); 
	maxwidth = strlen(digit) + 1;

//	printf("maxwidth = %d\r\n", maxwidth);
//	wait_key();

	while(!done) {

		while((i < Rn.size()) && (len < (CONSOLE_WIDTH-maxwidth))) {           // print modulus header
	
			if(Rn[i]->PowerValid == 0) {
				itoa(Rn[i]->GetFullPowMod(), digit, radix);		// If no powers, then print base modulus
			}
			else {
				itoa(Rn[i]->GetPowMod2(), digit, radix);
			}

			temp_len = strlen(digit);
			strcat(hdr, digit);
			strcat(hdr, " ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}
		

		printf("%s\r\n", hdr);			// print header border
		for(k=last_i; k<i; k++) {
			size = digsize[k];
			while(size) {
				if(Rn[k]->Skip) 
					printf("-");
				else if(Rn[k]->PowerValid < Rn[k]->Power) 
					printf("=");
				else 
					printf("-");
				size -= 1;
			}
			printf(" ");
		}
		printf("\r\n");

		for(k=last_i; k<i; k++) {           // print the digit value itself

			if(!Rn[k]->Skip) {
				if(k == last_i) {
					itoa(Rn[k]->Digit, digit, radix);				// testing the print radix feature
				}
				else {
					itoa(Rn[k]->Digit, digit, radix); 
				}
			}
			else if(Rn[k]->PowerValid) {

				itoa(Rn[k]->Digit, digit, radix);
			}
			else {
				digit[0] = 0;
				strcpy(digit, "*");
			}

			size = strlen(digit);
			if(size > digsize[k]) {     // there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while(size) {
					printf("?");
					size -= 1;
				}
				printf(" ");
			}
			else {
				printf("%s", digit);                 // digit is smaller than modulus size, so pad with blanks
				while(digsize[k] - size) {
					printf(" ");
					size += 1;
					}
				printf(" ");
			}
		}

		printf("\r\n");
		

		hdr[0] = 0;
		len = 0;
		last_i = i;
//		wait_key();

		if(i >= Rn.size()) break;

	}

	printf("\n");


}*/



void PPM::PrintDemoPM(int radix)
{
	int done = 0;
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned int temp_len, maxwidth, size, k, last_i;

	string hdr = ""; 

	vector<int> digsize(NumDigits);
	//int digsize[NUM_PPM_DIGS]; 1/9/20 replaced with a vector in order to initialize digsize with size NumDigits

	string digit = ""; 


	last_i = 0;

	int hdr_radix = radix;

	size = NumDigits - 1;							// find the width of the largest possible digit
	digit = my_to_string(Rn[size]->GetFullPowMod(), hdr_radix);
	maxwidth = digit.length(); 


	//	wait_key();


	while (!done) {

		while ((i < Rn.size()) && (len < (console_width - maxwidth))) {           // print modulus header

			if (Rn[i]->PowerValid == 0) {
				digit = my_to_string(Rn[i]->GetFullPowMod(), hdr_radix); // testing the print radix feature using active modulus
			}
			else {
				digit = my_to_string(Rn[i]->GetPowMod2(), hdr_radix);
			}

			temp_len = digit.length();
			hdr.append(digit);
			hdr.append(" ");

			len += temp_len + 1;
			digsize[i] = temp_len;
			i += 1;
		}


		cout << hdr << endl;			// print header border
		for (k = last_i; k < i; k++) {
			size = digsize[k];
			while (size) {
				if (Rn[k]->Skip)
					cout << "-"; 
				else if (Rn[k]->PowerValid < Rn[k]->Power)
					cout << "="; 
				else
					cout << "-"; 
				size -= 1;
			}
			cout << " "; 
		}
		cout << endl; 

		for (k = last_i; k < i; k++) {           // print the digit value itself

			if (!Rn[k]->Skip)
				digit = my_to_string(Rn[k]->Digit, hdr_radix);
			else
			{
				digit = "*";
			}

			size = digit.length(); 
			if (size > digsize[k]) {     // there is a digit to modulus size mismatch, print ?
				size = digsize[k];
				while (size) {
					cout << "?"; 
					size -= 1;
				}
				cout << " "; 
			}
			else {
				cout << digit;           // digit is smaller than modulus size, so pad with blanks
				while (digsize[k] - size) {
					cout << " ";	
					size += 1;
				}
				cout << " ";
			}
		}

		cout << endl;


		hdr[0] = 0;
		len = 0;
		last_i = i;
		//wait_key();

		if (i >= Rn.size()) break;

	}

	cout << endl;


}


// added print handling routine for partial digit
string PPM::Prints(void)
{
string output;


	for(unsigned int i=0; i<Rn.size(); i++) {

		std::stringstream ss;
		if(!Rn[i]->Skip) {
			if(Rn[i]->PowerValid == Rn[i]->Power) {		// add check for partial digits
//				ss << dec << Rn[i]->Digit << " ";
				ss << hex << Rn[i]->Digit << " ";
			}
			else {										// print a partial digit
//				ss << dec << "X|" << Rn[i]->GetDigit() << " ";
				ss << hex << "X|" << Rn[i]->GetDigit() << " ";
			}
		}
		else {
			ss << dec << "* ";
		}
		output.append(ss.str());

	}

	return(output);
}

// print raw RNS value in either Decimal or Hexidecimal format
string PPM::Prints(int radix)
{
string output;


	if(radix == 10) {
		for(unsigned int i=0; i<Rn.size(); i++) {

			std::stringstream ss;
			if(!Rn[i]->Skip) {
				if(Rn[i]->PowerValid == Rn[i]->Power) {		// add check for partial digits
					ss << dec << Rn[i]->Digit << " ";
				}
				else {										// print a partial digit
					ss << dec << "X|" << Rn[i]->GetDigit() << " ";
				}
			}
			else {
				ss << dec << "* ";
			}
			output.append(ss.str());
		}
	}
	else if(radix == 16) {
		for(unsigned int i=0; i<Rn.size(); i++) {

			std::stringstream ss;
			if(!Rn[i]->Skip) {
				if(Rn[i]->PowerValid == Rn[i]->Power) {		// add check for partial digits
					ss << hex << Rn[i]->Digit << " ";
				}
				else {										// print a partial digit
					ss << hex << "X|" << Rn[i]->GetDigit() << " ";
				}
			}
			else {
				ss << dec << "* ";
			}
			output.append(ss.str());
		}

	}
	else {
		cout << "Prints(): no such radix supported" << endl;
	}
	return(output);

}

// routine to print the decimal integer string equivalent
// adding support for power based moduli, but NOT partial digits
// to use this routine, there must exist a two's modulus, and a five's modulus
string PPM::Print10(void)
{
int val2 = 0;
int val5 = 0;
//PPM *ppmtmp = new PPM(0);
PPM *ppm2 = new PPM(0L);
std::string s;

	ppm2->Assign(this);
	ppm2->Normalize();

	if((ppm2->Mod2_index == INVALID_MODULUS) || (ppm2->Mod5_index == INVALID_MODULUS)) {
		cout << "ERROR: Print10() cannot print if 2 and 5 modulus are invalid" << endl; // printf("ERROR: Print10() cannot print if 2 and 5 modulus are invalid\n");
//		wait_key();
	}
	else if(!ppm2->Zero()) {              // Need this check to make sure mod 2 and mode 5 exists!
			
//		printf("conversion to decimal: ");
		while(!ppm2->Zero()) {

			if((ppm2->Rn[Mod2_index]->Skip == 1) || (ppm2->Rn[Mod5_index]->Skip == 1)) {
				ppm2->ExtendPart2Norm();

//				printf("\nafter base extend2:\n");
//				ppm->PrintDemo();
			}

			val2 = ppm2->Rn[Mod2_index]->GetPowOffset(1);
			ppm2->Sub(val2);
			ppm2->ModDiv(2);

			val5 = ppm2->Rn[Mod5_index]->GetPowOffset(1);
			ppm2->Sub(val5);
			ppm2->ModDiv(5);

			int decval = val5 * 2 + val2;

			s.insert(0, 1, '0'+decval); 
//			printf("%d", decval);

//			wait_key();
		}
	}
	else {

		s.insert(0, 1, '0'); 
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm2);
//	delete(ppmtmp);

	return(s);
}

// routine to print the hexadecimal integer value equivalent
// adding support for power based moduli, but NOT partial digits
// to use this routine, there must exist four powers of the two's modulus
string PPM::Print16(void)
{
int val16 = 0;
int zero_flag;
//int val5 = 0;
//PPM *ppmtmp = new PPM(0);
PPM *ppm2 = new PPM(0L);
std::string s;

	ppm2->Assign(this);
	ppm2->Normalize();			// this allows partial power number formats to be printed

	if(ppm2->Mod2_index == INVALID_MODULUS) {
		cout << "ERROR: Print16() cannot print if 2 modulus is invalid" << endl; //printf("ERROR: Print16() cannot print if 2 modulus is invalid\n");
//		wait_key();
	}
	else if(ppm2->Rn[Mod2_index]->PowerValid < 4) {
		cout << "Error: Print16(), not enough powers of 2 for routine as currently written" << endl;//printf("Error: Print16(), not enough powers of 2 for routine as currently written\n");
	}
	else if(!(zero_flag=ppm2->Zero())) {              // Need this check to make sure mod 2 and mode 5 exists!

		while(!ppm2->Zero()) {

			if(ppm2->Rn[Mod2_index]->PowerValid < 4) {
				ppm2->ExtendPart2Norm();			// just extend each cycle, it's easier
			}

			val16 = ppm2->Rn[Mod2_index]->GetPowOffset(4);
			ppm2->Sub(val16);
			ppm2->ModDiv(16);

//			printf("val16: %d\n", val16);
//			cout << "ppm: " << ppm2->Prints() << endl;

			int hexval = val16;
			if(hexval < 10) {
				s.insert(0, 1, '0'+hexval); 
			}
			else {
				s.insert(0, 1, 'a'+(hexval-10));
			}

//			wait_key();
		}
		s.insert(0, 1, 'x');
		s.insert(0, 1, '0');
	}
	else if(zero_flag) {

		s.insert(0, 1, '0'); 
		s.insert(0, 1, 'x');
		s.insert(0, 1, '0');
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm2);
//	delete(ppmtmp);

	return(s);
}

// routine to print the hexadecimal integer value equivalent
// adding support for power based moduli, but NOT partial digits
// to use this routine, there must exist four powers of the two's modulus
// does not print the 0x header
string PPM::Print16_NoHdr(void)
{
int val16 = 0;
int zero_flag;
PPM *ppm2 = new PPM(0L);
std::string s;

	ppm2->Assign(this);
	ppm2->Normalize();			// this allows partial power number formats to be printed

	if(ppm2->Mod2_index == INVALID_MODULUS) {
		cout << "ERROR: Print16() cannot print if 2 modulus is invalid" << endl; //printf("ERROR: Print16() cannot print if 2 modulus is invalid\n");
//		wait_key();
	}
	else if(ppm2->Rn[Mod2_index]->PowerValid < 4) {
		cout << "Error: Print16(), not enough powers of 2 for routine as currently written" << endl; //printf("Error: Print16(), not enough powers of 2 for routine as currently written\n");
	}
	else if(!(zero_flag=ppm2->Zero())) {              // Need this check to make sure mod 2 and mode 5 exists!

		while(!ppm2->Zero()) {

			if(ppm2->Rn[Mod2_index]->PowerValid < 4) {
				ppm2->ExtendPart2Norm();			// just extend each cycle, it's easier
			}

			val16 = ppm2->Rn[Mod2_index]->GetPowOffset(4);
			ppm2->Sub(val16);
			ppm2->ModDiv(16);

//			printf("val16: %d\n", val16);
//			cout << "ppm: " << ppm2->Prints() << endl;

			int hexval = val16;
			if(hexval < 10) {
				s.insert(0, 1, '0'+hexval); 
			}
			else {
				s.insert(0, 1, 'a'+(hexval-10));
			}

//			wait_key();
		}
//		s.insert(0, 1, 'x');
//		s.insert(0, 1, '0');
	}
	else if(zero_flag) {

		s.insert(0, 1, '0'); 
//		s.insert(0, 1, 'x');
//		s.insert(0, 1, '0');
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm2);

	return(s);
}

string PPM::Print(int radix)
{

	if(radix == 2) {
		return(this->Print2());
	}
	else if(radix == 10) {
		return(this->Print10());
	}
	else if(radix == 16) {
		return(this->Print16());
	}
	else {
		cout << "ERROR: Print(radix): Radix not supported" << endl; //printf("ERROR: Print(radix): Radix not supported\n");
		//return(0);
	}

}

string PPM::Print_NoHdr(int radix)
{

	if(radix == 2) {
		return(this->Print2_NoHdr());
	}
	else if(radix == 10) {
		return(this->Print10());
	}
	else if(radix == 16) {
		return(this->Print16_NoHdr());
	}
	else {
		cout << "ERROR: Print(radix): Radix not supported" << endl; //printf("ERROR: Print(radix): Radix not supported\n");
		//return(0);
	}

}

// prototype routine for power based moduli WITH partial power digits
// enhanced to support variable power based modulus
// to use this routine, there must exist a two's modulus, and a five's modulus
/*
string PPM::Print10p(void)
{
int val2 = 0;
int val5 = 0;
//PPM *ppmtmp = new PPM(0);
PPM *ppm = new PPM(0);
std::string s;

	ppm->AssignPM(this);		// this is a new routine for copying the power modulus structure into the new PPM type

	if(!ppm->Zero() && (ppm->NumDigits > 2)) {

//		printf("conversion to decimal: ");
		while(!ppm->Zero()) {
			
			if((ppm->Rn[Mod2_index]->Skip == 1) || (ppm->Rn[Mod5_index]->Skip == 1)) {
				ppm->ExtendPart2Norm();

//				printf("\nafter base extend2:\n");
//				ppm->PrintDemo();
			}
//			ppm->PrintDemo();

			val2 = ppm->Rn[Mod2_index]->GetPowOffset(1);
			ppm->Sub(val2);
			ppm->ModDiv(2);

//			printf("\nafter divide by 2\n");
//			ppm->PrintDemo();

			val5 = ppm->Rn[Mod5_index]->GetPowOffset(1);
			ppm->Sub(val5);
			ppm->ModDiv(5);
			
//			printf("\nafter divide by 5\n");
//			ppm->PrintDemo();

			int decval = val5 * 2 + val2;

			s.insert(0, 1, '0'+decval); 
	//		printf("%d", decval);
			
//			wait_key();
		}
	}
	else {

		s.insert(0, 1, '0'); 
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm);


	return(s);
}
*/

// this version prints binary with a single two's modulus
// the two's modulus must exist
string PPM::Print2(void)
{
//PPM *ppmtmp = new PPM(0);
PPM *ppm2 = new PPM(0);
std::string s;

	ppm2->Assign(this);
	ppm2->Normalize();

	if(ppm2->Mod2_index == INVALID_MODULUS) {
		cout << "Error:, Print2() must have a 2 modulus to print" << endl;  //printf("Error:, Print2() must have a 2 modulus to print\n");
	}
	else if(!ppm2->Zero()) {

		while(!ppm2->Zero()) {
			
			if(ppm2->Rn[Mod2_index]->Skip == 1) {
				ppm2->ExtendPart2Norm();

			}

			int val2 = ppm2->Rn[Mod2_index]->GetPowOffset(1);
			ppm2->Sub(val2);
			ppm2->ModDiv(2);

			s.insert(0, 1, '0'+val2); 
	//		printf("%d", decval);

//			wait_key();
		}

		s.insert(0, 1, ':');
		s.insert(0, 1, 'b');
	}
	else {

		s.insert(0, 1, '0'); 
		s.insert(0, 1, ':');
		s.insert(0, 1, 'b');
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm2);
//	delete(ppmtmp);

	return(s);
}

// this version prints binary with a single two's modulus
// the two's modulus must exist
string PPM::Print2_NoHdr(void)
{
//PPM *ppmtmp = new PPM(0);
PPM *ppm2 = new PPM(0);
std::string s;

	ppm2->Assign(this);
	ppm2->Normalize();

	if(ppm2->Mod2_index == INVALID_MODULUS) {
		cout << "Error:, Print2() must have a 2 modulus to print" << endl;
	}
	else if(!ppm2->Zero()) {

		while(!ppm2->Zero()) {
			
			if(ppm2->Rn[Mod2_index]->Skip == 1) {
				ppm2->ExtendPart2Norm();

			}

			int val2 = ppm2->Rn[Mod2_index]->GetPowOffset(1);
			ppm2->Sub(val2);
			ppm2->ModDiv(2);

			s.insert(0, 1, '0'+val2); 
	//		printf("%d", decval);

//			wait_key();
		}

//		s.insert(0, 1, ':');
//		s.insert(0, 1, 'b');
	}
	else {

		s.insert(0, 1, '0'); 
//		s.insert(0, 1, ':');
//		s.insert(0, 1, 'b');
	}

//	printf("complete: %s\r\n", s.c_str());

	delete(ppm2);
//	delete(ppmtmp);

	return(s);
}


// this routine converts the PPM variable to an __int64 type, but should only uses half the range of this type (positive values only).
// nothing is stopping the PPM variable from overflowing the __int64 type, causing it to wrap around, or go negative
// the user must take care that this does not happen.
__int64 PPM::Convert(void)
{

	MRN *mrn = new MRN(this);

	__int64 val = mrn->Decimal64();
	
	delete(mrn);

	return(val);

}

// this routine same as Decimal64, but uses a more "appropriate" unsigned long long return type
// nothing is stopping the PPM variable from overflowing the unsigned long long type,
// the user must take care that this does not happen, or use another conversion method.
unsigned long long PPM::uConvert(void)
{
	
	MRN *mrn = new MRN(this);

	unsigned long long val = mrn->Decimal_ull();
	
	delete(mrn);

	return(val);

}

void PPM::Increment(void)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Add(1);

	}

}


void PPM::Decrement(void)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Sub(1);

	}

}

// returns true if all non skipped digits of PPM are zero
int PPM::Zero(void)
{

	for(int i=0; i<NumDigits; i++) {

		if(Rn[i]->Digit && !Rn[i]->Skip) return(0);

	}

	return(1);		// all non skipped digits are zero, so value is zero
	
}

// returns true if all non skipped digits starting with index of PPM are zero
int PPM::Zero(int i)
{

	while(i < NumDigits) {

		if(Rn[i]->Digit && !Rn[i]->Skip) return(0);
		i += 1;

	}

	return(1);		// all non skipped digits are zero, so value is zero
	
}

// returns true if all non skipped digits of PPM are one
int PPM::One(void)
{

	for(int i=0; i<NumDigits; i++) {

		if((Rn[i]->Digit != 1) && !Rn[i]->Skip) return(0);

	}

	return(1);		// all non skipped digits are zero, so value is zero
	
}

// returns true if this > argument ppm
// compare ... is this > ppm ?   return 1 if yes, 0 if no
// this version requires values to have same "full" modulus format
int PPM::Compare(PPM *ppm)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *ppm2 = new PPM(0);
int compare_flag = 0;		// start with not greater than or equal

	ppm1->Assign(this);
	ppm2->Assign(ppm);

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
	int index = 0;
	int zero1, zero2;
	do {

		int digval1 = 0;
		int digval2 = 0;
		if((digval1=ppm1->Rn[index]->Digit) > (digval2=ppm2->Rn[index]->Digit)) {
			compare_flag = 1;
		}
		else if(digval1 != digval2) {

			compare_flag = 0;
		}

//		printf("dig1= %d, dig2= %d, compare= %d\r\n", digval1, digval2, compare_flag);
		
		ppm1->Sub(digval1);
		ppm1->ModDiv(Rn[index]->GetFullPowMod());
		ppm1->Rn[index]->SkipDigit();

		ppm2->Sub(digval2);
		ppm2->ModDiv(Rn[index]->GetFullPowMod());
		ppm2->Rn[index]->SkipDigit();

		this->counter[PPM::DEC_COUNT] += 1;
		this->counter[PPM::DIV_COUNT] += 1;

//		ppm1->Print();
//		printf("\r\n");
//		ppm2->Print();
//		printf("\r\n");

		index += 1;

		zero1 = ppm1->Zero();
		zero2 = ppm2->Zero();

	} while(!zero1 && !zero2);


	delete(ppm2);
	delete(ppm1);


	if(!zero1 && zero2) {	// ppm2 number go to zero?
		return(1);			// automatically greater than, the ppm1 number is shorter
	}
	else if(zero1 && !zero2) {	// this go to zero?
		return(0);
	}
					
	return(compare_flag);
		

}

// compare the first numdigs number of RNS digits
// assumes value is fully extended, and fully normalized
int PPM::ComparePart(PPM *ppm, int numdigs)
{
int compare_flag = 0;		// start with not greater than

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
//	int index = 0;


	if(numdigs > this->NumDigits) {
		cout << "ERROR: numdigs is > NumDigits" << endl; //printf("ERROR: numdigs is > NumDigits\n");
		return(0);
	}

	PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
	PPM *ppm2 = new PPM(0);
	
	ppm1->Assign(this);
	ppm2->Assign(ppm);

	for(int i=0; i<numdigs; i++) {     // oncly compare the first numdigs number of digits

		int digval1 = 0;
		int digval2 = 0;
		if((digval1=ppm1->Rn[i]->Digit) > (digval2=ppm2->Rn[i]->Digit)) {
			compare_flag = 1;
		}
		else if(digval1 != digval2) {

			compare_flag = 0;
		}

//		printf("dig1= %x, dig2= %x, compare= %d\r\n", digval1, digval2, compare_flag);
		
		ppm1->Sub(digval1);
		ppm1->ModDiv(Rn[i]->GetFullPowMod());
		ppm1->Rn[i]->SkipDigit();

		ppm2->Sub(digval2);
		ppm2->ModDiv(Rn[i]->GetFullPowMod());
		ppm2->Rn[i]->SkipDigit();

		this->counter[PPM::DEC_COUNT] += 1;
		this->counter[PPM::DIV_COUNT] += 1;

//		ppm1->Print();
//		printf("\r\n");
//		ppm2->Print();
//		printf("\r\n");

//		index += 1;
	} 

	delete ppm2;
	delete ppm1;
	
	return(compare_flag);
		
}

// trucate and keep first numdigs of digit and extend the result
// should work with both partial and full power modulus
void PPM::TruncateFirst(PPM *ppm, int numdigs)
{

	for(int i=0; i<NumDigits; i++) {

		if(i >= numdigs) {

			this->Rn[i]->Skip = 1;
		}

	}

	this->ExtendPart2Norm();


}

// returns true if this > argument ppm
//compare ... is ppm1 > ppm2 ?   return 1 if yes, 0 if no
int PPM::Compare(PPM *ppm, int &clocks)
{
PPM *ppm1 = new PPM(0);		// need to create the right contructor for this
PPM *ppm2 = new PPM(0);
int compare_flag = 0;		// start with not greater than
int myclocks = 0;

	ppm1->Assign(this);
	ppm2->Assign(ppm);

//	int index = this->NumDigits - 1;			// start with largest digit for optimization
	int index = 0;
	int zero1, zero2;
	do {

		int digval1 = 0;
		int digval2 = 0;
		if((digval1=ppm1->Rn[index]->Digit) > (digval2=ppm2->Rn[index]->Digit)) {
			compare_flag = 1;
		}
		else if(digval1 != digval2) {

			compare_flag = 0;
		}

//		printf("dig1= %x, dig2= %x, compare= %d\r\n", digval1, digval2, compare_flag);
		
		ppm1->Sub(digval1);
		ppm1->ModDiv(Rn[index]->GetFullPowMod());
		ppm1->Rn[index]->SkipDigit();

		ppm2->Sub(digval2);
		ppm2->ModDiv(Rn[index]->GetFullPowMod());
		ppm2->Rn[index]->SkipDigit();

//		this->counter[PPM::DEC_COUNT] += 1;		// only service this, and only count 1 set, since we have dual accumulator architecture
//		this->counter[PPM::DIV_COUNT] += 1;
//		clocks += 2;
		myclocks += 2;

//		ppm1->Print();
//		printf("\r\n");
//		ppm2->Print();
//		printf("\r\n");

//		cout << ppm1->Prints() << endl;
//		cout << ppm2->Prints() << endl;

//		wait_key();

		index += 1;

		zero1 = ppm1->Zero();
		zero2 = ppm2->Zero();

	} while(!zero1 && !zero2);

	clocks += myclocks;
//	counter[PPM::COMPARE_CLK] += clocks;
//	counter[PPM::COMPARE_COUNT] += 1;

	delete(ppm2);
	delete(ppm1);


	if(!zero1 && zero2) {	// ppm2 number go to zero?
		return(1);			// automatically greater than, the ppm1 number is shorter
	}
	else if(zero1 && !zero2) {	// this go to zero?
		return(0);
	}
					
	return(compare_flag);
		
}

// Compares the difference, which if the numbers are close, executes in less clocks
// this proposed test routine can be used to speed execution of division, for example
int PPM::CompareDif(PPM *ppm, int &clocks)
{
PPM *temp1 = new PPM(0);
PPM *temp2 = new PPM(0);

	
	temp1->Assign(this);
	temp1->Sub(ppm);	// is this just > 0, or much > 0 ? (due to wrap arround)

	temp2->Assign(ppm);
	temp2->Sub(this);			// is this just > 0, or much > 0 ?  (due to wrap around)

	clocks += 1;			// start with one from subtractions above

	if(this->IsEqual(ppm)) {
		cout << "even division, clks = " << clocks;
		return(0);
	}
	else if(!temp1->Compare(temp2, clocks)) {
		cout << "type 1 decision, clks = " << clocks;
		return(1);
	}
	else {
		cout << "type 0 decision, clks = " << clocks;
		return(0);
	}

	delete temp2;
	delete temp1;
	
}

// Is Equal looks at any unskipped digits for an equality comparison
// returns true if this is equal to ppm, else return is false
// skipped digits must occur in both 'this' and argument in same digit position, 
// otherwise, there is no way to guarantee check of equality
// in these cases, the IsEqual routine may not correctly determine if two values are equal.
int PPM::IsEqual(PPM *ppm)
{

	for(int i=0; i<NumDigits; i++) {

		if(!Rn[i]->Skip && !ppm->Rn[i]->Skip) {

			if(Rn[i]->Digit != ppm->Rn[i]->Digit) return(0);
		}

	}

	return(1);

}
// IsEqualPart is same as IsEqual but only compares the first numdigs number of digits 
int PPM::IsEqualPart(PPM *ppm, int numdigs)
{

	for (int i = 0; i<numdigs; i++) {

		if (!Rn[i]->Skip && !ppm->Rn[i]->Skip) {

			if (Rn[i]->Digit != ppm->Rn[i]->Digit) return(0);

		}

	}

	return(1);

}


// this routine assumes all the PPM classes are same format
// this Assign command preserves the modulus format of destination (defined by Power and Normal), but copies the partial value (PowerValid)
void PPM::Assign(PPM *ppm)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Skip = ppm->Rn[i]->Skip;					// this assign copies the skipped positions
		Rn[i]->PowerValid = ppm->Rn[i]->PowerValid;		// and also copies the power valid values
		Rn[i]->Digit = ppm->Rn[i]->Digit;				// and the digit values verbatum
	}

}

// used to assign a PPM valid power modulus structure to a new base PPM (this)
// by altering the Power values.  This makes the new derived number system atomic for reset functions
// this Assign command copies the modulus format to destination (defined by Power) and creates new derived modulus format
// defined by the current power of the source argument, but preserves the Normal (base) format for recovery by Normalize
void PPM::AssignPM(PPM *ppm)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Skip = ppm->Rn[i]->Skip;					// this assign copies the skipped positions
		Rn[i]->PowerValid = ppm->Rn[i]->PowerValid;		// and also copies the power valid values
		Rn[i]->Power = ppm->Rn[i]->PowerValid;			// secure the max size for use with (old) base extension
		Rn[i]->Digit = ppm->Rn[i]->Digit;				// and the digit values verbatum

	}

}

// Assigns an integer value without resetting the PPM to normal modulus format
void PPM::AssignPM(__int64 x)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Digit = x % Rn[i]->GetPowMod2();
		Rn[i]->Skip = 0;							// this assign function clears all skips
//		Rn[i]->ResetPowerValid();					// and resets the power valid counts

	}

}

// used to assign a PPM valid power modulus structure to a new base PPM (this)
// by altering the Power values.  This makes the new derived number system atomic for use with similar (but temporary) formats
// Assigns the format of ppm argument, but assigns the value of x
void PPM::AssignPM(int x, PPM *ppm)
{
	
	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Skip = ppm->Rn[i]->Skip;						// this assign copies the skipped positions of ppm arg
		Rn[i]->PowerValid = ppm->Rn[i]->PowerValid;			// and also copies the power valid values of ppm
		Rn[i]->Power = ppm->Rn[i]->PowerValid;				// and defines max power (for use with (old) base extension)
		Rn[i]->Digit = x % Rn[i]->GetPowMod2();					// BUT the digit value is derived from x arg

	}

}

// use to check that the PM modulus of this and the argument are identical
// if all modulus identical, then return 1, else return 0
int PPM::chk_PM_format(PPM *ppm)
{
int i;

	for(i=0; i<NumDigits; i++) {

		if(Rn[i]->PowerValid != ppm->Rn[i]->PowerValid) {
//			printf("PowerValid is NOT identical\n");
//			wait_key();
			return (0);
		}

		if(Rn[i]->Modulus != ppm->Rn[i]->Modulus) {       // shouldn't need to check, but will just in case

//			printf("Modulus is NOT identical\n");
//			wait_key();
			return (0);
		}

	}

	return(1);			// everything is OK

}


// Copy by value, helper function, usually for assigning a digit value
// this also works for derived power modulus variables if they are assigned using assignPM().
// NOTE - RESETS POWER BASED MODULUS TO DERIVED RNS FORMAT (Power value)
void PPM::Assign(int x)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Digit = x % Rn[i]->GetFullPowMod();
		Rn[i]->Skip = 0;							// this assign function clears all skips
		Rn[i]->ResetPowerValid();					// and resets the Power valid counts

	}

}

// NOTE - RESETS POWER BASED MODULUS to DERIVED RNS FORMAT (Power value)
void PPM::Assign(__int64 x)
{

	for(unsigned int i=0; i<Rn.size(); i++) {

		Rn[i]->Digit = x % Rn[i]->GetFullPowMod();
		Rn[i]->Skip = 0;							// this assign function clears all skips
		Rn[i]->ResetPowerValid();					// and resets the power valid counts

	}

}

// this routine assigns a value represented in an ascii string of variable length
// the string must not be signed (no - sign), and can include a decimal format (all numbers)
// or a hexadecimal format
// Resets the PPM value to that of the derived format (Power Value)
// return a '0' if string format bad, or return a '1'
int PPM::Assign(string sval)
{
int base = 10;			// assume a base of ten
int start = 0;
std::vector<char> s(sval.size()+1);

	std::copy(sval.begin(), sval.end(), s.begin());			// copy string into vector for easier processing

	this->Assign(0);				// init ppm to zero first

	if(s[0] == '+') start = 1;		// allow a single plus sign at start, no minus
	if((s[start] == '0') && (s[start+1] == 'x')) {
		base = 16;
		start += 2;
	}

	for(unsigned int i=start; i<sval.size(); i++) {		// make sure that all characters are legal

		this->Mult(base);			// each time through, multiply by the base

		if((s[i] >= '0') && (s[i] <= '9')) {
			this->Add(s[i] - '0');			
			}
		else if((s[i] >= 'a') && (s[i] <= 'f')) {
			if(base == 16) {
				this->Add((s[i]-'a')+10);
			}
			else {
//				cout << "ERROR in RNS string format, returning" << endl;
				this->Assign(0);
				return(0);
			}
		}
		else if((s[i] >= 'A') && (s[i] <= 'F')) {
			if(base == 16) {
				this->Add((s[i]-'A')+10);
			}
			else {
//				cout << "ERROR in RNS string format, returning" << endl;
				this->Assign(0);
				return(0);
			}
		}
		else {
//			cout << "ERROR in RNS string format, returning" << endl;
			this->Assign(0);
			return(0);
		}

	}

	return(1);		// return a '1' if OK

}

// Assigns a random number via a random decimal string of num_digs long
void PPM::AssignRnd(int num_digs)
{
string s;

	for(int i=0; i<num_digs; i++) {
		s += (char)(get_rand('0', '9'));
	}

	Assign(s);

//	cout << "string is: " << s << endl;

}

// This routine base extends all digits flagged as skipped
// to full modulus value, i.e. extends to full power based (Power value)
// but does not reset the power valid flags ... THIS FUNCTION MAY BE DEPRECATED IN FUTURE, USE EXTENDPARTTONORM INSTEAD
// should only be used with normal PPM or non-power based values
int PPM::ExtendNorm(void)
{
int clocks = 0;

	if(!this->IsNormal()) {
		cout << "ERROR: Attempting to extend non-normal value in ExtendNorm method" << endl;
		wait_key();
		return(0);
	}

	MRN *mrn = new MRN(this, clocks);		// convert the RNS number to its Mixed Radix digits (value and power)
	
//	cout << "mrn: " << endl; 
//	mrn->Print();
	
	Clear_Skips();
	clocks += mrn->Convert(this);

	counter[PPM::BASE_EXTEND] += 1;
	counter[PPM::EXTEND_CLK] += clocks;

	delete mrn;
	
	return(clocks);

}

// this is the most universal call, it simply calls ExtendPart2Norm()
int PPM::Extend(void)
{

	return(ExtendPart2Norm());

}

// This routine base extends all digits flagged as skipped,
// and retores digit values containing partial power digits to Normal power
// returns number of clocks
int PPM::ExtendPart2Norm(void)
{
int clocks = 0;
PPM *ppm = new PPM(0);		// need a temp init to zero

	
//	MRN *mrn = new MRN(this, clocks);		// convert the RNS number to its Mixed Radix digits (value and power)
	MRN *mrn = new MRN(ppm, clocks);

	mrn->Assign2(this);

//	cout << "mrn: " << endl; 
//	mrn->Print();
	
	Clear_Skips();
	Reset_PowerSkips();			// reset the Modulus powers to Normal powers of instance (restores to ValidPower=Power)

	clocks += mrn->Convert(this);

	counter[PPM::BASE_EXTEND] += 1;
	counter[PPM::EXTEND_CLK] += clocks;

	delete mrn;
	delete ppm;
	
	return(clocks);

}

// base extend the value, but normalize the Modulus Power values to the base instance (NormalPower)
int PPM::Normalize(void)
{
int clocks = 0;
PPM *ppm = new PPM(0);		// need a temp init to zero

	
//	MRN *mrn = new MRN(this, clocks);		// convert the RNS number to its Mixed Radix digits (value and power)
	MRN *mrn = new MRN(ppm, clocks);

	mrn->Assign2(this);

//	cout << "mrn: " << endl; 
//	mrn->Print();
	
	Clear_Skips();
	Reset_Normal();			// reset the modulus powers to NORMAL values, which is base instance normal values

	clocks += mrn->Convert(this);

	counter[PPM::BASE_EXTEND] += 1;
	counter[PPM::EXTEND_CLK] += clocks;

	delete mrn;
	delete ppm;
	
	return(clocks);

 }

// return TRUE if normal, otherwise, return false
// doesn't matter if digit is skipped or not
int PPM::IsNormal(void)
{

	for(int i=0; i<NumDigits; i++) {

		if((Rn[i]->PowerValid != Rn[i]->Power) && (!Rn[i]->Skip)) {
			return(0);		// value is not normal
		}

	}

	return(1);		// value is normal

}

// this returns the Unsigned range via ppm, and returns true if successful,
// else if request out of range, returns false.
// this value returns the digit range of the full modulus as defined by Power
int PPM::GetRange(PPM *val, int num_digits)
{

	if(num_digits > NumDigits) {
		return(0);
	}


	PPM *temp = new PPM(0);

	val->Assign(1);
	for(int i=0; i<num_digits; i++) {

		temp->Assign(Rn[i]->GetFullPowMod());
		val->Mult(temp);

	}

	delete temp;
	return(1);

}

// supports both partial and full power value maximum range calculations
void PPM::GetFullRange(PPM *ppm)
{

	ppm->AssignPM((__int64)0);

	ppm->Decrement();		// decrement from zero, and get the max value


}

// return the multiplicative range value
void PPM::GetMultRange(PPM *val)
{

	if(Mult_range == NULL) {
		cout << "calculating range" << endl;
		val->GetRange(val, UNSIGNED_RANGE);
		cout << "val = " <<  val->Print10() << endl;
		
		val->Sqrt();
		Mult_range = new PPM(0);	// allocate the range once !
		Mult_range->Assign(val);
	
		cout << val->Print10() << " range calculated" << endl; 
	}
	else {
		val->Assign(Mult_range);
	}

}

// Computes this = n! in PPM integer format.
// Stays entirely in the RNS-APAL / PPM integer domain.
void PPM::Factorial(int n)
{
	this->Assign(1);

	for (int i = 2; i <= n; i++) {
		this->Mult(i);
	}
}

// adds any integer x, usually used to add digit values up to largest PPM digit modulus
void PPM::Add(int x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Add(x % Rn[i]->GetPowMod2());		// add x to the digit, MOD digit modulus

	}

}

// adds any integer x, used for initialization of larger values
void PPM::Add(__int64 x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Add(x % Rn[i]->GetPowMod2());		// add x to the digit, MOD digit modulus

	}

}

// adds any integer x, but usually used to add digit values up to largest PPM digit modulus
// now translates skipped digits to the base
// now supports error detection for mismatched types
void PPM::Add(PPM *ppm)
{

	for(int i=0; i<NumDigits; i++) {

		if(ppm->Rn[i]->Skip) {
			Rn[i]->Skip = 1;			// if source is skipped, then make destination skipped
		}
//		else if(ppm->Rn[i]->PowerValid < Rn[i]->PowerValid) {     // cannot allow the additive argument to have a lower power modulus then base argument
																  // (this represents missing infomration for that digit with respect to base argument)
		else if(ppm->Rn[i]->PowerValid != Rn[i]->PowerValid) {    // now supporting a stricter policy to ensure the addition is only of same modulus types 
																 
			cout << "ERROR: PPM::Add - attempting to add incompatible types" << endl;
			wait_key();
		}

		Rn[i]->Add(ppm->Rn[i]->Digit);		// add ppm[i] to the digit,
		
	}

}

// subtracts any integer x, but usually used to add digit values up to largest PPM digit modulus
void PPM::Sub(int x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Sub(x % Rn[i]->GetPowMod2());		// add x to the digit, MOD digit modulus

	}

}

// subtracts any integer x, but used to subtract large native c++ integer values
void PPM::Sub(__int64 x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Sub(x % Rn[i]->GetPowMod2());		// add x to the digit, MOD digit modulus

	}

}

// subtracts any integer x, but usually used to add digit values up to largest PPM digit modulus
// now translates skipped digits to the base
// now supports error detection for mismatched types
void PPM::Sub(PPM *ppm)
{

	for(int i=0; i<NumDigits; i++) {

		if(ppm->Rn[i]->Skip) {
			Rn[i]->Skip = 1;			// if source is skipped, then make destination skipped
		}
//		else if(ppm->Rn[i]->PowerValid < Rn[i]->PowerValid) {     // cannot allow the subtractive argument to have a lower power modulus then base argument
																  // (this represents missing infomration for that digit with respect to base argument)
		else if(ppm->Rn[i]->PowerValid != Rn[i]->PowerValid) {    // now supporting a stricter policy regarding only subtracting types with same PowerValid modulus structure   
																
			cout << "Error PPM::Sub - attempting to subtract incompatible types at modulus " << i << endl;
			wait_key();
		}

		Rn[i]->Sub(ppm->Rn[i]->Digit);		// Sub ppm, digit by digit, MOD digit modulus

	}

}

// Multiplies any integer x, but usually used to add digit values up to largest PPM digit modulus
// disregards skipped positions
void PPM::Mult(int x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Mult(x % Rn[i]->GetPowMod2());		// Mult x to the digit, MOD digit modulus

	}

}

// Multiplies any integer x, but used to initialize larger values from c++ integer values
// disregards skipped positions
void PPM::Mult(__int64 x)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Mult(x % Rn[i]->GetPowMod2());		// Mult x to the digit, MOD digit modulus

	}

}

// Multiplies any integer x, but usually used to add digit values up to largest PPM digit modulus
// passes on skipped digits
// checks the validity of the modulus format.
void PPM::Mult(PPM *ppm)
{

	for(int i=0; i<NumDigits; i++) {

		if(ppm->Rn[i]->Skip) {
			Rn[i]->Skip = 1;			// if source is skipped, then make destination skipped
		}
//		else if(ppm->Rn[i]->PowerValid < Rn[i]->PowerValid) {     // cannot allow the multiplicative argument to have a lower power modulus then base argument
																  // (this represents missing information for that digit with respect to base argument)
		else if(ppm->Rn[i]->PowerValid != Rn[i]->PowerValid) {	  // now supporting a stricter policy of only multiplying same types (having same PowerValid settings).
			cout << "ERROR: PPM::Mult - attempting to multiply incompatible types" << endl;

			wait_key();
		}

		Rn[i]->Mult(ppm->Rn[i]->Digit);		// Mult ppm to the digit, MOD digit modulus

	}

}


// Divides the RN number by a number equal to an RN Modulus
// Only valid if divisor is equal to modulus of a zero digit
// have not put in any power check on modulus yet, thinking they should be the same
void PPM::ModDiv(PPM *ppm)
{

	for(int i=0; i<NumDigits; i++) {

		if(!Rn[i]->Skip) {

//			Rn[i]->Digit = ModTable::arrayDivTbl[i][Rn[i]->Digit][ppm->Rn[i]->Digit];		// LUT used to calculate this/ppm  (old way)
			Rn[i]->Digit = Rn[i]->Div(ppm->Rn[i]->Digit);
		
		}
	
	}

}

// Divides the RN number by x, which is usually  digit value
// Only valid if this is evenly divisiblble by divisor
// ModDiv and ModDiv2 should be equivalent if and only if when PowerValid goes to zero, the skip digit is guaranteed to be set
void PPM::ModDiv(int digval)
{

	for(int i=0; i<NumDigits; i++) {

		if(!Rn[i]->Skip) {

//			Rn[i]->Digit = ModTable::arrayDivTbl[i][Rn[i]->Digit][digval % Rn[i]->GetPowMod()];		// LUT used to calculate this/digval
			Rn[i]->Digit = Rn[i]->Div(digval % Rn[i]->GetPowMod2());
		
		}
	}

}


// for testing of new efficient LUT method
void PPM::ModDiv2(int digval)
{

	for(int i=0; i<NumDigits; i++) {

		if(!Rn[i]->Skip) {

//			Rn[i]->Digit = ModTable::arrayDivTbl[i][Rn[i]->Digit][digval % Rn[i]->GetPowMod()];		// LUT used to calculate this/digval
			Rn[i]->Digit = Rn[i]->Div2(digval % Rn[i]->GetPowMod2());
		
		}
	}

}



// Divides the RN number by x, which is usually  digit value
// Only valid if this is evenly divisiblble by divisor
/*
void PPM::ModDivNZ(int digval)
{
	PPM *ppm = new PPM(0);
	ppm->Assign(digval);

	for(int i=0; i<NumDigits; i++) {

		if(Rn[i]->Digit) {

			Rn[i]->Digit = ModTable::arrayDivTbl[i][Rn[i]->Digit][ppm->Rn[i]->Digit];		// LUT used to calculate this/digval
		
		}
	
	}

}
*/

// return true if the PPM is divisible by any of the first n PM digit modulus
int PPM::Divisible_by_n_pmd(int index)
{
int i = 0;
	
	while(i<NumDigits) {

		if(i < index) {
			if(!Rn[i]->Digit) return(1);
		}
		else {
			break;
		}

		i++;
	}

	return(0);
}

// return True if the PPM is divisible, or power digits that are partially divisible
int PPM::Divisible_by_n_pmd_PM(int index)
{
int i = 0;
int pwr;
	
	while(i<NumDigits) {

		if(i < index) {
//			if(!Rn[i]->Digit) return(1);
			if((Rn[i]->IsZero(pwr)) && (Rn[i]->PowerValid != 0)) return(1);		// new version 11/13/2016, now checks if powervalid is not zero (could check skip)
//			if(Rn[i]->IsZero(pwr)) return(1);				// Old version

		}
		else {
			break;
		}

		i++;
	}
	

	return(0);
}

// return True if the PPM is divisible, or power digits that are partially divisible
// this version must check that powervalid is in range
int PPM::Divisible_by_n_pmd_PM2(int index)
{
int i = 0;
int pwr;
	
	while(i<NumDigits) {

		if(i < index) {
//			if(!Rn[i]->Digit) return(1);
			if(Rn[i]->IsZero(pwr)) return(1);

		}
		else {
			break;
		}

		i++;
	}
	

	return(0);
}

// Decrement this by the next zero factor in ppm, starting with prime_index, and searching to end_index, and returning the next prime_index and the dig_val
// Looks like skip digit is not tracked here, since the routine assumes none will be present between start and end index.
int PPM::Dec_by_next_fact(PPM *ppm, int &dig_val, int &start_index, int end_index)
{

	int msd = dig_val = ModTable::primes[end_index];		// get the largest digit plus one (largest modulus)

	for(int i=start_index; i<end_index;	i++) {

		if(!ppm->Rn[i]->Digit) {
			start_index = i;
			dig_val = Rn[i]->Digit;		// get numerator digit
			break;
		}

	}

	if(dig_val == msd) {
		return(0);			// no factors present in this
	}
	else {
		this->Sub(dig_val);	// decrement this by the selected digit position
	}

	return(1);

}

// this version modified to support Power PM digits
int PPM::Dec_by_next_fact_PM(PPM *ppm, int &dig_val, int &start_index, int &pwr, int end_index)
{
//int pwr;

	int msd = dig_val = ModTable::primes[end_index];		// get the largest digit plus one (largest modulus)	
															// I assume primes array goes this far?
	for(int i=start_index; i<end_index;	i++) {

		if(ppm->Rn[i]->IsZeroPM(pwr)) {
			start_index = i;
			dig_val = this->Rn[i]->GetPowOffset(pwr);
			break;
		}

//		if(!ppm->Rn[i]->Digit) {
//			start_index = i;
//			dig_val = Rn[i]->Digit;		// get numerator digit
//			break;
//		}

	}

	if(dig_val == msd) {
		return(0);			// no factors present in this
	}
	else {
		this->Sub(dig_val);	// decrement this by the selected digit position
//		printf("dividend subtracted by: %d\n", dig_val);
	}

	return(1);

}

// this version modified to support Power PM digits
int PPM::Dec_by_next_fact_PM_1pwr(PPM *ppm, int &dig_val, int &start_index, int &pwr, int end_index)
{
//int pwr;

	int msd = dig_val = ModTable::primes[end_index];		// get the largest digit plus one (largest modulus)	
															// I assume primes array goes this far?
	for(int i=start_index; i<end_index;	i++) {

		if(ppm->Rn[i]->IsZeroPM_1pwr(pwr)) {
			start_index = i;
			dig_val = this->Rn[i]->GetPowOffset(pwr);
			break;
		}

//		if(!ppm->Rn[i]->Digit) {
//			start_index = i;
//			dig_val = Rn[i]->Digit;		// get numerator digit
//			break;
//		}

	}

	if(dig_val == msd) {
		return(0);			// no factors present in this
	}
	else {
		this->Sub(dig_val);	// decrement this by the selected digit position
	}

	return(1);

}

int PPM::AnySkips(void)
{
	
	for(int i=0; i<NumDigits; i++) {

		if(Rn[i]->Skip) return(1);

	}

	return(0);

}

int PPM::AnyPartSkips(void)
{
	
	for(int i=0; i<NumDigits; i++) {

//		if(Rn[i]->Skip) return(1);
		if(Rn[i]->PowerValid != Rn[i]->Power) return(1);

	}

	return(0);

}


// used in teh CFR procedure below to detect any zero digits in the word
// does not process skipped digits as zeros, but ignores skipped digits
int PPM::AnyZero(void)
{
int i, power;

	for(i=0; i<NumDigits; i++) {

		if(!(Rn[i]->Skip)) {

			if(Rn[i]->IsZeroPM(power)) {
				return(1);
			}
		}

	}

	return(0);


}

// CFR reduction test routine for testing of the procedure and characterization
// this version will work on natural residue system first
void PPM::Cfr(PPM *d, cfr_data *stats)
{
int i, power;

//	cout << "starting value: " << endl;
//	d->PrintDemo();
//	wait_key();


	while(!d->Zero()) {

//		d->PrintDemo();
//		wait_key();

		while(AnyZero()) {
		// check for zero digit
			for(i=0; i<NumDigits; i++) {

				if(d->Rn[i]->IsZeroPM(power)) {

//					printf("%d ", Rn[i]->Modulus);
					d->ModDiv(Rn[i]->Modulus);

					stats->num_moddivs++;

				}

			}

			d->ExtendPart2Norm();
			stats->num_extends++;
		}

//		printf("\n");
//		wait_key();

//		printf("subtracting one\n");
		d->Sub(1);
		stats->num_decs++;

//		wait_key();
//		d->Extend2();			// don't need if we're going after repeated fators


	}

	stats->num_decs--;		// last dec not counted
//	wait_key();


}

int PPM::DivStd(PPM *divisor, PPM *rem)
{

//	DivPM_Rez9A(divisor, rem);		// this version is for testing Rez9 hardware  (the modulus needs to be set same as Rez-9)

//	DivPM3(divisor, rem);			// this version uses original decrement of divisor, needs at least one redundant digit	
//	DivPM4(divisor, rem);			// this version uses original decrement of divisor method, similat tor DivPM3 includes debugging commands

//	return(DivPM5T(divisor, rem));		// use this if you want to debug or see inside the integer DivPM5 divide routine!
//	return(DivPM5(divisor, rem));		// USE THIS VERSION, CURRENTLY MOST STABLE
//	return(DivPM6(divisor, rem));		// FOR TESTING OF ELIMINATION OF COMPARE ON EACH ITERATION
	return(DivPM7(divisor, rem));		// FOR TESTING OF ELIMINATION OF COMPARE ON EACH ITERATION AND REDUCTION OF BASE EXTENSIONS

	return(0);
}


// divide using VARIABLE power based modulus... testing & development
// supports power based divide, but does not track subdigit skips (don't cares)
// based on DivPM()
void PPM::DivPM3(PPM *ppm, PPM *rm)
{
int sign = 1;			// sign value to inverse additions
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
		cout << "ERROR: DIVIDE BY ERROR!" << endl;
		wait_key();
		div_done = 1;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {						

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << " ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;


//					Dividend->ModDiv(ModTable::primes[prime_index]);			// perform modulo digit division
//					Divisor->ModDiv(ModTable::primes[prime_index]);
					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell

					Divisor->Rn[prime_index]->Skip = 1;			// TEST: MODDIV should do this!  flag this digit as needing base extension
					Dividend->Rn[prime_index]->Skip = 1;	
					
//					if(this->ena_dplytrc) printf("%d[]:", ModTable::primes[prime_index]);
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnySkips()) {				// if any pending skip pos set, then perform base extension

						numcnt = Dividend->ExtendNorm();
						Divisor->ExtendNorm();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

						Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
						Dividend->Clear_Skips();

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnySkips()) {
				numcnt = Dividend->ExtendNorm();
				Divisor->ExtendNorm();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

				Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
				Dividend->Clear_Skips();
			}

//			cout << "\r\nDividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;
//			cout << "Divisor:  " << Divisor->Prints() << " = " << Divisor->Print10() << endl;
				
			if(this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";			// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}



				if(sign > 0) {				// process the accumulator with the latest dividend value, observing the polarity, either add or subtract
					acc->Add(Dividend);			// acc += result
				}
				else {
					acc->Sub(Dividend);			// acc -= result
				}

//				cout << "acc: " << acc->Print10() << endl;

				sign *= -1;						// change accumulate sign (toggle flag)!
				counter[COUNTERS::ADD_COUNT] += 1;


				dif->Assign(DivisorOrig);			// dif = original divisor 
				dif->Mult(Dividend);				// dif = original divisor * current working dividend (temp quotient)
				counter[COUNTERS::MULT_COUNT] += 1;

				temp->Assign(DividendCopy);			// temp = copy of dividend at start of iternation					
				temp->Add(DivisorOrig);			    // add original divisor since comparison is: if ((quotient_result * divisor) < (divisor + dividend_copy)) 
													//									same as: if(((quotient_result * divisor) - dividend_copy) < divisor)
				counter[COUNTERS::ADD_COUNT] += 1;

				// Note: Instead of comparing the results of last iteration to its expected target, 
				// could we instead always go back to check error against final result instead?
				// (the first time through, we are checking against the final result anyways)
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

//				printf("\r\n");
//				cout << "dif= " << dif->Print10() << "  temp= " << temp->Print10() << endl;
				if(dif->Compare(temp, clocks)) {		// if dif > temp then .....
					

					dif->Sub(DividendCopy);			// subtract DividendCopy from dif (creates the real difference)		
					counter[COUNTERS::SUB_COUNT] += 1;

					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//					printf("dif = %I64d, dif %.*f percent of DividendCopy", dif->decimal3(), 5, (float) dif->decimal3()/(float) DividendCopy->decimal3()*100.0);
//					printf("\r\n");

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

//					cout << "Dividend: " << Dividend->Print10() << endl;

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
				else {							// division is now complete ....
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					if(dif->IsEqual(temp)) {	// added this check, in case dif is EQUAL to temp
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						// This is the poorly understood accum adjustment part
												// Need to study this!  - it's an error measure of the final division
						dif->Assign(acc);				// get the accum final accumulator (quotient) value
						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
//						clocks = 0;
						
//						printf("\r\n");
//						cout << "dif= " << dif->Print10() << "  Dividend= " << DividendOrig->Print10() << endl;
						
						if(dif->Compare(DividendOrig, clocks)) {	// see if quotient is high by one, if so, adjust ...
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
							acc->Decrement();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//							printf(" no adjust ");
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

//					printf("the acc: %s\r\n", acc->Print10().c_str());

					// ERROR DETECTION ROUTINE
//					if((acc->Convert() != (DividendOrig->Convert() / DivisorOrig->Convert())) || (rm->Convert() != (DividendOrig->Convert() % DivisorOrig->Convert()))) {
//						printf("\r\n error detected in divide7! \r\n");
//						wait_key();
////						while(1);
//					}	
					
					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;
//				dif->Clear_Counters();
			}
			else {

//				dcnt = Dividend->dec_by_next_fact2(Divisor);		
					
//				if(dcnt && this->ena_dplytrc) printf("-%dn ", dcnt);	// if there is a factor, print the offset, else skip this print
					
			}
		}		// end if(pm2.divisible_by_pm())
		else {
			Divisor->Decrement();
			if (this->ena_dplytrc) cout << "-d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

}


// divide using VARIABLE power based modulus... testing & development
// supports power based divide, but does not track subdigit skips (don't cares)
// based on DivPM()
// This was a debugging session started on 4/10/2016.  Problem is that all the divides are requiring too many redundant digits
// This has led to a lot of instability
void PPM::DivPM4(PPM *ppm, PPM *rm)
{
int sign = 1;			// sign value to inverse additions
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
		cout << "ERROR: DIVIDE BY ERROR!" << endl;
		wait_key();
		div_done = 1;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	cout << endl << " At start: " << endl;
	Dividend->PrintDemo();
	cout << endl;
	Divisor->PrintDemo();
	cout << endl;
	cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
	wait_key();

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {			// subtracts the dividend by enough to make divisble by mod^pwr			

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;

					cout << "before divide: " << endl;
					Dividend->PrintDemo();
					cout << endl;
					Divisor->PrintDemo();
					cout << endl;
					cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
					wait_key();

					cout << "divided by: " << power(ModTable::primes[prime_index], pwr);
					cout << endl;

					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell

//					Divisor->Rn[prime_index]->Skip = 1;			// TEST: MODDIV should do this!  flag this digit as needing base extension
//					Dividend->Rn[prime_index]->Skip = 1;	

					Dividend->PrintDemo();
					cout << endl;
					Divisor->PrintDemo();
					cout << endl;
					cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
					wait_key();
					
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnyPartSkips()) {				// if any pending full skips or partial skips, then perform base extension

						cout << "extend 1: "<< endl;
						numcnt = Dividend->ExtendPart2Norm();
						Divisor->ExtendPart2Norm();

						Dividend->PrintDemo();
						cout << endl;
						Divisor->PrintDemo();
						cout << endl;
						cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
						wait_key();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

						Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
						Dividend->Clear_Skips();

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnyPartSkips()) {
				cout << "extend 2: " << endl;
				numcnt = Dividend->ExtendPart2Norm();
				Divisor->ExtendPart2Norm();

				Dividend->PrintDemo();
				cout << endl;
				Divisor->PrintDemo();
				cout << endl;
				cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
				wait_key();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

				Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
				Dividend->Clear_Skips();
			}

//			cout << "\r\nDividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;
//			cout << "Divisor:  " << Divisor->Prints() << " = " << Divisor->Print10() << endl;
				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				cout << "divisor is one, or dividend is zero" << endl;
				wait_key();

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";		// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}



				if(sign > 0) {				// process the accumulator with the latest dividend value, observing the polarity, either add or subtract
					acc->Add(Dividend);			// acc += result
					cout << "dividend = " << Dividend->Print10() << " ADDED to acc = " << acc->Print10() << endl;
				}
				else {
					acc->Sub(Dividend);			// acc -= result
					cout << "dividend = " << Dividend->Print10() << " SUBTRACTED from acc = " << acc->Print10() << endl;
				}

//				cout << "acc: " << acc->Print10() << endl;

				sign *= -1;						// change accumulate sign (toggle flag)!
				counter[COUNTERS::ADD_COUNT] += 1;

				cout << "DivisorOrig: " << DivisorOrig->Print10() << ", Dividend: " << Dividend->Print10() << ", DividendCopy: " << DividendCopy->Print10() << endl;
				wait_key();

				dif->Assign(DivisorOrig);			// dif = original divisor 
				dif->Mult(Dividend);				// dif = original divisor * current working dividend (temp quotient)
				counter[COUNTERS::MULT_COUNT] += 1;

				temp->Assign(DividendCopy);			// temp = copy of dividend at start of iternation					
				temp->Add(DivisorOrig);			    // add original divisor since comparison is: if ((quotient_result * divisor) < (divisor + dividend_copy)) 
													//									same as: if(((quotient_result * divisor) - dividend_copy) < divisor)
				counter[COUNTERS::ADD_COUNT] += 1;

				// Note: Instead of comparing the results of last iteration to its expected target, 
				// could we instead always go back to check error against final result instead?
				// (the first time through, we are checking against the final result anyways)
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

//				printf("\r\n");
				cout << "dif= " << dif->Print10() << "  temp= " << temp->Print10() << endl;
				wait_key();

				if(dif->Compare(temp, clocks)) {		// if dif > temp then .....
					

					dif->Sub(DividendCopy);			// subtract DividendCopy from dif (creates the real difference)		
					counter[COUNTERS::SUB_COUNT] += 1;

					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//					printf("dif = %I64d, dif %.*f percent of DividendCopy", dif->decimal3(), 5, (float) dif->decimal3()/(float) DividendCopy->decimal3()*100.0);
//					printf("\r\n");

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

//					cout << "Dividend: " << Dividend->Print10() << endl;

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
				else {							// division is now complete ....
					if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					if(dif->IsEqual(temp)) {	// added this check, in case dif is EQUAL to temp
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						// This is the poorly understood accum adjustment part
												// Need to study this!  - it's an error measure of the final division
						dif->Assign(acc);				// get the accum final accumulator (quotient) value
						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
//						clocks = 0;
						
//						printf("\r\n");
//						cout << "dif= " << dif->Print10() << "  Dividend= " << DividendOrig->Print10() << endl;
						
						if(dif->Compare(DividendOrig, clocks)) {	// see if quotient is high by one, if so, adjust ...
							if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
							acc->Decrement();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//							printf(" no adjust ");
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

//					printf("the acc: %s\r\n", acc->Print10().c_str());

					// ERROR DETECTION ROUTINE
//					if((acc->Convert() != (DividendOrig->Convert() / DivisorOrig->Convert())) || (rm->Convert() != (DividendOrig->Convert() % DivisorOrig->Convert()))) {
//						printf("\r\n error detected in divide7! \r\n");
//						wait_key();
////						while(1);
//					}	
					
					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;
//				dif->Clear_Counters();
			}
			else {

//				dcnt = Dividend->dec_by_next_fact2(Divisor);		
					
//				if(dcnt && this->ena_dplytrc) printf("-%dn ", dcnt);	// if there is a factor, print the offset, else skip this print
					
			}
		}		// end if(pm2.divisible_by_pm())
		else {

			Divisor->Decrement();
			
			cout << "divisor decremented by 1: " << endl;
			Dividend->PrintDemo();
			cout << endl;
			Divisor->PrintDemo();
			cout << endl;
			cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
			wait_key();
			
			if (this->ena_dplytrc) cout << "-d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

}

// THIS IS A NEW METHOD, USING INCREMENT OF DIVISOR, NOT DECREMENT
// BENEFIT OF METHOD IS THAT IT PROVIDES A FULL RANGE OF DIVIDE WITHOUT REDUNDANT MODULUS
// divide using VARIABLE power based modulus... testing & development
// supports power based divide, but does not track subdigit skips (don't cares)
// based on DivPM5(), THIS IS TEST VERSION, PLEASE KEEP THIS and T VERSION IN SYNC !!!
// future advancements of changing the increment to decrement and back seem probable
// as well as delay of base extension, likley key off 2's modulus for that (as long as it is valid, keep on trucking!)
int PPM::DivPM5(PPM *ppm, PPM *rm)
{
//int sign = 1;			// sign value to inverse additions (not used in thie version at this time)
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;
int error_code = 0;		// start with no error
int zero;


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
//	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

//	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
//	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
//		printf("ERROR: DIVIDE BY ZERO ERROR!\r\n");
//		wait_key();
		div_done = 1;
		error_code = DIVIDE_BY_ZERO;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	if(Dividend->Zero()) {     // I guess we need it, since the routine does not work with Dividend = zero
		rm->Assign(0);
		div_done = 1;
	}

	if(this->IsEqual(Divisor)) {     // This check not needed, but makes the division of same number faster
		this->Assign((__int64)1);
		rm->Assign(0);
		div_done = 1;
	}

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {			// subtracts the dividend by enough to make divisble by mod^pwr			

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;

					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell
					
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnyPartSkips()) {				// if any pending full skips or partial skips, then perform base extension

						numcnt = Dividend->ExtendPart2Norm();
						Divisor->ExtendPart2Norm();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnyPartSkips()) {

				numcnt = Dividend->ExtendPart2Norm();
				Divisor->ExtendPart2Norm();

				if(this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

			}

				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";		// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}

				acc->Add(Dividend);			// acc += result
				counter[COUNTERS::ADD_COUNT] += 1;

//				sign *= -1;						// change accumulate sign (toggle flag)! NO LONGER NEEDED
				
				dif->Assign(DividendCopy);

				temp->Assign(DivisorOrig);
				temp->Mult(Dividend);
				counter[COUNTERS::MULT_COUNT] += 1;
				
				dif->Sub(temp);
				counter[COUNTERS::SUB_COUNT] += 1;
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

				if(!Dividend->Zero() && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {		// if dif >= DivisorOrig then .....
					
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
				else {							// division is now complete ....
					if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl; 
												// This is the post accum adjustment part
					if(dif->IsEqual(temp)) {	// added this check, in case dif is EQUAL to temp
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						
												// Need to study this!  - it's an error measure of the final division
//						dif->Assign(acc);				// get the accum final accumulator (quotient) value
//						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
//						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
						
						if((zero=Dividend->Zero()) && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {	// see if divdend went to zero, and dif >= DivisorOrig
							if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl; 
							acc->Increment();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

					// ERROR DETECTION ROUTINE
					if(!DivCheck(DividendOrig, DivisorOrig, acc, rm)) {
						cout << "ERROR in INTEGER DIVIDE !!! - DIVPM5" << endl;
						cout << "DivisorOrig: " << DivisorOrig->Print10() << " DividendOrig: " << DividendOrig->Print10() << endl;
						wait_key();
						error_code = INTERNAL_ERROR;
					}

					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;

			}
			else {


					
			}
		}		// end if(pm2.divisible_by_pm())
		else {

			Divisor->Increment();
			
			if (this->ena_dplytrc) cout << "+d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

	return(error_code);

}

// THIS IS A NEW METHOD, USING INCREMENT OF DIVISOR, NOT DECREMENT
// BENEFIT OF METHOD IS THAT IT PROVIDES A FULL RANGE OF DIVIDE WITHOUT REDUNDANT MODULUS
// divide using VARIABLE power based modulus... testing & development
// supports power based divide, but does not track subdigit skips (don't cares)
// based on DivPM5(), THIS IS TEST VERSION, PLEASE KEEP BOTH IN SYNC !!!
int PPM::DivPM5T(PPM *ppm, PPM *rm)
{
int sign = 1;			// sign value to inverse additions
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;
int error_code = 0;		// start with no error


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

//	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
//	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
		cout << "ERROR: DIVIDE BY ZERO ERROR!" << endl;
		error_code = DIVIDE_BY_ZERO;
		wait_key();
		div_done = 1;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	if(this->IsEqual(Divisor)) {     // check not needed, but makes same number division faster
		this->Assign((__int64)1);
		rm->Assign(0);
		div_done = 1;
	}


	cout << endl << " At start: " << endl;
	Dividend->PrintDemo();
	cout << endl;
	Divisor->PrintDemo();
	cout << endl;
	cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
	wait_key();

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {			// subtracts the dividend by enough to make divisble by mod^pwr			

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;

					cout << "before divide: " << endl;
					Dividend->PrintDemo();
					cout << endl;
					Divisor->PrintDemo();
					cout << endl;
					cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
					wait_key();

					cout << "divided by: " << power(ModTable::primes[prime_index], pwr);
					cout << endl;

					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell

//					Divisor->Rn[prime_index]->Skip = 1;			// TEST: MODDIV should do this!  flag this digit as needing base extension
//					Dividend->Rn[prime_index]->Skip = 1;	

					Dividend->PrintDemo();
					cout << endl;
					Divisor->PrintDemo();
					cout << endl;
					cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
					wait_key();
					
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnyPartSkips()) {				// if any pending full skips or partial skips, then perform base extension

						cout << "extend 1: " << endl;
						numcnt = Dividend->ExtendPart2Norm();
						Divisor->ExtendPart2Norm();

						Dividend->PrintDemo();
						cout << endl;
						Divisor->PrintDemo();
						cout << endl;
						cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
						wait_key();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

						Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
						Dividend->Clear_Skips();

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnyPartSkips()) {
				cout << "extend 2: " << endl;
				numcnt = Dividend->ExtendPart2Norm();
				Divisor->ExtendPart2Norm();

				Dividend->PrintDemo();
				cout << endl;
				Divisor->PrintDemo();
				cout << endl;
				cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
				wait_key();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

				Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
				Dividend->Clear_Skips();
			}

//			cout << "\r\nDividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;
//			cout << "Divisor:  " << Divisor->Prints() << " = " << Divisor->Print10() << endl;
				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				cout << "divisor is one, or dividend is zero" << endl;
				wait_key();

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";		// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}



//				if(sign > 0) {				// process the accumulator with the latest dividend value, observing the polarity, either add or subtract
					acc->Add(Dividend);			// acc += result
					cout << "dividend = " << Dividend->Print10() << " ADDED to acc = " << acc->Print10() << endl;
//				}
//				else {
//					acc->Sub(Dividend);			// acc -= result
//					cout << "dividend = " << Dividend->Print10() << " SUBTRACTED from acc = " << acc->Print10() << endl;
//				}

//				cout << "acc: " << acc->Print10() << endl;

				sign *= -1;						// change accumulate sign (toggle flag)!
				counter[COUNTERS::ADD_COUNT] += 1;

				cout << "DivisorOrig: " << DivisorOrig->Print10() << ", Dividend: " << Dividend->Print10() << ", DividendCopy: " << DividendCopy->Print10() << endl;
				wait_key();

//				dif->Assign(DivisorOrig);			// dif = original divisor 
//				dif->Mult(Dividend);				// dif = original divisor * current working dividend (temp quotient)

				dif->Assign(DividendCopy);

				temp->Assign(DivisorOrig);
				temp->Mult(Dividend);

				dif->Sub(temp);
				counter[COUNTERS::MULT_COUNT] += 1;

//				temp->Assign(DividendCopy);			// temp = copy of dividend at start of iternation					
//				temp->Add(DivisorOrig);			    // add original divisor since comparison is: if ((quotient_result * divisor) < (divisor + dividend_copy)) 
													//									same as: if(((quotient_result * divisor) - dividend_copy) < divisor)
				counter[COUNTERS::ADD_COUNT] += 1;

				// Note: Instead of comparing the results of last iteration to its expected target, 
				// could we instead always go back to check error against final result instead?
				// (the first time through, we are checking against the final result anyways)
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

//				printf("\r\n");
				cout << "dif= " << dif->Print10() << "  temp= " << temp->Print10() << endl;
				wait_key();

////				if(!Dividend->Zero() && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {		// if dif >= DivisorOrig then .....
				// performing an interesting test to see if we can eliminate the iteration compare, presumably at the cost of extra iteration.
				if(!Dividend->Zero()) {		// if dif >= DivisorOrig then .....					

//					dif->Sub(DividendCopy);			// subtract DividendCopy from dif (creates the real difference)		
					counter[COUNTERS::SUB_COUNT] += 1;

					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//					printf("dif = %I64d, dif %.*f percent of DividendCopy", dif->decimal3(), 5, (float) dif->decimal3()/(float) DividendCopy->decimal3()*100.0);
//					printf("\r\n");

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

//					cout << "Dividend: " << Dividend->Print10() << endl;

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
				else {							// division is now complete ....
					if(this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					if(dif->IsEqual(temp)) {	// added this check, in case dif is EQUAL to temp
//					if(dif->IsEqual(DivisorOrig)) {
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
						cout << "Post increment when dif equals temp" << endl;
					}
					else {						// This is the poorly understood accum adjustment part
												// Need to study this!  - it's an error measure of the final division
//						dif->Assign(acc);				// get the accum final accumulator (quotient) value
//						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
//						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
//						clocks = 0;
						
//						printf("\r\n");
//						cout << "dif= " << dif->Print10() << "  Dividend= " << DividendOrig->Print10() << endl;
						
//						if(dif->Compare(DividendOrig, clocks)) {	// see if quotient is high by one, if so, adjust ...
						if((Dividend->Zero()) && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
							acc->Increment();
							counter[COUNTERS::DEC_COUNT] += 1;
							cout << "Post increment with Dividend = 0" << endl;
						}
						else {
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//							printf(" no adjust ");
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

//					printf("the acc: %s\r\n", acc->Print10().c_str());

					// ERROR DETECTION ROUTINE
					if(!DivCheck(DividendOrig, DivisorOrig, acc, rm)) {
						cout << "ERROR in INTEGER DIVIDE !!!" << endl;
						error_code = INTERNAL_ERROR;
						wait_key();
					}

					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;
//				dif->Clear_Counters();
			}
			else {

//				dcnt = Dividend->dec_by_next_fact2(Divisor);		
					
//				if(dcnt && this->ena_dplytrc) printf("-%dn ", dcnt);	// if there is a factor, print the offset, else skip this print
					
			}
		}		// end if(pm2.divisible_by_pm())
		else {

//			Divisor->Decrement();
			Divisor->Increment();

			cout << "divisor Incremented by 1: " << endl;
			Dividend->PrintDemo();
			cout << endl;
			Divisor->PrintDemo();
			cout << endl;
			cout << "Dividend: " << Dividend->Print10() << ", Divisor: " << Divisor->Print10() << endl;
			wait_key();
			
			if (this->ena_dplytrc) cout << "+d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);

	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

	return(error_code);
}

int PPM::DivPM6(PPM *ppm, PPM *rm)
{
//int sign = 1;			// sign value to inverse additions (not used in thie version at this time)
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;
int error_code = 0;		// start with no error
int zero;
int first_zero = 0;		// flag to indicate the dividend has gone to zero at least once


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
//	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

//	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
//	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
//		printf("ERROR: DIVIDE BY ZERO ERROR!\r\n");
//		wait_key();
		div_done = 1;
		error_code = DIVIDE_BY_ZERO;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	if(Dividend->Zero()) {     // I guess we need it, since the routine does not work with Dividend = zero
		rm->Assign(0);
		div_done = 1;
	}

	if(this->IsEqual(Divisor)) {     // This check not needed, but makes the division of same number faster
		this->Assign((__int64)1);
		rm->Assign(0);
		div_done = 1;
	}

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {			// subtracts the dividend by enough to make divisble by mod^pwr			

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;

					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell
					
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnyPartSkips()) {				// if any pending full skips or partial skips, then perform base extension

						numcnt = Dividend->ExtendPart2Norm();
						Divisor->ExtendPart2Norm();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnyPartSkips()) {

				numcnt = Dividend->ExtendPart2Norm();
				Divisor->ExtendPart2Norm();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

			}

				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";		// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}

				acc->Add(Dividend);			// acc += result
				counter[COUNTERS::ADD_COUNT] += 1;

//				sign *= -1;						// change accumulate sign (toggle flag)! NO LONGER NEEDED
				
				dif->Assign(DividendCopy);

				temp->Assign(DivisorOrig);
				temp->Mult(Dividend);
				counter[COUNTERS::MULT_COUNT] += 1;
				
				dif->Sub(temp);
				counter[COUNTERS::SUB_COUNT] += 1;
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

//				if(!Dividend->Zero() && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {		// if dif >= DivisorOrig then .....
				if(!Dividend->Zero()) {		// if dif >= DivisorOrig then .....
					
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
/*
				else if(!first_zero) {
					first_zero = 1;			// flag that the dividend went to zero
					
					if(this->ena_dplytrc) printf(" *(%d)\r\n", clocks);

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;

				}
*/
				else {							// division is now complete ....
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
												// This is the post accum adjustment part
					if(dif->IsEqual(temp) && !dif->Zero()) {	// added this check, in case dif is EQUAL to temp, but not zero!
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						
												// Need to study this!  - it's an error measure of the final division
//						dif->Assign(acc);				// get the accum final accumulator (quotient) value
//						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
//						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
						
						if((zero=Dividend->Zero()) && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {	// see if divdend went to zero, and dif >= DivisorOrig
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//							if(dif->Compare(DivisorOrig, clocks)) printf("TEST: dif is larger\n");
							acc->Increment();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

					// ERROR DETECTION ROUTINE
					if(!DivCheck(DividendOrig, DivisorOrig, acc, rm)) {
						cout << "ERROR in INTEGER DIVIDE !!! - DIVPM5" << endl;
						cout << "DivisorOrig: " << DivisorOrig->Print10() << " DividendOrig: " << DividendOrig->Print10() << endl;
						wait_key();
						error_code = INTERNAL_ERROR;
					}

					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;

			}
			else {


					
			}
		}		// end if(pm2.divisible_by_pm())
		else {

			Divisor->Increment();
			
			if (this->ena_dplytrc) cout << "+d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

	return(error_code);

}

// TEST version for reducing the number of base extends
// derived from DivPM6 which eliminates compare on iteration
int PPM::DivPM7(PPM *ppm, PPM *rm)
{ 
//int sign = 1;			// sign value to inverse additions (not used in thie version at this time)
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;
int error_code = 0;		// start with no error
int zero;
int first_zero = 0;		// flag to indicate the dividend has gone to zero at least once


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
//	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

//	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
//	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
//		printf("ERROR: DIVIDE BY ZERO ERROR!\r\n");
//		wait_key();
		div_done = 1;
		error_code = DIVIDE_BY_ZERO;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	if(Dividend->Zero()) {     // I guess we need it, since the routine does not work with Dividend = zero
		rm->Assign(0);
		div_done = 1;
	}

	if(this->IsEqual(Divisor)) {     // This check not needed, but makes the division of same number faster
		this->Assign((__int64)1);
		rm->Assign(0);
		div_done = 1;
	}

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM(Divisor, dcnt, prime_index, pwr, NumDigits)) {			// subtracts the dividend by enough to make divisble by mod^pwr			

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";	// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;

					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell
					
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {
/*
					if(Divisor->AnyPartSkips()) {				// if any pending full skips or partial skips, then perform base extension

						numcnt = Dividend->ExtendPart2Norm();
						Divisor->ExtendPart2Norm();

						if(this->ena_dplytrc) printf(" <E:%d> ", numcnt);
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

					}
*/
					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

//			if(Divisor->AnyPartSkips()) {
//			if(Divisor->AnySkips()) {
			if(Divisor->Rn[Mod2_index]->Skip) {        // try to only base extend on two's modulus being completely divided out
				numcnt = Dividend->ExtendPart2Norm();
				Divisor->ExtendPart2Norm();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

			}

				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";			// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}

				if(Divisor->AnyPartSkips()) {          // extend here to ensure addition and other operations are valid
//				if(Divisor->AnySkips()) {
					numcnt = Dividend->ExtendPart2Norm();
					Divisor->ExtendPart2Norm();

					if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
					counter[COUNTERS::BASE_EXTEND] += 1;
					counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

				}

				acc->Add(Dividend);			// acc += result
				counter[COUNTERS::ADD_COUNT] += 1;

//				sign *= -1;						// change accumulate sign (toggle flag)! NO LONGER NEEDED
				
				dif->Assign(DividendCopy);

				temp->Assign(DivisorOrig);
				temp->Mult(Dividend);
				counter[COUNTERS::MULT_COUNT] += 1;
				
				dif->Sub(temp);
				counter[COUNTERS::SUB_COUNT] += 1;
				
//				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

				counter[COUNTERS::LOOP_COUNT] += 1;

//				if(!Dividend->Zero() && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {		// if dif >= DivisorOrig then .....
				if(!Dividend->Zero()) {		// if dif >= DivisorOrig then .....
					
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					
				}
				else {							// division is now complete ....
					if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
												// This is the post accum adjustment part
					if(dif->IsEqual(temp) && !dif->Zero()) {	// added this check, in case dif is EQUAL to temp, but not zero!
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						
												// Need to study this!  - it's an error measure of the final division
//						dif->Assign(acc);				// get the accum final accumulator (quotient) value
//						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
//						counter[COUNTERS::MULT_COUNT] += 1;

//						counter[COUNTERS::COMPARE_COUNT] += 1;
						clocks = 0;
						
						if((zero=Dividend->Zero()) && (dif->IsEqual(DivisorOrig) || dif->Compare(DivisorOrig, clocks))) {	// see if divdend went to zero, and dif >= DivisorOrig
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
//							if(dif->Compare(DivisorOrig, clocks)) printf("TEST: dif is larger\n");
							acc->Increment();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							if (this->ena_dplytrc) cout << " *(" << clocks << ")" << endl;
						}

//						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						counter[COUNTERS::COMPARE_CLK] += clocks;
						if(clocks) counter[COUNTERS::COMPARE_COUNT] += 1;
//						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

					// ERROR DETECTION ROUTINE
					if(!DivCheck(DividendOrig, DivisorOrig, acc, rm)) {
						cout << "ERROR in INTEGER DIVIDE !!! - DIVPM5" << endl;
						cout << "DivisorOrig: " << DivisorOrig->Print10() << " DividendOrig: " << DividendOrig->Print10() << endl;
						wait_key();
						error_code = INTERNAL_ERROR;
					}

					counter[COUNTERS::TOTAL_CLK] = counter[DIV_COUNT]+counter[EXTEND_CLK]+counter[ADD_COUNT]+counter[MULT_COUNT]+counter[COMPARE_CLK]+counter[DEC_COUNT];
					div_done = 1;
				}
//				counter[COUNTERS::COMPARE_CLK] += clocks;

			}
			else {


					
			}
		}		// end if(pm2.divisible_by_pm())
		else {

			Divisor->Increment();
			
			if (this->ena_dplytrc) cout << "+d ";
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

	return(error_code);

}

// divide using power based modulus... used to simulate the hardware apparatus
// supports power based divide, but like first hardware prototype, should be restricted to one power at a time.
void PPM::DivPM_Rez9A(PPM *ppm, PPM *rm)
{
int sign = 1;			// sign value to inverse additions
int last_sign;
int div_done = 0;			// need flag to inidcate if divide by parts is complete
int prime_index, scnt, dcnt;
int numcnt = 0;			//  temporary variables for the base_extend clock counts 
int dencnt = 0;
int j=0;
int pwr;


	PPM *dif = new PPM(0);			// used for conditional calcs
	PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	PPM *DividendCopy = new PPM(0);
	PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	PPM *temp = new PPM(0);

	PPM *Divisor = new PPM(0);
	PPM *Dividend = new PPM(0);

	Divisor->Assign(ppm);
	Dividend->Assign(this);

//	dif->ena_dplytrc = pm1->ena_dplytrc;		// stupid, but works...enable of disable trace
//	acc->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	pm1_tmp2->ena_dplytrc = pm1->ena_dplytrc;
//	pm2_tmp->ena_dplytrc = pm1->ena_dplytrc;
//	temp->ena_dplytrc = pm1->ena_dplytrc;
//	temp2->ena_dplytrc = pm1->ena_dplytrc;


	acc->Assign(0);					// clear the accumulator
	sign = 1;						// reset the sign flag initially
	div_done = 0;					// reset the done flag

	DividendCopy->Assign(Dividend);		// initially create a backup copy to Dividend, will be temp backup during processing
	DividendOrig->Assign(Dividend);		// a copy of the original value of the Dividend, does not change during processing		
	DivisorOrig->Assign(Divisor);		// assign a temp PM counter for pm2

	Divisor->Clear_Skips();			// clear the skip arrays before proceeding!  (don't really need now)
	Dividend->Clear_Skips();

//	clear_counters();			// clear the counters first

	if(Divisor->Zero()) {	// check for divide by zero, flag error and return
		cout << "ERROR: DIVIDE BY ERROR!" << endl;
		wait_key();
		div_done = 1;
	}

	if(Divisor->One()) {	// check for divide by 1, return Dividend, and assign reminder=0
		rm->Assign(0);
		div_done = 1;
	}

	while(!div_done) {
		
		prime_index = 0;		

		if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {
			scnt = 0;
			while(!Divisor->One()) {

				if(Dividend->Dec_by_next_fact_PM_1pwr(Divisor, dcnt, prime_index, pwr, NumDigits)) {		// MODIFIED to allow only ONE power for hardware simulation				

					if (dcnt && this->ena_dplytrc) cout << " -" << dcnt << "n ";							// if there is a factor, print the offset, else skip this print					
					if(dcnt) counter[COUNTERS::SUB_COUNT] += 1;


//					Dividend->ModDiv(ModTable::primes[prime_index]);			// perform modulo digit division
//					Divisor->ModDiv(ModTable::primes[prime_index]);
					Dividend->ModDiv(power(ModTable::primes[prime_index], pwr));
					Divisor->ModDiv(power(ModTable::primes[prime_index], pwr));

					counter[COUNTERS::DIV_COUNT] += 1;			// two divides above count as one clock, done in paralell

					Divisor->Rn[prime_index]->Skip = 1;			// flag this digit as needing base extension
					Dividend->Rn[prime_index]->Skip = 1;	
					
			cout << "\r\nDividend: " << Dividend->Prints() << endl;
			cout << "Divisor:  " << Divisor->Prints() << endl;

//					if(this->ena_dplytrc) printf("%d[]:", ModTable::primes[prime_index]);
					if (this->ena_dplytrc) cout << power(ModTable::primes[prime_index], pwr) << "[]:";
					
					scnt += 1;
					prime_index += 1;		// dec by next factor starts with this 

				}
				else {

					if(Divisor->AnySkips()) {				// if any pending skip pos set, then perform base extension

						numcnt = Dividend->ExtendNorm();
						Divisor->ExtendNorm();

						if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
						counter[COUNTERS::BASE_EXTEND] += 1;
						counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

						Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
						Dividend->Clear_Skips();

					}

					if(Divisor->Divisible_by_n_pmd_PM(NumDigits)) {

						prime_index = 0;		// still more zeroes in the denominator, start scan at beginning
					}
					else {
						break;		// go get next divisor
					}
				
				}
				
			}								// end of while(!pm2.one()  && (prime_index < NUM_PM_DIGS-1)) {

			if(Divisor->AnySkips()) {
				numcnt = Dividend->ExtendNorm();
				Divisor->ExtendNorm();

				if (this->ena_dplytrc) cout << " <E:" << numcnt << "> ";
				counter[COUNTERS::BASE_EXTEND] += 1;
				counter[COUNTERS::EXTEND_CLK] += numcnt;	// numerator extension takes longer, and is the key value we need

				Divisor->Clear_Skips();		// clear the skip variables!  SHOULDN'T NEED
				Dividend->Clear_Skips();
			}

//			cout << "\r\nDividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;
//			cout << "Divisor:  " << Divisor->Prints() << " = " << Divisor->Print10() << endl;
				
			if (this->ena_dplytrc) cout << "{" << scnt << "} ";

			if(Divisor->One() || Dividend->Zero()) {				// division loop done ..., added zero check, doesn't seem to chg fraction case

				if (Divisor->One() && this->ena_dplytrc) cout << " <1> ";
					
				if(Dividend->Zero()) {
					if (this->ena_dplytrc) cout << " <0> ";			// this may be a short cut to zero result, but it is not inclusive, not all zero results pass through this logic
				}



				if(sign > 0) {				// process the accumulator with the latest dividend value, observing the polarity, either add or subtract
					acc->Add(Dividend);			// acc += result
					cout << "\r\nAcc after Add: " << acc->Prints() << " = " << acc->Print10() << endl;

				}
				else {
					acc->Sub(Dividend);			// acc -= result
					cout << "\r\nAcc after Sub: " << acc->Prints() << " = " << acc->Print10() << endl;
				}

				last_sign = sign;
				sign *= -1;						// change accumulate sign (toggle flag)!
				counter[COUNTERS::ADD_COUNT] += 1;

				cout << "\r\nCurrent working Dividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;
				cout << "\r\nCurrent DividendCopy: " << DividendCopy->Prints() << " = " << DividendCopy->Print10() << endl;

				dif->Assign(DivisorOrig);			// dif = original divisor 
				dif->Mult(Dividend);				// dif = original divisor * current working dividend (temp quotient)
				counter[COUNTERS::MULT_COUNT] += 1;

				temp->Assign(DividendCopy);			// temp = copy of dividend at start of iternation					
				temp->Add(DivisorOrig);			    // add original divisor since comparison is: if ((quotient_result * divisor) < (divisor + dividend_copy)) 
													//									same as: if(((quotient_result * divisor) - dividend_copy) < divisor)
				counter[COUNTERS::ADD_COUNT] += 1;

				cout << "\r\ndif: " << dif->Prints() << " = " << dif->Print10() << endl;
				cout << "\r\ntemp: " << temp->Prints() << " = " << temp->Print10() << endl;
			
				// Note: Instead of comparing the results of last iteration to its expected target, 
				// could we instead always go back to check error against final result instead?
				// (the first time through, we are checking against the final result anyways)
				
				counter[COUNTERS::COMPARE_COUNT] += 1;
				int clocks = 0;

//				printf("\r\n");
//				cout << "dif= " << dif->Print10() << "  temp= " << temp->Print10() << endl;
				if(dif->Compare(temp, clocks)) {		// if dif > temp then .....
					

					dif->Sub(DividendCopy);			// subtract DividendCopy from dif (creates the real difference)		
					counter[COUNTERS::SUB_COUNT] += 1;
					 
					if (this->ena_dplytrc) cout << " *(" << clocks << ")\r\n" << endl;
//					printf("dif = %I64d, dif %.*f percent of DividendCopy", dif->decimal3(), 5, (float) dif->decimal3()/(float) DividendCopy->decimal3()*100.0);
//					printf("\r\n");

					DividendCopy->Assign(dif);		// set up DividendCopy for next iteration
					Dividend->Assign(dif);			// set up Dividend register for next iteration

					cout << "\r\ntrial>(dividend_copy*divisor), create new dividend_copy: " << endl;
					cout << "\r\nDividend: " << Dividend->Prints() << " = " << Dividend->Print10() << endl;

					Divisor->Assign(DivisorOrig);		// restore the Divisor for next iteration

					counter[COUNTERS::LOOP_COUNT] += 1;
				}
				else {							// division is now complete ....
					cout << "\r\ntrial<=(dividend_copy*divisor), division terminating: " << endl;
					if (this->ena_dplytrc) cout << " *(" << clocks << ")\r\n";

					if(dif->IsEqual(temp)) {	// added this check, in case dif is EQUAL to temp
						acc->Increment();		// (this case comes about when dividing evenly by large semi primes > PM ?)
												// one case is: 	val1 = 53 * 59 * 71 * 73 * 73;			// dividing these results in error!  (should be fixed now)	
												//					val2 = 59;
						counter[COUNTERS::ADD_COUNT] += 1;
					}
					else {						// This is the poorly understood accum adjustment part
												// Need to study this!  - it's an error measure of the final division
						dif->Assign(acc);				// get the accum final accumulator (quotient) value
						dif->Mult(DivisorOrig);		    // multiply by the original divisor to get our target Dividend
						counter[COUNTERS::MULT_COUNT] += 1;

						counter[COUNTERS::COMPARE_COUNT] += 1;
//						clocks = 0;
						
						printf("\r\n");
						cout << "final trial= " << dif->Prints() << " = " << dif->Print10() << endl;
						cout << "original dividend = " << DividendOrig->Prints() << " = " << DividendOrig->Print10() << endl;
						
						if(dif->Compare(DividendOrig, clocks)) {	// see if quotient is high by one, if so, adjust ...
							cout << "final trial greater than original Dividend, decrement acc" << endl;
							cout << "last sign: " << last_sign << endl;
							if(this->ena_dplytrc) printf(" *(%d)\r\n", clocks);
							acc->Decrement();
							counter[COUNTERS::DEC_COUNT] += 1;
						}
						else {
							cout << "final trial not greater than original Dividend, do nothing to acc" << endl;
							cout << "last sign: " << last_sign << endl;
							if(this->ena_dplytrc) printf(" *(%d)\r\n", clocks);
//							printf(" no adjust ");
						}

						counter[COUNTERS::COMPARE_CLK] += dif->counter[COUNTERS::COMPARE_CLK];
						dif->Clear_Counters();
					}

					// calculate remainder ...
					temp->Assign(DivisorOrig);		// temp = original divisor
					temp->Mult(acc);				// temp = original divisor * accum
					counter[COUNTERS::MULT_COUNT] += 1;

					rm->Assign(DividendOrig);		// rm = original dividend
					rm->Sub(temp);					// rm = original dividend - (original divisor * accum)
					counter[COUNTERS::SUB_COUNT] += 1;		

					Dividend->Assign(acc);			// got to assign the final answer!
					this->Assign(acc);			// save result into the class	

//					printf("the acc: %s\r\n", acc->Print10().c_str());

					// ERROR DETECTION ROUTINE
//					if((acc->Convert() != (DividendOrig->Convert() / DivisorOrig->Convert())) || (rm->Convert() != (DividendOrig->Convert() % DivisorOrig->Convert()))) {
//						printf("\r\n error detected in divide7! \r\n");
//						wait_key();
////						while(1);
//					}	
					
					div_done = 1;
				}
				counter[COUNTERS::COMPARE_CLK] += clocks;
//				dif->Clear_Counters();
			}
			else {

//				dcnt = Dividend->dec_by_next_fact2(Divisor);		
					
//				if(dcnt && this->ena_dplytrc) printf("-%dn ", dcnt);	// if there is a factor, print the offset, else skip this print
					
			}
		}		// end if(pm2.divisible_by_pm())
		else {
			Divisor->Decrement();
			if(this->ena_dplytrc) printf("-d ");
			counter[COUNTERS::DEC_COUNT] += 1;
		}

	}			// end while(!div-done)


	// order of creation if it makes a difference
	//PPM *dif = new PPM(0);			// used for conditional calcs
	//PPM *acc = new PPM(0);			// whole dividend working register ... will utiamtely contain the quotient
	//PPM *DividendCopy = new PPM(0);
	//PPM *DividendOrig = new PPM(0);	// original copy of Dividend value
	//PPM *DivisorOrig = new PPM(0);	// original copy of Divisor value
	//PPM *temp = new PPM(0);

	//PPM *Divisor = new PPM(0);
	//PPM *Dividend = new PPM(0);


	delete Dividend;
	delete Divisor;
	delete temp;
	delete DivisorOrig;
	delete DividendOrig;
	delete DividendCopy;
	delete acc;				// working register for quotient
	delete dif;				// used for conditional calcs

}


// ***************************   NEW GOLDSCHMIDT BASED INTEGER DIVIDE PROTOTYPE  ******************************************


void PPM::GoldDiv(PPM *divisor, PPM *rem)
{



}

// checks to see if the division is correct
int PPM::DivCheck(PPM *dividend, PPM *divisor, PPM *quotient, PPM *rem)
{
PPM *temp = new PPM(0);

	temp->Assign(divisor);
	if(!temp->Compare(rem)) {
		delete temp;
		return(0);		// error, need to validate this fact
	}
	temp->Mult(quotient);
	temp->Add(rem);

	if(dividend->IsEqual(temp)) {
		delete temp;
		return(1);	// if its equal, it's good
	}

	delete temp;
	return(0);

}

void PPM::Clear_Skips(void)
{
int size = Rn.size();

	for(int i=0; i<size; i++) {

		Rn[i]->Skip = 0;

	}

}

void PPM::Reset_PowerSkips(void)
{
int size = Rn.size();

	for(int i=0; i<size; i++) {

		Rn[i]->ResetPowerValid();

	}

}

void PPM::Reset_Normal(void)
{
int size = Rn.size();

	for(int i=0; i<size; i++) {

		Rn[i]->SetPowerNormal();

	}

}

void PPM::Backup(void)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->DigitCopy = Rn[i]->Digit;

	}
	
}

void PPM::Restore(void)
{

	for(int i=0; i<NumDigits; i++) {

		Rn[i]->Digit = Rn[i]->DigitCopy;

	}
	
}

void PPM::Clear_Counters(void)
{

	for(int i=0; i<NUM_PPM_COUNTERS; i++) {
		counter[i] = 0;
	}

}

// now returns total clocks
int PPM::PrintCounters(void)
{
int total = 0;

		printf("Clocks counts are:\r\n");
		printf("DIV Clocks:		%d\r\n", counter[PPM::COUNTERS::DIV_COUNT]);
		printf("SUB Clocks:		%d\r\n", counter[PPM::COUNTERS::SUB_COUNT]);
		printf("BE Clocks:		%d\r\n", counter[PPM::COUNTERS::EXTEND_CLK]);
		printf("CompareClk:		%d\r\n", counter[PPM::COUNTERS::COMPARE_CLK]);
		printf("ADD Clocks:		%d\r\n", counter[PPM::COUNTERS::ADD_COUNT]);
		printf("MultClocks:		%d\r\n", counter[PPM::COUNTERS::MULT_COUNT]);
		
		printf("Decrements:		%d\r\n", counter[PPM::COUNTERS::DEC_COUNT]);	
		printf("Compares:		%d\r\n", counter[PPM::COUNTERS::COMPARE_COUNT]);
		printf("BaseExtend:		%d\r\n", counter[PPM::COUNTERS::BASE_EXTEND]);
		printf("Iterations:		%d\r\n", counter[PPM::COUNTERS::LOOP_COUNT]);
		printf("Adjust Type:	%d\r\n", counter[PPM::COUNTERS::ADJUST_TYPE]);
		printf("total Clocks:	%d\r\n", counter[PPM::COUNTERS::TOTAL_CLK]);
		printf("\r\n");

		return(total);
}

// simple brute force method for conversion to decimal for initial testing
int PPM::DecimalB(void)
{
int val = 0;


	Backup();

	while(!Zero()) {

		Decrement();
		val += 1;

	}

	Restore();

	return(val);

}

/*
__int64 PPM::Decimal64(void)
{

	MRN *mrn = new MRN(this);

	__int64 val = mrn->Decimal64();

	delete(mrn);
	
	return(val);

}
*/

// M's Complement of the unsigned number
void PPM::Complement(void)
{

	PPM *temp = new PPM(0);

	temp->Sub(this);

	this->Assign(temp);

	delete temp;

}

// integer square root adapted from Newtons iteration plus error check!
// modeled after routine set up in square_root_test1
// Modified to use DivStd, which should be best division routine (should support full range, & no redundant modulus)
void PPM::Sqrt(void)
{

	PPM *y = new PPM(0);
	PPM *x = new PPM(0);
	PPM *last_x = new PPM(0);
	PPM *temp = new PPM(0);
//	PPM *temp2 = new PPM(0);
	PPM *rem = new PPM(0);
	PPM *result = new PPM(0);

	last_x->Assign(this);		// last_x = a;
	x->Assign(2);				// x = 2;

	while(1) {

		temp->Assign(this);		// y = x + a/x
		temp->ena_dplytrc = 0;		// disable dump of divide
		temp->DivStd(x, rem);
		y->Assign(x);
		y->Add(temp);
		
		temp->Assign(2);		// y = y / 2;
		y->ena_dplytrc = 0;
		y->DivStd(temp, rem);

//		cout << "PPM: x= " << x->Print10() << " y= " << y->Print10() << " last_x= " << last_x->Print10() << endl;
//		wait_key();

		if(y->IsEqual(last_x)) break;

		last_x->Assign(x);		// last_x = x;
		x->Assign(y);			// x = y;

	}
	
	if(last_x->Compare(x)) {	// choose the smaller of the two
		result->Assign(x);
	}
	else {
		result->Assign(last_x);
	}


/*
//
// Validation routine 
//
	temp->Assign(result);		// Checking routine just in case error detected
	temp2->Assign(temp);

	temp->Mult(temp2);

//	cout << "result sqr= " << temp->Print10() << " this= " << this->Print10() << endl;


	if(temp->IsEqual(this)) {
		this->Assign(result);
	}
	else if(this->Compare(temp)) {

		temp->Assign(result);
		temp->Add(1);
		temp2->Assign(temp);
		temp->Mult(temp2);

		if(temp->Compare(this)) {
			this->Assign(result);		// result is good
//			printf("result checked\r\n");
		}
		else {
			printf("Error1 in SquareRoot\r\n");
//			wait_key();
		}
	}
	else {
		printf("Error2 in SquareRoot\r\n");
//		wait_key();
	}
*/
	this->Assign(result);


	delete result;
	delete rem;
//	delete temp2;
	delete temp;
	delete last_x;
	delete x;
	delete y;
	
}




// ---------------------------------------------------------------------
//  PPMDigit Class member function definitions
// ---------------------------------------------------------------------

PPMDigit::PPMDigit(int digval, int mod_index, int power)
{

	Modulus = ModTable::primes[mod_index];		// Assign the base modulus of the digit
	Index = mod_index;						// assign index of the object, modulus, and primes array
	Power = power;							// Assign normal power of the modulus
	NormalPower = power;					// (copy of the normal Power), NormalPower needed to return derived number systems back to original base RNS format (constant)
	PowerValid = power;						// variable to define number of presently "valid" powers of the modulus, used to support variable digit modulus (partial powers)
	Digit = digval;							// value of the digit, which is maintained correctly for all arithmetic and power operations.


}

PPMDigit::~PPMDigit(void)
{

//	printf("Digit %d destructor\r\n", Index);

}

int PPMDigit::Add(int digval)
{


	Digit += digval;
	Digit %= power(Modulus, PowerValid);

	return(Digit);

}

int PPMDigit::Sub(int digval)
{

	Digit -= digval;
	Digit %= power(Modulus, PowerValid);			// shouldn't need  (DO NEED IN GENERAL!!)
													// the problem stemming from lack of ppm digit type, i.e., other than int type
	if(Digit < 0) 
		Digit += power(Modulus, PowerValid);

//	Digit %= power(Modulus, PowerValid);			

	return(Digit);

}

// added support for digits > 15 bits wide, less than 30 bits wide
int PPMDigit::Mult(int digval)
{
unsigned long long longResult;

	longResult = Digit;
	longResult *= digval;
	longResult %= power(Modulus, PowerValid);
	Digit = (int)longResult;

	return(Digit);

}

// Digit = Digit/digval mod Modulus
// Does this really work for each value of PowerValid?
// (name temporarily switched to DivPM to see impact on original code!)
int PPMDigit::DivPM(int digval)
{

	Digit = (int) ModTable::arrayDivTbl[Index][Digit][digval];		// need to reference this static member from the PPM class

	return(Digit);

}

// this version calculates the reverse multiply using variable power modulus
// digval is really digmod (digit modulus)
int PPMDigit::Div(int digval)
{
int mod;
int dig_limit, trial_dig;
unsigned long long i;
__int64 dig_inv;

	mod = power(Modulus, PowerValid);		// get the current modulus

	if(!(digval % mod)) {        // if there is a divide by zero, return zero as convention
		Digit = 0;
		PowerValid = 0;
		Skip = 1;

		return(0);
	}
	else if(!(mod % digval)) {                // we are trying to divide by a power of the modulus
		
		if((PowerValid > 0) && !Digit) {    // the digit has a value if it is zero and has valid power!
			Digit = mod;
		}

		Digit = Digit / digval;				// the new value is divided out of the current digit
		PowerValid -= get_num_of_powers(digval, Modulus);
		Digit = Digit % power(Modulus, PowerValid);				// truncate the digit range may be neccesary

		if(!PowerValid) Skip = 1;				// THIS IS A KEY POINT, THAT THIS IS AUTOMATICALLY DONE, THIS IS IMPORTANT!

		if(PowerValid < 0) {
			cout << "ERROR: PowerValid <= 0 in PPMDigit::Div" << endl;
			wait_key();
			return(-1);
		}

		return(Digit);
	}
	else {          // search for an multiplicative inverse, not elegant, but works for lib
		if (ModTable::returnRoutine() & USE_MODDIV_LUT) {

			int InvMod = (int)ModTable::ModDivTbl[Index][PowerValid - 1][digval];		// get the multiplicative inverse of this_mod (@ Index) with respect to digmod
			Digit = (Digit * InvMod) % power(Modulus, PowerValid);						// multiply the digit by the multiplicative inverse, then take MOD (this->current_modulus)
			return(Digit);
		}
		else if (ModTable::returnRoutine() & USE_BRUTE_LUT) {

			Digit = (int)ModTable::arrayDivTbl[Index][Digit][digval];		// perform a ModDiv brute force look up
			Digit = Digit % power(Modulus, PowerValid);						// truncate the digit range is neccesary
			return(Digit);
		}
		else {

			dig_limit = power(Modulus, PowerValid);


			// default is to use the Euclidean method for finding inverse

			dig_inv = get_inv(digval, dig_limit);			// get inverse using extended Euclidean
			trial_dig = (unsigned long long)(dig_inv * Digit) % dig_limit;
			i = trial_dig;		// take out if using brute force

			Digit = i;		// don't forget!
		return(i);

		cout << "ERROR: in PPMDigit::Div, Digit = " << Digit << ", dig_limit = " << dig_limit
			<< ", index = " << i << ", digval = " << digval << ", skip = " << Skip; 
		//printf("ERROR: in PPMDigit::Div, Digit=%d, dig_limit=%d, index=%lld, digval=%d, skip=%d\n", Digit, dig_limit, i, digval, Skip);
		wait_key();
		return(-1);			// first error condition, likley unhandled

		}

		

		// -----------------------------------------------------------------------------------------
//#ifdef USE_MODDIV_LUT
//	#ifdef USE_BRUTE_LUT
//
//		Digit = (int) ModTable::arrayDivTbl[Index][Digit][digval];		// perform a ModDiv brute force look up
//		Digit = Digit % power(Modulus, PowerValid);						// truncate the digit range is neccesary
//
//	#else 
//
//		int InvMod = (int) ModTable::ModDivTbl[Index][PowerValid-1][digval];		// get the multiplicative inverse of this_mod (@ Index) with respect to digmod
//		Digit = (Digit * InvMod) % power(Modulus, PowerValid);						// multiply the digit by the multiplicative inverse, then take MOD (this->current_modulus)
//
//	#endif
//
//	return(Digit);
//
//#else
//
//
//		dig_limit = power(Modulus, PowerValid);
//
//#ifdef USE_BRUTE_LUT
//
//		
////	brute force search
//		for(i=0; i<dig_limit; i++) {
//			trial_dig = (unsigned long long)(i * (digval)) % dig_limit;
//			if(trial_dig == Digit) {
////				Digit = i;
////				return(i);
//				break;
//			}
//		}
//
//#else			// default is to use the Euclidean method for finding inverse
//
//		dig_inv = get_inv(digval, dig_limit);			// get inverse using extended Euclidean
//		trial_dig = (unsigned long long)(dig_inv * Digit) % dig_limit;
//		i = trial_dig;		// take out if using brute force
//
//#endif
//
////		if (trial_dig != 0) {
////			printf("ERROR: in PPMDigit::Div, Digit=%d, dig_limit=%d, index=%d, digval=%d, skip=%d\n", Digit, dig_limit, i, digval, Skip);
////			wait_key();
////		}
//
////		if (trial_dig != i) {
////			printf("ERROR: in PPMDigit::Div, Digit=%d, dig_limit=%d, index=%d, digval=%d, skip=%d\n", Digit, dig_limit, i, digval, Skip);
////			wait_key();
////		}
//
//		Digit = i;		// don't forget!
//		return(i);
//
//		cout << "ERROR: in PPMDigit::Div, Digit = " << Digit << ", dig_limit = " << dig_limit
//			<< ", index = " << i << ", digval = " << digval << ", skip = " << Skip; 
//		//printf("ERROR: in PPMDigit::Div, Digit=%d, dig_limit=%d, index=%lld, digval=%d, skip=%d\n", Digit, dig_limit, i, digval, Skip);
//		wait_key();
//		return(-1);			// first error condition, likley unhandled
//#endif
	}

}

// this version calculates the reverse multiply using multiplicative inverse of this->modulus, with respect to digmod
// then the routine calculates the reverse multiply by: 
int PPMDigit::Div2(int digmod)
{
int mod;
int dig_limit, trial_dig;
unsigned long i;

	mod = power(Modulus, PowerValid);		// get the current modulus

	if(!(digmod % mod)) {        // if there is a divide by zero, return zero as convention
		Digit = 0;
		PowerValid = 0;
		Skip = 1;

		return(0);
	}
	else if(!(mod % digmod)) {                // we are trying to divide by a power of the modulus
		
		if((PowerValid > 0) && !Digit) {    // the digit has a value if it is zero and has valid power!
			Digit = mod;
		}

		Digit = Digit / digmod;				// the new value is divided out of the current digit
		PowerValid -= get_num_of_powers(digmod, Modulus);
		Digit = Digit % power(Modulus, PowerValid);				// truncate the digit range may be neccesary

		if(!PowerValid) Skip = 1;				// THIS IS A KEY POINT, THAT THIS IS AUTOMATICALLY DONE, THIS IS IMPORTANT!

		if(PowerValid < 0) {
			cout << "ERROR: PowerValid <= 0 in PPMDigit::Div" << endl;
			wait_key();
			return(-1);
		}

		return(Digit);
	}
	else {          // search for an multiplicative inverse, not elegant, but works for lib
		if (ModTable::returnRoutine() & USE_MODDIV_LUT) {
			int InvMod = (int)ModTable::ModDivTbl[Index][PowerValid - 1][digmod];		// get the multiplicative inverse of this_mod (@ Index) with respect to digmod
			Digit = (Digit * InvMod) % power(Modulus, PowerValid);		// multiply the digit by the multiplicative inverse, then take mod this->modulus

			return(Digit);
		}
		else {
			dig_limit = power(Modulus, PowerValid);
			for (i = 0; i < dig_limit; i++) {
				trial_dig = (unsigned long)(i * digmod) % dig_limit;
				if (trial_dig == Digit) {
					Digit = i;
					return(i);
				}
			}

			cout << "ERROR: in PPMDigit::Div, Digit=" << Digit << ", dig_limit=" << dig_limit << ", index=" << i << ", digval=" << digmod << ", skip=" << Skip << endl;
			wait_key();
			return(-1);			// first error condition, likley unhandled
		}
// --------------------------------------------------------------------------------------------
//#ifdef USE_MODDIV_LUT
//
////		int this_mod = power(Modulus, PowerValid);
////		int InvMod = (int) ModTable::ModDivTbl[this_mod][digmod];	// get the multiplicative inverse of this_mod with respect to digmod
//		int InvMod = (int) ModTable::ModDivTbl[Index][PowerValid-1][digmod];		// get the multiplicative inverse of this_mod (@ Index) with respect to digmod
//		Digit = (Digit * InvMod) % power(Modulus, PowerValid);		// multiply the digit by the multiplicative inverse, then take mod this->modulus
//
//		return(Digit);
//#else
//		dig_limit = power(Modulus, PowerValid);
//		for(i=0; i<dig_limit; i++) {
//			trial_dig = (unsigned long)(i * digmod) % dig_limit;
//			if(trial_dig == Digit) {
//				Digit = i;
//				return(i);
//			}
//		}
//
//		printf("ERROR: in PPMDigit::Div, Digit=%d, dig_limit=%d, index=%d, digval=%d, skip=%d\n", Digit, dig_limit, i, digmod, Skip);
//		wait_key();
//		return(-1);			// first error condition, likley unhandled
//#endif
	}

}


void PPMDigit::SkipDigit(void)
{

	Skip = 1;	

}

void PPMDigit::ClearSkip(void)
{

	Skip = 0;	

}

// sets all powervalid counts to Normal, which is Power
void PPMDigit::ResetPowerValid(void)
{

	PowerValid = Power;

}

// reset the variable modulus to it's NORMALIZED format
void PPMDigit::SetPowerNormal(void)
{
	Power = NormalPower;	
	PowerValid = NormalPower;
}

//truncates the PPM digit modulus by the number of powers specified
// used in SPMF2 scaling routines
// returns 1 if OK, else 0 if error
int PPMDigit::TruncMod(unsigned int num_pwrs)
{

	if(num_pwrs < PowerValid) {
		PowerValid -= num_pwrs;
		Digit %= power(Modulus, PowerValid);
		return(1);
	}
	else if(num_pwrs == PowerValid) {
		PowerValid = 0;
		Digit = 0;
		Skip = 1;
		return(1);
	}
	else {
		cout << "Error: Truncation error in truncmod" << endl;
		wait_key();
		return(0);
	}

}

int PPMDigit::GetModulus(int &power)
{

	power = Power;
	return(Modulus);

}

int PPMDigit::GetFullPowMod(void)
{

	return(power(Modulus, Power));		// return the full modulus of the digit

}

int PPMDigit::GetPowMod2(void)
{

	return(power(Modulus, PowerValid));		// return the current partial modulus of the digit

}

// returns the offset that when subtracted, rounds to nearest power
int PPMDigit::GetPowOffset(int pwr)
{

	return(Digit % power(Modulus, pwr));	

}

// returns the value of the digit, and also handles the partial power digit case
int PPMDigit::GetDigit(void)
{

	if(PowerValid == Power) {				// this check is also redundant, and not needed
		return(Digit);
	}
	else {
		return(Digit % power(Modulus, PowerValid));		// the mod operation is redundant, since this is performed in all operations anyways
	}

}

// copies a PPM digit from a source PPM digit
// to be used by constructor only
void PPMDigit::CopyDigit(PPMDigit *digit_src)
{

	Digit = digit_src->Digit;
	DigitCopy = digit_src->DigitCopy;
	Skip = digit_src->Skip;
	PowerValid = digit_src->PowerValid;
	Modulus = digit_src->Modulus;
	Index = digit_src->Index;
	Power = digit_src->Power;
	NormalPower = digit_src->NormalPower;


}

int PPMDigit::IsZero(int &powref)
{

	for(int i=Power; i>0; i--) {

		if(!(Digit % power(Modulus, i))) {
			powref = i;
			return(1);
		}
	}

	return(0);
		
}

int PPMDigit::IsZeroPM(int &powref)
{

	for(int i=PowerValid; i>0; i--) {

		if(!(Digit % power(Modulus, i))) {
			powref = i;
			return(1);
		}
	}

	return(0);
		
}

// this is a restricted version of IsZeroPM(&pwrref)
// used for the simulation of hardware prototype
// restricted to returning a single power only
int PPMDigit::IsZeroPM_1pwr(int &powref)
{


	if(PowerValid >= 1) {
		for(int i=1; i>0; i--) {

			if(!(Digit % power(Modulus, i))) {
				powref = i;
				return(1);
			}
		}
	}

	return(0);
		
}

// used to develope DivPM2, this version supports power digits with partial skipped sub-digits
int PPMDigit::IsZero2(int &powref)
{

	for(int i=Power; i>0; i--) {

		if(!(Digit % power(Modulus, i))) {
			powref = i;
			return(1);
		}
	}

	return(0);
		
}

