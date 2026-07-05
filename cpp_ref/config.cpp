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
#include "mrn.h"
#include "config.h"

using namespace std;

// For user defined modulus, define both the modulus array, and also the ModPowers array below. See examples.
// then set #define CUSTOM_MODULUS 1 in "config.h", and also set NUM_PPM_DIGS, NUM_MODULUS_POWERS and SPMF_FRACTION_DIGS in config.h
// each entry of the ModPowers array is associated with each modulus value of the modulus array
// the ModPowers array is defined for the first NUM_MODULUS_POWERS modulus, and can be less than NUM_PPM_DIGS, so be sure to set NUM_MODULUS_POWERS in "config.h"

//const int ModPowers[] = {1};				// if you want a custom RNS system with NO powers, set NUM_MODULUS_POWERS to 1

// *****************  Rez-9 Example Modulus ******************//
// *****************  18 digit Rez-9  modulus = {121, 125, 169, 243, 256, 289, 343, 361, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509}
// set the config.h options below as shown:
#define NUM_PPM_DIGS 18
#define SPMF_FRACTION_DIGS 7
#define NUM_MODULUS_POWERS 8
#define CUSTOM_MODULUS 1
// set the modulus values as shown below in this file:
//int Modulus[18] =		 { 11,    5,   13,  3,   2,  17,   7,  19, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509 };
int Modulus[18] =         {11,    5,   13,  3,   2,  17,   7,  19, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509};		// 2's power is fifth position
//const int ModPowers[18] = {2,   3,   2,   5,   8,   2,   3,  2,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1 };
const int ModPowers[18] = { 2,    3,   2,   5,   8,   2,   3,  2,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1 };


// ******************  RNS TPU Example Modulus  ********************//
// set the config.h options below as shown:
//#define NUM_PPM_DIGS 8
//#define SPMF_FRACTION_DIGS 2
//#define NUM_MODULUS_POWERS 8
//#define CUSTOM_MODULUS 1
// set the modulus values as shown below in this file:
//int Modulus[NUM_PPM_DIGS] = { 5, 2, 3, 262111, 262121, 262127, 262133, 262139};		// experiemental modulus for RNS TPU, Q=18
//const int ModPowers[NUM_MODULUS_POWERS] = { 7, 17, 11, 1, 1, 1, 1, 1 };

// ******************  RNS TPU Example Modulus with complete power based digits  ********************//
// set the config.h options below as shown:
//#define NUM_PPM_DIGS 8
//#define SPMF_FRACTION_DIGS 2
//#define NUM_MODULUS_POWERS 8
//#define CUSTOM_MODULUS 1
// set the modulus values as shown below in this file:
//int Modulus[NUM_PPM_DIGS] = { 13, 2, 5, 17, 7, 19, 11, 3};		// an example 8 digit RNS set with power based, Q=18 bit digits
//const int ModPowers[NUM_MODULUS_POWERS] = { 4, 16, 7, 4, 6, 4, 5, 11};


// test modulus case 1
// NUM_PPM_DIGS = 18
int modulus_1[18] = { 11,   5,  13,   3,   -2,  17,   7,  19, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509 };		// 2's power is fifth position
const int ModPowers_1[18] = { 2,    3,   2,   5,   8,   2,   3,  2,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1 };

//test modulus case 2
// NUM_PMM_DIGS = 8
int modulus_2[8] = { 5, 2, 3, 262111, 262121, 262127, 262133, 262139};			// experimental modulus for RNS TPU, Q=18
const int ModPowers_2[8] = { 7, 17, 11, 1, 1, 1, 1, 1 };

//test modulus case 3
// NUM_PMM_DIGS = 8
int modulus_3[8] = { 5, 2, 6, 262111, 262121, 262127, 262133, 262139 };			// test for pairwise prime modulus
const int ModPowers_3[8] = { 7, 17, 11, 1, 1, 1, 1, 1 };

//test modulus case 4
// NUM_PMM_DIGS = 8
int modulus_4[8] = { 5, 2, -3, 262111, 262121, 262127, 262133, 262139 };		// test for negative modulus
const int ModPowers_4[8] = { 7, 17, 11, 1, 1, 1, 1, 1 };

//test modulus case 5
// NUM_PMM_DIGS = 8
int modulus_5[8] = { 5, 2, 2147483647, 262111, 262121, 262127, 262133, 262139 };		// test for modulus value exceeding MAX_DIGIT
const int ModPowers_5[8] = { 7, 17, 11, 1, 1, 1, 1, 1 };

//test modulus case 6
// NUM_PMM_DIGS = 8
int modulus_6[8] = { 5, 7, 3, 262111, 262121, 262127, 262133, 262139 };		// test for at least one modulus value being a power of 2
const int ModPowers_6[8] = { 7, 17, 11, 1, 1, 1, 1, 1 };