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
#include <cmath>
typedef int64_t __int64;
#else

#endif
#include "stdafx.h"
#include <iostream>
#include "ppm.h"
#include "init.h"

int console_width = WIN10_WIDTH; // Sets the default console width to WIN10_WIDTH
bool print_the_remainder = TRUE;  // Sets the remainder to print by default

void print_modulus(PPM* ppm_base, int num_digs) {

	cout << endl << "The modulus in use are: " << endl;
	for (int i = 0; i < num_digs; i++) {
		cout << ppm_base->Rn[i]->GetFullPowMod() << ", ";
	}
	cout << endl << endl;

	if (ModTable::returnMode()) {
		cout << "The residue number system is using CUSTOM modulus" << endl;
	}
	else {
		cout << "The residue number system is using AUTO generated modulus" << endl;
	}

	if (ppm_base->PowerBased) {
		cout << "The system is using POWER BASED modulus" << endl;
	}
	else {
		cout << "The system is using NON Power Based modulus" << endl;
	}

}

void print_remainder(bool req_print_remainder) {
	print_the_remainder = req_print_remainder;
}

// Changes the default console width, either to 128 (Windows 10), or 80 (Windows 7/8)
void change_console_width(int new_console_width) {
	console_width = new_console_width;
}

void init_RNS_APAL(int mode, int routine, int num_digs, int num_frac_digs, int* mod_array, const int* powers_array) {

	validate_modulus(mode, mod_array, num_digs);

	//cout << "starting RNS-APAL library demo application ... \r" << endl;
	ModTable* modtbl = new ModTable(mode, routine, num_digs, num_frac_digs,  mod_array, powers_array);

	delete modtbl;

}

bool check_for_negative_numbers(int* mod_array, int num_digs) {

	for (int i = 0; i < num_digs; i++)
	{
		if (mod_array[i] < 0)
			return 1;								// there is a negative number in mod array
	}

	return 0; // there are no negative numbers in mod array
}

int gcd(int num_1, int num_2) {
	int gcdenom = 1;
	if (num_2 > num_1) {
		swap(num_1, num_2);
	}

	for (int i = 1; i <= num_2; i++) {
		if (num_1 % i == 0 && num_2 % i == 0) {
			gcdenom = i;
		}
	}

	return gcdenom;
}

bool verify_pairwise_prime(int* mod_array, int num_digs) {

	int num_1, num_2;

	for (int i = 0; i < num_digs - 2; i++)
	{
		num_1 = mod_array[i];
		for (int j = i + 1; j < num_digs - 1; j++)
		{
			num_2 = mod_array[j];
			if (gcd(num_1, num_2) > 1)
				return 0;		// modulus is NOT pair-wise prime
		}
	}

	return 1; // modulus is pair-wise prime
}


bool check_for_mod_power_of_two(int* mod_array, int num_digs) {

	for (int i = 0; i < num_digs; i++)
	{
		if (ceil(log2(mod_array[i])) == floor(log2(mod_array[i])))  // if we get the same number,
																	// mod_array[i] is a power of two.
			return 1;	// at least one modulus is a power of two
	}

	return 0; // no modulus is a power of two
}
bool check_for_digs(int* mod_array, int num_digs) {

	if (num_digs <= 0) {
		return 1;			// the number of digits is less than or equal to 0.
	}

	return 0; // the mod_array has > 0 digits
}

bool check_for_max_dig(int* mod_array, int num_digs) {

	for (int i = 0; i < num_digs; i++)
	{
		if (mod_array[i] > MAX_DIGIT)
			return 1;		// a modulus is greater than the maximum allowed digit.
	}

	return 0; // each digit in mod_array is < MAX_DIGIT
}

// Test function that will be used to test modulus and verify that it follows
// the proper requirements, such as:
//		1. No negative numbers are allowed in the modulus.
//		2. Modulus must be pairwise prime.
//		3. At least one of the modulus is a power of 2.
//      4. There must be more than 0 digits.
//      5. Each modulus must be less than MAX_NUM
bool validate_modulus(int mode, int* mod_array, int num_digs) {
	if (!(mode & CUSTOM))
		return 1;				// if the modulus is generated automatically, there is no need to validate.
	if (check_for_digs(mod_array, num_digs) == 1) {
		cout << "ERROR: Number of digits must be greater than 0." << endl;
		exit(0);
	}
	if (check_for_max_dig(mod_array, num_digs) == 1) {
		cout << "ERROR: Digits must be less than " << MAX_DIGIT << endl;
		exit(0);
	}
	if (check_for_negative_numbers(mod_array, num_digs) == 1) {
		cout << "ERROR: Negative number found in modulus." << endl;
		exit(0);
	}
	if (verify_pairwise_prime(mod_array, num_digs) == 0) {
		cout << "ERROR: Modulus must be pairwise prime." << endl;
		exit(0);
	}
	if (check_for_mod_power_of_two(mod_array, num_digs) == 0) {
		cout << "ERROR: No number from modulus is a power of two." << endl;
		exit(0);
	}


	return 1;					// modulus has been validated to be correct.
}