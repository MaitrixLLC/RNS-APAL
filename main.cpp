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
typedef char _TCHAR;

#else

#endif


#include "stdafx.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "time.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <sstream>
#include "utilities.h"
#include "ppm.h"
#include "mrn.h"
#include "sppm.h"
#include "spmf.h"
#include "init.h"

void print_modulus(PPM* ppm_base, int num_digs);			// this routine found in init.cpp

using namespace std;

void pm_alu_tests(void);

void new_mult_test(void);


int main(int argc, _TCHAR* argv[])
{

//  Example arrays for RNS with custom modulus
//	int Modulus[18] = { 11,   5,  13,   3,   2,  17,   7,  19, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509 };
//	int ModPowers[18] = { 2,    3,   2,   5,   8,   2,   3,  2,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1 };

	change_console_width(WIN10_WIDTH);
	print_remainder(FALSE);

//	Format: init_RNS_APAL(int mode, int routine, int num_digs, int num_frac_digs, int* mod_array, const int* powers_array);	
//	init_RNS_APAL(AUTO_GEN, USE_EXT_EUCL, 8, 0, NULL, NULL);								// Using Auto-generatoed, Integers Only, used in many tutorial examples
	init_RNS_APAL(CUSTOM, USE_EXT_EUCL, 18, 8, Modulus, ModPowers);							// 18 digit, CUSTOM modulus, with 6 digits of fractional range, see arrays above
//	init_RNS_APAL(AUTO_GEN|NO_POWERS, USE_EXT_EUCL, 31, 8, NULL, NULL);						// Auto generated, No powers, 31 digits with 8 digits of fractional range				
//	init_RNS_APAL(AUTO_GEN, USE_EXT_EUCL, 31, 8, Modulus, ModPowers);						// Auto generated, With Powers, 31 digits with 8 digits fractional range


	cout << "starting RNS-APAL library demo application ... \r" << endl;
	PPM* range = new PPM(0);

	print_modulus(range, range->NumDigits);
	cout << endl;

	range->Decrement();

	cout << "largest integer contained in the RNS is: " << range->Print(10) << endl;
	cout << endl;

	cout << "Press any key then enter to continue ..." << endl;
	wait_key();

	pm_alu_tests();

	return 0;
}


void new_mult_test(void)
{
char input[100];

	SPMF *arg1 = new SPMF(0);
	SPMF *arg2 = new SPMF(0);
	SPMF *arg3 = new SPMF(0);

	printf("Enter operand A or 'q' to quit (Example: -12.345): ");
	scanf("%s", &input[0]);

	while (strcmp(input, "q")) {

		arg1->AssignFP(input);
		cout << "arg1: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		arg3->Assign(arg1);		// make copy so that we can compare

		printf("Enter operand B: ");
		scanf("%s", &input[0]);

		arg2->AssignFP(input);
		cout << "arg2: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();

		arg1->Mult4(arg2);
		cout << "arg1: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		arg3->Mult4b(arg2);
		cout << "arg1b: " << arg3->Print(10) << endl << arg3->PPM::Print(DEC) << endl;
		arg3->PrintDemo();

		printf("\n\n");
		printf("Try another fractional multiplication?:\n");
		printf("Enter operand A or 'q' to quit (Example: -12.345): ");
		scanf("%s", &input[0]);

	}

	printf("Fractional Multiply demo terminated\n");
	wait_key();

}


void cls(void)
{
int i;

	for(i=0; i<25; i++) printf("\r\n");

}

void inc_demo(void)
{
char c;
int val;
	
	printf("INCREMENT DEMO\r\n");
	printf("This demo simply increments a starting RNS value\n\n");
	printf("Enter a starting RNS value as positive decimal integer: ");
	scanf("%d", &val);

	printf("the starting value entered is: %d\n\n", val);
	
	PPM *ppm = new PPM((__int64) val);
	ppm->PrintDemo();
	printf("\r\n");

	wait_key();

	cout << "Press enter to Increment or 'q' to quit" << endl;
	c = wait_key();

	while(c != 'q') {

		ppm->Add(1);
		cout << "Decimal value: " << ppm->Print(DEC) << endl;
		ppm->PrintDemo();

		printf("\r\n");

		cout << "Press a key to Increment or 'q' to quit" << endl;
		c = wait_key();

	}

	printf("increment demo terminated\n");
	delete(ppm);

}

void dec_demo(void)
{
int val;
	
	printf("DECREMENT DEMO\r\n");
	printf("this demo simply decrements a starting RNS value\n\n");
	printf("Enter the starting value as positive decimal integer: ");
	scanf("%d", &val);

	printf("the value entered is: %d\n\n", val);
	
	PPM *ppm = new PPM((__int64) val);
	printf("the starting RNS value is:\r\n");
	ppm->PrintDemo();
	printf("\r\n");

	wait_key();		// blow off the return from the scan

	cout << "Press enter to Decrement or 'q' to quit" << endl;
	char key = wait_key();

	while(key != 'q') {

//		cls();
		ppm->Sub(1);
		cout << "Decimal value: " << ppm->Print(DEC) << endl;
		cout << endl;
		ppm->PrintDemo();

		cout << "Press a key to Increment or 'q' to quit" << endl;
		key = wait_key();

	}

	printf("decrement demo terminated\n");


	delete(ppm);

}



void add_demo2(void)
{
SPPM *a = new SPPM(0);
SPPM *b = new SPPM(0);
char input[100];


	printf("ADDITION OF SIGNED INTEGERS DEMO\n\n");
	printf("this demo adds two signed RNS integers.\n");
	printf("arguments are entered in decimal or hexadecimal format.\n");
	printf("result is printed in decimal format and raw RNS format.\n\n");

	printf("Enter first arg (a) or 'q' to quit: ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		a->Assign(input);
		cout << "a: " << a->Print(DEC) << endl << endl;
		a->PrintDemo();
		printf("\n");

		printf("Enter (b) or 'q' to quit: ");
		scanf("%s", &input[0]);
		
		if(strcmp(input, "q")) {

			b->Assign(input);
			cout << "b: " << b->Print(DEC) << endl << endl;
			b->PrintDemo();
			printf("\n");

			a->Add(b);

			cout << "result: " << a->Print(DEC) << endl << endl;
			a->PrintDemo();
	
			printf("\n\n");
			printf("Try another:\n");
			printf("Enter first arg (a) or 'q' to quit: ");
			scanf("%s", &input[0]);

		}

	}

	printf("ADD demo routine terminated\n");

	delete a;
	delete b;

}


void sub_demo2(void)
{
SPPM *a = new SPPM(0);
SPPM *b = new SPPM(0);
char input[100];


	printf("SUBTRACTION OF SIGNED INTEGERS DEMO\r\n");
	printf("this demo subtracts two signed RNS integers.\n");
	printf("arguments are entered in decimal or hexadecimal format.\n");
	printf("result is printed in decimal format and raw RNS format.\n\n");

	printf("Enter first arg (a) or 'q' to quit: ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		a->Assign(input);
		cout << "a: " << a->Print(DEC) << endl << endl;
		a->PrintDemo();
		printf("\n");

		printf("Enter (b) or 'q' to quit: ");
		scanf("%s", &input[0]);
		
		if(strcmp(input, "q")) {

			b->Assign(input);
			cout << "b: " << b->Print(DEC) << endl << endl;
			b->PrintDemo();
			printf("\n");

			a->Sub(b);

			cout << "result: " << a->Print(DEC) << endl << endl;
			a->PrintDemo();

			printf("\n\n");
			printf("Try another:\n");
			printf("Enter first arg (a) or 'q' to quit: ");
			scanf("%s", &input[0]);

		}

	}

	printf("SUBTRACT demo routine terminated\n");

	delete a;
	delete b;

}


void multiply_demo2(void)
{
SPPM *a = new SPPM(0);
SPPM *b = new SPPM(0);
char input[100];


	printf("MULTIPLICATION OF SIGNED INTEGERS DEMO\r\n");
	printf("this demo multiplies two signed RNS integers.\n");
	printf("arguments are entered in decimal or hexadecimal format.\n");
	printf("result is printed in decimal format and raw RNS format.\n\n");

	printf("Enter first arg (a) or 'q' to quit: ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		a->Assign(input);
		cout << "a: " << a->Print(DEC) << endl << endl;
		a->PrintDemo();
		printf("\n");

		printf("Enter (b) or 'q' to quit: ");
		scanf("%s", &input[0]);
		
		if(strcmp(input, "q")) {

			b->Assign(input);
			cout << "b: " << b->Print(DEC) << endl << endl;
			b->PrintDemo();
			printf("\n");

			a->Mult(b);

			cout << "result: " << a->Print(DEC) << endl << endl;
			a->PrintDemo();

			printf("\n\n");
			printf("Try another:\n");
			printf("Enter first arg (a) or 'q' to quit: ");
			scanf("%s", &input[0]);

		}

	}

	printf("MULTIPLY demo routine terminated\n");

	delete a;
	delete b;

}


void mrconvert_demo(void)
{
char val[100];
int i;
int digit, mr_digcnt;
int power = 0;	
int mrn[100];

	printf("RNS TO MIXED RADIX CONVERSION DEMO\r\n");
	printf("This routine will convert an unsigned RNS integer into mixed radix format\n\n");
	printf("Enter an unsigned value to convert: ");
	scanf("%s", &val);

	printf("the string value entered is: %s\n\n", val);
	
	PPM *ppm_tmp = new PPM(0);
	PPM *ppm = new PPM(0);
	ppm_tmp->Assign(val);
	ppm->Assign(ppm_tmp);

	cout << "starting RNS value is:" << ppm->Print(DEC) << endl << endl;
	ppm->PrintDemo(10);
	printf("\nHit any key to start Mixed Radix conversion step by step\r\n");

	wait_key();

	mr_digcnt = 0;
	i = 0;
	while((i < ppm->NumDigits) && (wait_key() != 'q')) {

		if(!ppm->Rn[i]->Skip) {

			digit = ppm->Rn[i]->Digit;
			printf("subtracting %d (from digit position %d)\r\n", digit, i);
			ppm->Sub(digit);

			ppm->PrintDemo(10);
			printf("\r\n");
			
			mrn[i] = digit;
			mr_digcnt += 1;

			printf("GENERATED MIXED RADIX DIGIT (from position %d): %d\r\n", i, digit);
			printf("\r\n");
			wait_key();

			if(ppm->Zero(i)) {		// leave asap to eliminate leading zeros and wasted time.
				printf("remaining number is now zero - DONE:\n");
				ppm->PrintDemo(10);
				printf("\n");
				wait_key();
				break;
			}			

			power = ppm->Rn[i]->GetFullPowMod();
			ppm->ModDiv(power);
			printf("dividing by %d (modulus of digit position %d)\r\n", power, i);
			
			ppm->Rn[i]->Skip = 1;
			ppm->PrintDemo(10);
			printf("\r\n");

		}
		else {
			printf("digit position %d marked as skipped\r\n", i); 
		}

		i++;
	}

	printf("mixed radix number in MSD first format: ");
	if(mr_digcnt > 0) {
		for(i=mr_digcnt-1; i>=0; i--) {
			printf("%d ", mrn[i]);
		}
	}
	printf("\r\n");

	printf("Mixed radix conversion complete\n");

	delete(ppm_tmp);
	delete(ppm);

}


void mrconvert2_demo(void)
{
//char c[100];
int i;
int digit, mr_digcnt, mr_digpos;
int power = 0;	
int mrn[100];
char input[100];

	printf("MIXED RADIX TO RNS CONVERSION DEMO\r\n");
	printf("This routine to will convert an unsigned value to mixed radix first.\n");
	printf("then it will recompose the mixed radix value back to original RNS value.\n\n");
	printf("Enter a positive integer value to convert: ");
	scanf("%s", input);

	printf("the value entered is: %s\r\n", input);
	
	PPM *ppm_tmp = new PPM(0);
	PPM *ppm = new PPM(0);
	ppm->Assign(input);
	cout << "the converted RNS value is: " << ppm->Print(DEC) << endl;
	ppm->PrintDemo(10);
	printf("\nHit any key to convert to Mixed Radix first\r\n\r\n");

	wait_key();
	wait_key();


	mr_digcnt = 0;
	i = 0;
	while((i < ppm->NumDigits)) {

		if(!ppm->Rn[i]->Skip) {
			if(digit=ppm->Rn[i]->Digit) {
				printf("subtracting %d from digit position %d\r\n", digit, i);
				ppm->Sub(digit);
			}
			else {
				printf("digit position %d already zero\r\n", i);
			}

			ppm->PrintDemo(10);
			printf("\r\n\r\n");
			
			mrn[i] = digit;
			mr_digcnt += 1;

			printf("MIXED RADIX DIGIT (position %d): %d\r\n", i, digit);
			printf("\r\n");
//			wait_key();

			

			power = ppm->Rn[i]->GetFullPowMod();

			ppm_tmp->Assign(power);
//			cout << ppm_tmp->Prints() << endl;
			ppm->ModDiv(ppm_tmp);
//			cout << ppm->Prints() << endl;
			printf("dividing by digit position %d modulus %d\r\n", i, power);
			
			ppm->Rn[i]->Skip = 1;
			ppm->PrintDemo(10);
			printf("\r\n");
//			wait_key();
//			MRn.push_back(new MRDigit(digit, power, 0));	// push a real MRN digit
		}
		else {
			printf("digit position %d marked as skipped\r\n", i); 
//			MRn.push_back(new MRDigit(0, power, 1));		// push a skipped digit so we can print, etc.
//			wait_key();
		}

		if(ppm->Zero(i)) {		// leave asap to eliminate leading zeros and wasted time.
					
			break;
		}

		i++;
	}

	printf("\r\n");
	printf("mixed radix number in normal format: ");
	if(mr_digcnt > 0) {
		for(i=mr_digcnt-1; i>=0; i--) {
			printf("%d ", mrn[i]);
		}
	}
	else {
		printf("problem\r\n");
		while(1);
	}
	printf("\r\n");

	wait_key();
	wait_key();

	ppm->Assign(0);		// clear the ppm variable
	printf("reconverting using MRN to RNS conversion\r\n");
	printf("starting with a clear register:\r\n");
	ppm->PrintDemo(10);
	printf("\r\n");

	printf("Now, starting with mrn last digit converted, add it\r\n");
	wait_key();

	mr_digpos = mr_digcnt - 1;
	printf("adding mrn digit %d position %d \r\n", mrn[mr_digpos], mr_digpos);
	ppm->Add(mrn[mr_digpos]);		// add the last converted digit
	ppm->PrintDemo(10);
	wait_key();

	while(mr_digpos > 0) {
		
		mr_digpos -= 1;

		power = ppm->Rn[mr_digpos]->GetFullPowMod();
		printf("multiplying by modulus %d, position %d\r\n", power, mr_digpos);
		ppm->Mult(power);
		ppm->PrintDemo(10);
		printf("\r\n");
		wait_key();
	
		printf("adding mrn digit %d position %d \r\n", mrn[mr_digpos], mr_digpos);
		ppm->Add(mrn[mr_digpos]);		// add the last converted digit
		ppm->PrintDemo(10);
		wait_key();

	}

	printf("\r\n");
	cout << "conversion complete, final result: " << ppm->Print(DEC) << endl;
	ppm->PrintDemo(10);
//	wait_key();

	delete(ppm_tmp);
	delete(ppm);

}

int print_bin(int x, int *bin, int radix, int reverse)
{
int i, digit;
int temp[100];
int dig_cnt = 0;
int digcnt2;

	while((x) && (dig_cnt < 100)) {
		digit = x % radix;
		temp[dig_cnt] = digit;
		x = x - digit;
		x /= radix;
		dig_cnt++;
	}

	digcnt2 = dig_cnt;

	i = 0;
	while(dig_cnt) {		
		printf("%d ", temp[dig_cnt-1]);
		if(reverse) {
			bin[i] = temp[dig_cnt-1];
		}
		else {
			bin[i] = temp[i];
		}
		
		dig_cnt--;
		i++;
	}

	return(digcnt2);
}

// development routine to test binary to RNS fractional scaler/converter
void bin_mult1(void)
{
int i, j, val1;
int digcnt1;	

int bin[100];		// binary input value
int acc[100];		// accumulator (bin2)
int carry[100];		// carry stages
int carry_out[100];
int enable[100];	// enable stages

const int power[100] = {2, 3, 5, 7, 11, 13, 17, 19};
int Modulus[100];
int pwrcnt = 4;

int shiftcnt;
int fract_cnt = pwrcnt;

	printf("Linear Binary to Fractional RNS scaler DEMO\r\n");
	printf("THIS ROUTINE NOT CONVERTED TO APAL LIB\n\n");

	printf("Enter the value1 to convert: ");
	scanf("%x", &val1);

	for(i=0; i<100; i++) {
		bin[i] = 0;
		acc[i] = 0;
		carry[i] = 0;
		carry_out[i] = 0;
		enable[i] = 0;
		Modulus[i] = 1;
	}
	

	digcnt1 = print_bin(val1, acc, 16, 0);
	printf("\r\n");

	for(i=0; i<digcnt1; i++) printf("%x", acc[i]);
	printf("\r\n");

	wait_key();

	shiftcnt = pwrcnt+digcnt1;

	for(i=0; i<shiftcnt+1; i++) {

		for(j=shiftcnt-1; j>0; j--) {		// run through multiplicand and process carry registers and power registers
			carry[j] = carry_out[j-1];
			Modulus[j] = Modulus[j-1];
		}
		if(i < pwrcnt) {
			Modulus[0] = power[i];
		}
		else {
			Modulus[0] = 1;
		}
		carry[0] = 0;

		printf("modulus: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", Modulus[j]);
		printf("\r\n");
		wait_key();
		printf("    acc: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", acc[j]);
		printf("\r\n");
		wait_key();
		printf("  carry: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", carry[j]);
		printf("\r\n");
		wait_key();
		
		for(j=0; j<shiftcnt; j++) {
			acc[j] = Modulus[j]*acc[j]+carry[j];
			carry_out[j] = (acc[j] >> 4) & 0xf;
			acc[j] = acc[j] & 0xf;
		}

		printf("iteration %d\r\n", i);
		for(j=0; j<shiftcnt; j++) printf("%x ", acc[j]);
		printf("\r\n");


		wait_key();

	}

	wait_key();
}

// development routine to test binary to RNS fractional scaler
void bin_mult2(void)
{
PPM *ppm_acc = new PPM(0);
int i, j, val1;
int digcnt1;	

int bin[100];		// binary input value
int acc[100];		// accumulator (bin2)
int carry[100];		// carry stages
int carry_out[100];
int enable[100];	// enable stages

const int power[100] = {2, 3, 5, 7, 11, 13, 17, 19};
int Modulus[100];
int pwrcnt = 4;

int shiftcnt;
int fract_cnt = pwrcnt;

	printf("Linear Binary to RNS scaler DEMO\r\n");

	printf("Enter the value1 to convert: ");
	scanf("%x", &val1);

	for(i=0; i<100; i++) {
		bin[i] = 0;
		acc[i] = 0;
		carry[i] = 0;
		carry_out[i] = 0;
		enable[i] = 0;
		Modulus[i] = 1;
	}
	

	digcnt1 = print_bin(val1, acc, 16, 0);
	printf("\r\n");

	for(i=0; i<digcnt1; i++) printf("%x", acc[i]);
	printf("\r\n");

	wait_key();

//	shiftcnt = pwrcnt+digcnt1;

	shiftcnt = 4;

	for(i=0; i<8; i++) {

		printf("iteration %d\r\n", i);
//		for(j=0; j<shiftcnt; j++) printf("%x ", acc[j]);
//		printf("\r\n");


//		printf("mod_out: %d, carry_out: %d \n", Modulus[4], carry[4]);

		printf("modulus: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", Modulus[j]);
		printf("\r\n");
//		wait_key();
		printf("    acc: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", acc[j]);
		printf("\r\n");
//		wait_key();
		printf("  carry: ");
		for(j=0; j<shiftcnt; j++) printf("%x ", carry[j]);
		printf("\r\n");
		wait_key();

		ppm_acc->Mult(Modulus[shiftcnt-1]);
		ppm_acc->Add(carry[shiftcnt-1]);
		cout << ppm_acc->Prints() << "  = " << ppm_acc->Print(DEC) << endl;


		for(j=shiftcnt; j>0; j--) {		// run through multiplicand and process carry registers and power registers
			carry[j] = carry_out[j-1];
			Modulus[j] = Modulus[j-1];
		}
		if(i < pwrcnt) {
			Modulus[0] = power[i];
		}
		else {
			Modulus[0] = 1;
		}
		carry[0] = 0;


		for(j=0; j<shiftcnt; j++) {
			acc[j] = Modulus[j]*acc[j]+carry[j];
			carry_out[j] = (acc[j] >> 4) & 0xf;
			acc[j] = acc[j] & 0xf;
		}




		wait_key();

	}

	ppm_acc->Add(1);
	cout << ppm_acc->Prints() << "  = " << ppm_acc->Print(DEC) << endl;

	wait_key();
}


void mrconvert3_demo(void)
{
char val[100];
int i, j;
int digit, mr_digcnt;
int power = 0;	
int mrn[25];
int Modulus[25];
int bin[25];
int carry[25];
int carry_out[25];
int mod_op[25];

	printf("CONVERT RNS TO BINARY DEMO\r\n");
	printf("This demonstrates a multiply, carry and save strategy for conversion.\n");
	printf("A linear conversion does not suffer effects of increasing carry.\n");
	printf("The RNS value is first converted to mixed radix, \n");
	printf("then the linear conversion is performed.\n");
	printf("Simulates Figures 21A & 21B of Patent #US20130311532\n\n");
	printf("Enter an unsigned value to convert: ");
	scanf("%s", &val);

	printf("the string value entered is: %s\r\n", val);
	
	PPM *ppm_tmp = new PPM(0);
	PPM *ppm = new PPM(0);
	ppm->Assign(val);
	cout << "binary equivalent of RNS start value is: " << ppm->Print(BIN) << endl;
	printf("the RNS value is:\r\n");
	ppm->PrintDemo();
	printf("\r\n");

	wait_key();
	wait_key();

	mr_digcnt = 0;
	i = 0;
	while((i < ppm->NumDigits)) {

		if(!ppm->Rn[i]->Skip) {
			if(digit=ppm->Rn[i]->Digit) {
				printf("subtracting %d from digit position %d\r\n", digit, i);
				ppm->Sub(digit);
			}
			else {
				printf("digit position %d already zero\r\n", i);
			}

			ppm->PrintDemo();
			printf("\r\n\r\n");
			
			mrn[i] = digit;
			mr_digcnt += 1;

			printf("MIXED RADIX DIGIT (position %d): %d\r\n", i, digit);
			printf("\r\n");
//			wait_key();

			
			power = ppm->Rn[i]->GetFullPowMod();
			Modulus[i] = power;

			ppm_tmp->Assign(power);
//			cout << ppm_tmp->Prints() << endl;
			ppm->ModDiv(ppm_tmp);
//			cout << ppm->Prints() << endl;
			printf("dividing by digit position %d modulus %d\r\n", i, power);
			
			ppm->Rn[i]->Skip = 1;
			ppm->PrintDemo();
			printf("\r\n");

		}
		else {
			printf("digit position %d marked as skipped\r\n", i); 

		}

		if(ppm->Zero(i)) {		// leave asap to eliminate leading zeros and wasted time.
			
			Modulus[i] = 0;		// end modulus should be sero
			break;
		}

		i++;
	}

	printf("\r\n");
	printf("mixed radix number in normal format: ");
	if(mr_digcnt > 0) {
		for(i=mr_digcnt-1; i>=0; i--) {
			if(i == mr_digcnt-1) {
				printf("<%d,1> ", mrn[i]);
			}
			else {
				printf("<%d,%d> ", mrn[i], Modulus[i]);
			}
		}
	}
	else {
		printf("problem\r\n");
		while(1);
	}
	printf("\r\n");


	wait_key();
	printf("NEXT: converting MRN to binary using linear conversion\r\n");
	wait_key();

	ppm->Assign(0);		// clear the ppm variable

	for(i=0; i<25; i++) {
		carry[i] = 0;
		carry_out[i] = 0;
		bin[i] = 0;
		mod_op[i] = 0;
	}



	for(i=mr_digcnt-1; i>=0; i--) {
		printf("<%d,%d> \r\n", mrn[i], Modulus[i]);
		for(j=mr_digcnt-1; j>0; j--) {
			carry[j] = carry_out[j-1];
			mod_op[j] = mod_op[j-1];
		}
		carry[0] = mrn[i];
		mod_op[0] = Modulus[i];

		printf("Mod: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", mod_op[j]);
		}
		printf("\r\n");

		printf("Bin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", bin[j]);
		}
		printf("\r\n");

		printf("Cin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", carry[j]);
		}
		printf("\r\n");
		wait_key();

		for(j=0; j<mr_digcnt; j++) {
			bin[j] = mod_op[j]*bin[j]+carry[j];
			carry_out[j] = (bin[j] >> ModTable::q) & get_binary_mask(ModTable::q);
			bin[j] = bin[j] & get_binary_mask(ModTable::q);
		}

		printf("Bin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", bin[j]);
		}
		printf("\r\n");

		printf("Cout: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", carry_out[j]);
		}
		printf("\r\n");

		wait_key();
	}

	for(i=mr_digcnt-1; i>=0; i--) {
		printf("<0,1> \r\n");
		for(j=mr_digcnt-1; j>0; j--) {
			carry[j] = carry_out[j-1];
			mod_op[j] = mod_op[j-1];
		}
		carry[0] = 0;
		mod_op[0] = 1;

		printf("Mod: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", mod_op[j]);
		}
		printf("\r\n");				
		
		printf("Bin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", bin[j]);
		}
		printf("\r\n");

		printf("Cin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", carry[j]);
		}
		printf("\r\n");
		wait_key();

		for(j=0; j<mr_digcnt; j++) {
			bin[j] = mod_op[j]*bin[j]+carry[j];
			carry_out[j] = (bin[j] >> ModTable::q) & get_binary_mask(ModTable::q);
			bin[j] = bin[j] & get_binary_mask(ModTable::q);
		}	
		
		printf("Bin: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", bin[j]);
		}
		printf("\r\n");

		printf("Cout: ");
		for(j=0; j<mr_digcnt; j++) {
			printf("%x ", carry_out[j]);
		}
		printf("\r\n");

		wait_key();

	}

	printf("\r\n");
	printf("binary converted answer is: ");
	for(i=mr_digcnt-1; i>=0; i--) {
		if(i == (mr_digcnt-1)) {
			print_bin(bin[i], ModTable::q, 1);
		}
		else {
			print_bin(bin[i], ModTable::q, 0);
		}
	}
	
	printf("\r\n");
	printf("\r\n");
	wait_key(); 

//	ppm->Restore();
	delete(ppm_tmp);
	delete(ppm);

}

void extend_demo(void)
{
char input[100];
int i, val, skip;
int digit, mr_digcnt, mr_digpos;
int power = 0;	
int mrn[100];
int mrn_power[100];

	printf("DIGIT EXTEND DEMO\r\n");
	printf("This demo routine performs base extension on multiple digits using MRC.\n");
	printf("See Patent #US 20130311532 figures 10A, 10B.\n\n");
	
	printf("Enter the value to start: ");
	scanf("%d", &val);

	printf("the value entered is: %d\r\n", val);
	
	PPM *ppm_tmp = new PPM(0);
	PPM *ppm = new PPM((__int64) val);
//	PPM *ppm2 = new PPM(val);
	printf("the RNS value is:\r\n");
	ppm->PrintDemo();
	printf("\r\n");


	printf("select the digit position to skip or q\r\n");
	scanf("%s", input);
	while(input[0] != 'q') {

		skip = atoi(input);
		if(skip < ppm->Rn.size()) {
			ppm->Rn[skip]->Skip = 1;
//			ppm2->Rn[skip]->Skip = 1;		// save skip digits backup
			printf("digit position %d marked as skipped\r\n", skip);
			ppm->PrintDemo();
			printf("\r\n");
			wait_key();
		}
		else {
			printf("input invalid\r\n");
			wait_key();
		}
		
		printf("select the digit position to skip or 'q': ");
		scanf("%s", input);

	}

//	wait_key();


//	cout << "ppm at enter: " << ppm->Prints() << endl;
//	ppm->Backup();

	mr_digcnt = 0;
	i = 0;
	while((i < ppm->NumDigits)) {

		if(!ppm->Rn[i]->Skip) {
			if(digit=ppm->Rn[i]->Digit) {
				printf("subtracting %d from digit position %d\r\n", digit, i);
				ppm->Sub(digit);
			}
			else {
				printf("digit position %d already zero\r\n", i);
			}

			ppm->PrintDemo();
			printf("\r\n\r\n");
			
			mrn[mr_digcnt] = digit;
			
			printf("MIXED RADIX DIGIT (position %d): %d\r\n", i, digit);
			printf("\r\n");
			wait_key();

			
			power = ppm->Rn[i]->GetFullPowMod();
			mrn_power[mr_digcnt] = power;
			mr_digcnt += 1;

			ppm_tmp->Assign(power);
//			cout << ppm_tmp->Prints() << endl;
			ppm->ModDiv(ppm_tmp);

			ppm->Rn[i]->SkipDigit();

//			cout << ppm->Prints() << endl;
			printf("dividing by digit position %d modulus %d\r\n", i, power);
			
//			ppm->Rn[i]->Skip = 1;
			ppm->PrintDemo();
			printf("\r\n");
			wait_key();
//			MRn.push_back(new MRDigit(digit, power, 0));	// push a real MRN digit
		}
		else {
			printf("digit position %d marked as skipped\r\n", i); 
//			MRn.push_back(new MRDigit(0, power, 1));		// push a skipped digit so we can print, etc.
			wait_key();
		}

		if(ppm->Zero(i)) {		// leave asap to eliminate leading zeros and wasted time.
					
			break;
		}

		i++;
	}

	printf("\r\n");
	printf("mixed radix number in normal format: ");
	if(mr_digcnt > 0) {
		for(i=mr_digcnt-1; i>=0; i--) {
			printf("%d ", mrn[i]);
		}
	}
	else {
		printf("problem\r\n");
		while(1);
	}
	printf("\r\n");

	wait_key();
	wait_key();

	ppm->Assign(0);		// clear the ppm variable
	printf("reconverting using MRN to RNS conversion\r\n");
	printf("starting with a clear register:\r\n");
	ppm->PrintDemo();
	printf("\r\n");

	printf("Now, starting with mrn last digit converted, add it\r\n");
	wait_key();

	mr_digpos = mr_digcnt - 1;
	printf("adding mrn digit %d position %d \r\n", mrn[mr_digpos], mr_digpos);
	ppm->Add(mrn[mr_digpos]);		// add the last converted digit
	ppm->PrintDemo();
	wait_key();

	while(mr_digpos > 0) {
		
		mr_digpos -= 1;

//		power = ppm->Rn[mr_digpos]->GetPowMod();
		power = mrn_power[mr_digpos];
		printf("multiplying by modulus %d, position %d\r\n", power, mr_digpos);
		ppm->Mult(power);
		ppm->PrintDemo();
		printf("\r\n");
		wait_key();
	
		printf("adding mrn digit %d position %d \r\n", mrn[mr_digpos], mr_digpos);
		ppm->Add(mrn[mr_digpos]);		// add the last converted digit
		ppm->PrintDemo();
		wait_key();

	}

	printf("\r\n");
	printf("conversion complete, final result: %I64d\r\n", ppm->Convert());
	ppm->PrintDemo();
	wait_key();

//	ppm->Restore();
	delete(ppm_tmp);
	delete(ppm);

}



// calcualte n/d with rem = remainder
void div_demo2(void)
{
int error = 0;
char input[100];
PPM *n = new PPM(0);
PPM *d = new PPM(0);
PPM *rem = new PPM(0);

	printf("ARBITRARY DIVIDE OF RNS INTEGERS DEMO\r\n");
	printf("This demo routine computes arbitrary integer division in RNS.\n");
	printf("See Patent #US 20130311532 figures 12A-13D.\n\n");

	printf("Enter Positive Dividend (n) or 'q' to quit (Example: 1234567): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		while(!n->Assign(input)) {
			printf("ERROR: Incorrect RNS string format for conversion ... press any key\n");
			wait_key();
			printf("Enter Positive Dividend (n) (Example: 1234567): ");
			scanf("%s", &input[0]);
		}
		cout << "Dividend (n): " << n->Print(DEC) << endl << endl;
		n->PrintDemo();
		printf("\n");

		printf("Enter Divisor (d) or 'q' to quit: ");
		scanf("%s", &input[0]);
		
		if(strcmp(input, "q")) {

			while(!d->Assign(input)) {
				printf("ERROR: Incorrect RNS string format for conversion ... press any key\n");
				wait_key();
				printf("Enter Divisor (d) or 'q' to quit: ");
				scanf("%s", &input[0]);
			}
			cout << "Divisor (d): " << d->Print(DEC) << endl << endl;
			d->PrintDemo();
			printf("\n");

			n->Clear_Counters();

			n->ena_dplytrc = 1;				// set to one if you want to see internal steps codes of division
			if(n->ena_dplytrc) printf("Division steps: \n");
			error = n->DivStd(d, rem);

			if(!error) {
				cout << endl << "the result: " << n->Print(DEC) << endl;
				cout << "the remainder: " << rem->Print(DEC) << endl << endl;
				n->PrintDemo();
				n->PrintCounters();
			}
			else {
				if(error == DIVIDE_BY_ZERO) {
					printf("ERROR: Divide By Zero\n");
				}
				else if(error == INTERNAL_ERROR) {
					printf("ERROR: Division Failed with Internal Error\n");
					printf("Check number ranges, and contact DSR if args are in range\n");
				}
				else {
					printf("Unknown error\n");
				}

			}

			printf("\n\n");
			printf("Try another division?:\n");
			printf("Enter Dividend (n) or 'q' to quit: ");
			scanf("%s", &input[0]);

		}

	}

	delete rem;
	delete d;
	delete n;

}

void fmult_demo2(void)
{
char input[100];

	printf("FRACTIONAL MULTIPLY DEMO2\r\n");
	printf("This demo illustrates fractional multiplication in RNS.\n");
	printf("It forms the basis for general purpose RNS computation\n");
	printf("See Patent #US20130311532 figures 15A-16C.\n\n");

	SPMF *arg1 = new SPMF(0);
	SPMF *arg2 = new SPMF(0);

	printf("Enter operand A or 'q' to quit (Example: -12.345): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {
		
		arg1->AssignFP(input);
		cout << "arg1: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("Enter operand B: ");
		scanf("%s", &input[0]);

		arg2->AssignFP(input);
		cout << "arg2: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();

		arg1->MultStd(arg2);
		cout << "arg1: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("\n\n");
		printf("Try another fractional multiplication?:\n");
		printf("Enter operand A or 'q' to quit (Example: -12.345): ");
		scanf("%s", &input[0]);

	}

	printf("Fractional Multiply demo terminated\n");
	wait_key();

}

void gcd(PPM* a, PPM* b, PPM* out_gcd)
{
	PPM* rem = new PPM(0);
	PPM* q = new PPM(0);

	while (!b->Zero()) {
		q->Assign(a);
		rem->Assign(0);
		q->DivStd(b, rem);

		a->Assign(b);
		b->Assign(rem);
	}

	out_gcd->Assign(a);

	delete rem;
	delete q;
}

void fpi_rat_demo_old(void)
{
	char input[100];
	int iter_cnt = 0;

	printf("FPI-RAT DEMO\r\n");
	printf("This demo explores fractional accuracy in computation using FPI-RAT iteration.\n");
	printf("This forms some testing for a future paper\n");
	printf("Investigating how RNS fractions can be more accurate.\n\n");

	SPMF* arg1 = new SPMF(0);
	arg1->AssignRatio(1, 3);
	cout << "this is 1/3: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
	arg1->PrintDemo();
	SPMF* arg2 = new SPMF(0);
	arg2->AssignRatio(1, 7);
	cout << "this is 1/7: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
	arg2->PrintDemo();
	SPMF* x_rns = new SPMF(0);
	SPMF* temp = new SPMF(0);
	SPMF* temp2 = new SPMF(0);

	cout << endl << "Let's do an iteration with x0 = 0!" << endl << endl;

	PPM* ppm_result = new PPM(0);
	PPM* ppm_remainder = new PPM(0);
	PPM* ppm_quotient = new PPM(0);
	SPMF* spmf_frac_range = new SPMF(0);
	PPM* ppm_frac_range = new PPM(0);

	PPM* ppm_exp_numerator = new PPM(3);
	PPM* ppm_exp_denominator = new PPM(14);

	spmf_frac_range->AssignUnitPM();
	ppm_frac_range->PPM::Assign(spmf_frac_range);

	PPM* one = new PPM(1);
	PPM* gcd_result = new PPM(0);

	MRN*  mrn = new MRN(x_rns);


	printf("Enter 'q' to quit (Example: -12.345): ");
//	scanf("%s", &input[0]);

	while (strcmp(input, "q")) {

		iter_cnt += 1;

//		arg1->AssignFP(input);
		cout << "arg1: " << arg1->PPM::Print(DEC) << endl; // arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

//		printf("Enter operand B: ");
//		scanf("%s", &input[0]);

//		arg2->AssignFP(input);
		cout << "arg2: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();

		temp->Assign(arg1);
		temp->MultStd(x_rns);
		temp->Add(arg2);

		x_rns->Assign(temp);			// this is the recurrence

		printf("iteration: %d\n", iter_cnt);
		cout << "temp: " << temp->Print(10) << endl << temp->PPM::Print(DEC) << endl;
		temp->PrintDemo();

		temp2->Assign(temp);

		mrn->Assign2(temp);
		cout << "mrn: ";
		mrn->Print();

		spmf_frac_range->AssignUnitPM();
		ppm_frac_range->Assign(spmf_frac_range);
//		ppm_frac_range->GetRange(ppm_frac_range, 8);
		cout << endl << "fractional range is: " << ppm_frac_range->Print(10) << endl;
		ppm_result->PPM::Assign(temp2);
		cout <<         "result is:           " << ppm_result->Print(10) << endl;
		gcd(ppm_frac_range, ppm_result, gcd_result);

//		ppm_frac_range->Mult(ppm_result);
//		ppm_frac_range->DivStd(ppm_exp_denominator, ppm_remainder);
//		cout << endl << "division is: " << ppm_frac_range->Print(10) << endl;
//		cout << "remainder is: " << ppm_remainder->Print(10) << endl;

//		if (ppm_remainder->Zero()) {
//			cout << "perfect fraction detected: " << endl;
//		}

		if (!gcd_result->IsEqual(one)) {
			cout << "RNS fraction un-reduced: " << endl;
			cout << "RNS fraction x/RF GCD: " << gcd_result->Print(10) << endl;
		}

		spmf_frac_range->AssignUnitPM();
		ppm_frac_range->Assign(spmf_frac_range);
		//		ppm_frac_range->GetRange(ppm_frac_range, 8);
		ppm_result->PPM::Assign(temp2);
		ppm_frac_range->Mult(ppm_exp_numerator);
		ppm_frac_range->DivStd(ppm_exp_denominator, ppm_remainder);
		cout << endl << "division is: " << ppm_frac_range->Print(10) << endl;
		cout << "remainder is: " << ppm_remainder->Print(10) << endl;

		if (ppm_remainder->Zero() && ppm_frac_range->IsEqual(ppm_result)) {
			cout << "perfect result detected: " << endl;
		}


		printf("\n\n");
		printf("Hit space bar?:\n");
//		printf("Enter operand A or 'q' to quit (Example: -12.345): ");
//		scanf("%s", &input[0]);
		wait_key();

	}

	printf("FPI-RAT test completed\n");
	wait_key();

}


enum FPIRAT_DetailLevel
{
	FPI_SUMMARY_ONLY = 0,
	FPI_KEY_EVENTS = 1,
	FPI_ITERATIONS = 2,
	FPI_FULL_DETAIL = 3
};

struct FPIRAT_TestCase
{
	const char* name;
	const char* category;

	int a_num;
	int a_den;

	int b_num;
	int b_den;

	int x0_num;
	int x0_den;

	int fx_num;
	int fx_den;

	int max_iters;
	bool expect_exact;
};

struct FPIRAT_Result
{
	const char* name;
	const char* category;

	bool exact_hit;
	int  exact_iter;

	bool locked;
	int  lock_iter;

	bool final_is_exact;

	char final_value_str[128];
	char reduced_ratio_str[128];
};

// ------------------------------------------------------------
// Preserve-input gcd
// ------------------------------------------------------------
void gcd_ppm(const PPM* a_in, const PPM* b_in, PPM* out_gcd)
{
	PPM* a = new PPM(0);
	PPM* b = new PPM(0);
	PPM* q = new PPM(0);
	PPM* rem = new PPM(0);

	a->Assign((PPM*)a_in);
	b->Assign((PPM*)b_in);

	while (!b->Zero()) {
		q->Assign(a);
		rem->Assign(0);
		q->DivStd(b, rem);

		a->Assign(b);
		b->Assign(rem);
	}

	out_gcd->Assign(a);

	delete a;
	delete b;
	delete q;
	delete rem;
}

void get_fractional_range(PPM* rf_out)
{
	SPMF* unit_pm = new SPMF(0);
	unit_pm->AssignUnitPM();
	rf_out->Assign(unit_pm);
	delete unit_pm;
}

void set_spmf_ratio(SPMF* x, int num, int den)
{
	x->AssignRatio(num, den);
}

bool exact_expected_ratio(
	const SPMF* current_value,
	int expected_num,
	int expected_den,
	PPM* rf,
	PPM* work,
	PPM* remainder,
	PPM* ppm_num,
	PPM* ppm_den)
{
	work->Assign(rf);
	ppm_num->Assign(expected_num);
	ppm_den->Assign(expected_den);

	work->Mult(ppm_num);
	remainder->Assign(0);
	work->DivStd(ppm_den, remainder);

	if (!remainder->Zero())
		return false;

	PPM* current_ppm = new PPM(0);
	current_ppm->Assign((PPM*)current_value);

	bool match = work->IsEqual(current_ppm);
	delete current_ppm;
	return match;
}

//void get_reduced_fraction_strings(
//	const SPMF* current_value,
//	PPM* rf,
//	char* ratio_buf,
//	size_t ratio_buf_size)
//{
//	PPM* value_ppm = new PPM(0);
//	PPM* g = new PPM(0);
//	PPM* num = new PPM(0);
//	PPM* den = new PPM(0);
//	PPM* rem = new PPM(0);
//
//	value_ppm->Assign((PPM*)current_value);
//	gcd_ppm(value_ppm, rf, g);
//
//	num->Assign(value_ppm);
//	rem->Assign(0);
//	num->DivStd(g, rem);
//
//	den->Assign(rf);
//	rem->Assign(0);
//	den->DivStd(g, rem);
//
////	snprintf(ratio_buf, ratio_buf_size, "%s / %s",
////		num->Print(10), den->Print(10));
//
//	delete value_ppm;
//	delete g;
//	delete num;
//	delete den;
//	delete rem;
//}

void get_reduced_fraction_strings(
	const SPMF* current_value,
	PPM* rf,
	char* ratio_buf,
	size_t ratio_buf_size)
{
	PPM value_ppm(0);
	PPM g(0);
	PPM num(0);
	PPM den(0);
	PPM rem(0);

	value_ppm.Assign((PPM*)current_value);
	gcd_ppm(&value_ppm, rf, &g);

	num.Assign(&value_ppm);
	rem.Assign(0);
	num.DivStd(&g, &rem);

	den.Assign(rf);
	rem.Assign(0);
	den.DivStd(&g, &rem);

	std::ostringstream oss;
	oss << num.Print(10) << " / " << den.Print(10);

	std::string s = oss.str();

	if (ratio_buf_size > 0) {
		std::strncpy(ratio_buf, s.c_str(), ratio_buf_size - 1);
		ratio_buf[ratio_buf_size - 1] = '\0';
	}
}

// ------------------------------------------------------------
// Run one affine rational fixed-point iteration test
// ------------------------------------------------------------
void run_fpi_rat_test(
	const FPIRAT_TestCase& tc,
	FPIRAT_DetailLevel detail_level,
	FPIRAT_Result* result)
{
	result->name = tc.name;
	result->category = tc.category;
	result->exact_hit = false;
	result->exact_iter = -1;
	result->locked = false;
	result->lock_iter = -1;
	result->final_is_exact = false;
	result->final_value_str[0] = 0;
	result->reduced_ratio_str[0] = 0;

	if (detail_level >= FPI_KEY_EVENTS) {
		cout << "\n============================================================\n";
		cout << "FPI-RAT TEST: " << tc.name << endl;
		cout << "Category: " << tc.category << endl;
		cout << "Iteration: x_{k+1} = (" << tc.a_num << "/" << tc.a_den
			<< ") x_k + (" << tc.b_num << "/" << tc.b_den << ")\n";
		cout << "Initial value: " << tc.x0_num << "/" << tc.x0_den << endl;
		cout << "Expected fixed point: " << tc.fx_num << "/" << tc.fx_den << endl;
		cout << "Max iterations: " << tc.max_iters << endl;
		cout << "============================================================\n";
	}

	SPMF* arg1 = new SPMF(0);
	SPMF* arg2 = new SPMF(0);
	SPMF* x_rns = new SPMF(0);
	SPMF* temp = new SPMF(0);
	SPMF* prev = new SPMF(0);

	PPM* rf = new PPM(0);
	PPM* work = new PPM(0);
	PPM* rem = new PPM(0);
	PPM* ppm_num = new PPM(0);
	PPM* ppm_den = new PPM(0);

	set_spmf_ratio(arg1, tc.a_num, tc.a_den);
	set_spmf_ratio(arg2, tc.b_num, tc.b_den);
	set_spmf_ratio(x_rns, tc.x0_num, tc.x0_den);

	get_fractional_range(rf);

	if (detail_level >= FPI_FULL_DETAIL) {
		cout << "arg1 = " << arg1->Print(10) << endl;
		cout << "arg2 = " << arg2->Print(10) << endl;
		cout << "x0   = " << x_rns->Print(10) << endl;
		cout << "RF   = " << rf->Print(10) << endl;

		cout << "arg1 raw:\n" << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();
		cout << "arg2 raw:\n" << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();
	}

	for (int iter = 1; iter <= tc.max_iters; iter++) {
		prev->Assign(x_rns);

		temp->Assign(arg1);
		temp->MultStd(x_rns);
		temp->Add(arg2);

		x_rns->Assign(temp);

		if (detail_level >= FPI_ITERATIONS) {
			cout << "\niteration: " << iter << endl;
			cout << "x_k = " << x_rns->Print(10) << endl;
		}

		if (!result->exact_hit &&
			exact_expected_ratio(x_rns, tc.fx_num, tc.fx_den,
				rf, work, rem, ppm_num, ppm_den))
		{
			result->exact_hit = true;
			result->exact_iter = iter;

			if (detail_level >= FPI_KEY_EVENTS) {
				cout << "*** exact expected fixed point detected at iteration "
					<< result->exact_iter << " ***" << endl;
			}
		}

		if (!result->locked && x_rns->IsEqual(prev)) {
			result->locked = true;
			result->lock_iter = iter;

			if (detail_level >= FPI_KEY_EVENTS) {
				cout << "*** machine state locked at iteration "
					<< result->lock_iter << " ***" << endl;
			}
		}

		if (detail_level >= FPI_ITERATIONS) {
			get_reduced_fraction_strings(x_rns, rf,
				result->reduced_ratio_str, sizeof(result->reduced_ratio_str));
			cout << "Reduced ratio x/RF = " << result->reduced_ratio_str << endl;
		}

		if (detail_level >= FPI_FULL_DETAIL) {
			cout << "raw value:\n" << x_rns->PPM::Print(DEC) << endl;
			x_rns->PrintDemo();
		}

		if (result->exact_hit && result->locked)
			break;
	}

	snprintf(result->final_value_str, sizeof(result->final_value_str),
		"%s", x_rns->Print(10));

	get_reduced_fraction_strings(x_rns, rf,
		result->reduced_ratio_str, sizeof(result->reduced_ratio_str));

	result->final_is_exact = exact_expected_ratio(
		x_rns, tc.fx_num, tc.fx_den, rf, work, rem, ppm_num, ppm_den);

	if (detail_level >= FPI_KEY_EVENTS) {
		cout << "\n---------------- SUMMARY ----------------\n";
		if (result->exact_hit)
			cout << "Exact expected ratio reached at iteration: "
			<< result->exact_iter << endl;
		else
			cout << "Exact expected ratio NOT reached within iteration limit." << endl;

		if (result->locked)
			cout << "Machine-state lock detected at iteration: "
			<< result->lock_iter << endl;
		else
			cout << "No machine-state lock detected within iteration limit." << endl;

		cout << "Final x_k = " << result->final_value_str << endl;
		cout << "Reduced ratio = " << result->reduced_ratio_str << endl;
		cout << "-----------------------------------------\n";
	}

	delete arg1;
	delete arg2;
	delete x_rns;
	delete temp;
	delete prev;
	delete rf;
	delete work;
	delete rem;
	delete ppm_num;
	delete ppm_den;
}

// ------------------------------------------------------------
// Print compact final summary table
// ------------------------------------------------------------
void print_fpi_rat_summary_table(FPIRAT_Result* results, int count)
{
	cout << "\n\n============================================================\n";
	cout << "FPI-RAT SUMMARY TABLE\n";
	cout << "============================================================\n";

	for (int i = 0; i < count; i++) {
		cout << "[" << i + 1 << "] " << results[i].name << endl;
		cout << "    Category        : " << results[i].category << endl;
		cout << "    Exact hit       : " << (results[i].exact_hit ? "YES" : "NO");
		if (results[i].exact_hit) cout << " @ iter " << results[i].exact_iter;
		cout << endl;

		cout << "    Machine lock    : " << (results[i].locked ? "YES" : "NO");
		if (results[i].locked) cout << " @ iter " << results[i].lock_iter;
		cout << endl;

		cout << "    Final exact     : " << (results[i].final_is_exact ? "YES" : "NO") << endl;
		cout << "    Final value     : " << results[i].final_value_str << endl;
		cout << "    Reduced ratio   : " << results[i].reduced_ratio_str << endl;
		cout << endl;
	}
}

// ------------------------------------------------------------
// Main multi-case demo
// ------------------------------------------------------------
void fpi_rat_demo(void)
{
//	FPIRAT_DetailLevel detail_level = FPI_SUMMARY_ONLY;
	FPIRAT_DetailLevel detail_level = FPI_FULL_DETAIL;

	printf("FPI-RAT DEMO\n");
	printf("This demo explores affine rational fixed-point iteration\n");
	printf("using RNS-APAL fractional arithmetic.\n\n");

	FPIRAT_TestCase tests[] =
	{
		// --------------------------------------------------------
		// Baseline exact-capture cases already working well
		// --------------------------------------------------------
		{ "a=1/3, b=1/5, x0=0, x*=3/10",      "baseline",         1,3,   1,5,   0,1,   3,10,   100, true },
		{ "a=1/3, b=1/7, x0=0, x*=3/14",      "baseline",         1,3,   1,7,   0,1,   3,14,   100, true },
		{ "a=2/5, b=1/7, x0=0, x*=5/21",      "baseline",         2,5,   1,7,   0,1,   5,21,   120, true },
		{ "a=4/15, b=2/9, x0=0, x*=10/33",    "baseline",         4,15,  2,9,   0,1,  10,33,   140, true },

		// --------------------------------------------------------
		// Fixed-point seed tests
		// --------------------------------------------------------
		{ "a=1/3, b=1/5, x0=3/10",            "fixed-point seed", 1,3,   1,5,   3,10,  3,10,    20, true },
		{ "a=1/3, b=1/7, x0=3/14",            "fixed-point seed", 1,3,   1,7,   3,14,  3,14,    20, true },
		{ "a=2/5, b=1/7, x0=5/21",            "fixed-point seed", 2,5,   1,7,   5,21,  5,21,    20, true },
		{ "a=4/15, b=2/9, x0=10/33",          "fixed-point seed", 4,15,  2,9,  10,33, 10,33,    20, true },

		// --------------------------------------------------------
		// Nearby-start tests
		// --------------------------------------------------------
		{ "a=1/3, b=1/5, x0=1/3",             "nearby start",     1,3,   1,5,   1,3,   3,10,   100, true },
		{ "a=1/3, b=1/7, x0=1/7",             "nearby start",     1,3,   1,7,   1,7,   3,14,   100, true },
		{ "a=2/5, b=1/7, x0=1/5",             "nearby start",     2,5,   1,7,   1,5,   5,21,   120, true },
		{ "a=4/15, b=2/9, x0=1/9",            "nearby start",     4,15,  2,9,   1,9,  10,33,   140, true },

		// --------------------------------------------------------
		// Additional low-prime exact cases
		// --------------------------------------------------------
		{ "a=1/5, b=1/3, x0=0, x*=5/12",      "alt exact",        1,5,   1,3,   0,1,   5,12,   100, true },
		{ "a=1/7, b=1/3, x0=0, x*=7/18",      "alt exact",        1,7,   1,3,   0,1,   7,18,   120, true },
		{ "a=1/7, b=1/5, x0=0, x*=7/30",      "alt exact",        1,7,   1,5,   0,1,   7,30,   120, true },
		{ "a=1/5, b=1/7, x0=0, x*=5/28",      "alt exact",        1,5,   1,7,   0,1,   5,28,   120, true },
		{ "a=1/3, b=1/11, x0=0, x*=3/22",     "alt exact",        1,3,   1,11,  0,1,   3,22,   120, true },
		{ "a=1/3, b=1/13, x0=0, x*=3/26",     "alt exact",        1,3,   1,13,  0,1,   3,26,   120, true },
		{ "a=1/11, b=1/3, x0=0, x*=11/30",    "alt exact",        1,11,  1,3,   0,1,  11,30,   100, true },
		{ "a=1/13, b=1/5, x0=0, x*=13/60",    "alt exact",        1,13,  1,5,   0,1,  13,60,   120, true },

		// --------------------------------------------------------
		// Mixed-prime denominator targets
		// --------------------------------------------------------
		{ "a=2/9, b=1/5, x0=0, x*=9/35",      "mixed prime",      2,9,   1,5,   0,1,   9,35,   140, true },
		{ "a=3/10, b=1/7, x0=0, x*=10/49",    "mixed prime",      3,10,  1,7,   0,1,  10,49,   140, true },
		{ "a=4/13, b=1/5, x0=0, x*=13/45",    "mixed prime",      4,13,  1,5,   0,1,  13,45,   140, true },
		{ "a=5/14, b=1/3, x0=0, x*=14/27",    "mixed prime",      5,14,  1,3,   0,1,  14,27,   160, true },
		{ "a=2/11, b=1/13, x0=0, x*=11/117",  "mixed prime",      2,11,  1,13,  0,1, 11,117,   160, true },
		{ "a=3/13, b=1/11, x0=0, x*=13/110",  "mixed prime",      3,13,  1,11,  0,1, 13,110,   160, true },
		{ "a=7/18, b=1/5, x0=0, x*=18/55",    "mixed prime",      7,18,  1,5,   0,1,  18,55,   180, true },

		// --------------------------------------------------------
		// Higher-prime support tests (11, 13, 17, 19)
		// --------------------------------------------------------
		{ "a=1/17, b=1/7, x0=0, x*=17/112",   "higher prime",     1,17,  1,7,   0,1, 17,112,   140, true },
		{ "a=1/19, b=1/5, x0=0, x*=19/90",    "higher prime",     1,19,  1,5,   0,1,  19,90,   140, true },
		{ "a=4/17, b=1/19, x0=0, x*=17/247",  "higher prime",     4,17,  1,19,  0,1, 17,247,   180, true },
		{ "a=5/19, b=1/17, x0=0, x*=19/238",  "higher prime",     5,19,  1,17,  0,1, 19,238,   180, true },

		// --------------------------------------------------------
		// Power-of-two-friendly controls
		// --------------------------------------------------------
		{ "a=1/2, b=1/4, x0=0, x*=1/2",       "binary control",   1,2,   1,4,   0,1,   1,2,    140, true },
		{ "a=1/4, b=1/8, x0=0, x*=1/6",       "binary contrast",  1,4,   1,8,   0,1,   1,6,    160, true },
		{ "a=1/8, b=1/16, x0=0, x*=1/7",      "binary contrast",  1,8,   1,16,  0,1,   1,14,   180, true },
		{ "a=1/2, b=1/7, x0=0, x*=2/7",       "binary contrast",  1,2,   1,7,   0,1,   2,7,    180, true }
	};

	const int num_tests = sizeof(tests) / sizeof(tests[0]);
	FPIRAT_Result* results = new FPIRAT_Result[num_tests];

	for (int i = 0; i < num_tests; i++) {
		run_fpi_rat_test(tests[i], detail_level, &results[i]);
	}

	print_fpi_rat_summary_table(results, num_tests);

	delete[] results;

	printf("\nFPI-RAT test completed\n");
	wait_key();
}

// fractional division by integer division method
void fdiv_int_demo()
{
char input[100];

	printf("FRACTIONAL DIVISION by Integer Div DEMO\r\n");
	printf("This demo routine computes RNS fractional division.\n");
	printf("The called function uses RNS integer division.\n\n");

	SPMF *arg1 = new SPMF(0);
	SPMF *arg2 = new SPMF(0);

	printf("Enter operand Dividend or 'q' to quit (Example: +34.567): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		arg1->AssignFP(input);
		cout << "dividend: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("Enter divisor B: ");
		scanf("%s", &input[0]);

		arg2->AssignFP(input);
		cout << "divisor: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();

		arg1->DivPM(arg2);
		cout << "result: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("\n\n");
		printf("Try another fractional division?:\n");
		printf("Enter operand Dividend or 'q' to quit (Example: +34.567): ");
		scanf("%s", &input[0]);


	}

	printf("fractional division by INT DIV demo terminated\n");
	wait_key();

	delete arg1;
	delete arg2;
}

void finv_int_demo()
{
char input[100];

	printf("FRACTIONAL INVERSE by Integer Div DEMO\r\n");
	printf("This demo routine computes RNS fractional inverse.\n");
	printf("The called function uses RNS integer division.\n\n");

	SPMF *arg1 = new SPMF(0);

	printf("Enter operand or 'q' to quit (Example: 78.65): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		arg1->AssignFP(input);
		cout << "operand: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		arg1->Inverse();
		cout << "inverse: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("\n\n");
		printf("Try another fractional invert?:\n");
		printf("Enter operand or 'q' to quit (Example: 78.65): ");
		scanf("%s", &input[0]);

	}

	printf("Fractional inverse by INT DIV demo terminated\n");
	wait_key();

	delete arg1;

}

// provide demo of square root using the SPMF SquareRoot demo
void square_root_demo()
{
char input[100];

	printf("FRACTIONAL SQUARE ROOT DEMO\r\n");
	printf("This demo routine computes fractional square root.\n");
	printf("The called function uses Newtons method and RNS integer divide.\n\n");


	SPMF *arg1 = new SPMF(0);

	printf("Enter positive operand or 'q' to quit (ex: 600.25): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		arg1->AssignFP(input);
		cout << "operand: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		arg1->Sqrt();
		cout << "square root: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

//		arg1->PPM::Add(1);			// for testing accuracy, does one click get result any more accurate?

		arg1->MultStd(arg1);
		cout << "square the root: " << arg1->Print(10) << endl;
		arg1->PrintDemo();

		printf("\n\n");
		printf("Try another square root?:\n");
		printf("Enter operand or 'q' to quit (ex: 600.25): ");
		scanf("%s", &input[0]);

	}

	printf("Fractional square root demo terminated\n");
	wait_key();

	delete arg1;

}

// ffractional division by multiplication (goldschmidt method)
void fdiv_gold_demo2(void)
{
char input[100];

	printf("GOLDSCHMIDT FRACTIONAL DIVISION DEMO\r\n");
	printf("This demo routine computes RNS fractional division.\n");
	printf("This demo uses a multiplicative Goldschmidt algorithm.\n");
	printf("More can be learned in Patent US 20130311532, figures 18A-18E\n");

	SPMF *arg1 = new SPMF(0);
	SPMF *arg2 = new SPMF(0);

	printf("Enter operand Dividend or 'q' to quit (Example: -600.25): ");
	scanf("%s", &input[0]);

	while(strcmp(input, "q")) {

		arg1->AssignFP(input);
		cout << "dividend: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("Enter divisor B: ");
		scanf("%s", &input[0]);

		arg2->AssignFP(input);
		cout << "divisor: " << arg2->Print(10) << endl << arg2->PPM::Print(DEC) << endl;
		arg2->PrintDemo();

		arg1->GoldDiv(arg2);
		cout << "result: " << arg1->Print(10) << endl << arg1->PPM::Print(DEC) << endl;
		arg1->PrintDemo();

		printf("\n\n");
		printf("Try another fractional division?:\n");
		printf("Enter operand Dividend or 'q' to quit (Example: -600.25): ");
		scanf("%s", &input[0]);

	}

	printf("Goldschmidt fractional division demo terminated\n");
	wait_key();

	delete arg1;
	delete arg2;

}



void fscale_demo(void)
{
int val1, val2;
int position, width;

	SPMF *pmfA = new SPMF(0);
	SPMF *pmfB = new SPMF(0);
	SPMF *temp = new SPMF(0);


//	pmf->GetUnit(one);
	
//	cout << "one: " << one->Print10() << endl;
//	cout << "one: " << one->Prints() << endl;

//	pmf->Add(200654);
//	pmf2->Add(25729703);
	
	printf("NumfractDigits = %d\r\n", pmfA->GetCurFractPos());
	
	printf("Enter operand A numerator: (Example: 123.456)");
	scanf("%d", &val1);

	printf("Enter operand A divisor: ");
	scanf("%d", &val2);

	temp->Assign((__int64)0);
	temp->Add1(val2);

	pmfA->Add1(val1);
	pmfA->DivPM(temp);
	cout << "pmfA = " << pmfA->Print(10) << endl;
	cout << "pmA raw:" << pmfA->Prints() << endl;
	cout << "pmfA int:" << pmfA->PPM::Print(DEC) << endl;

	printf("Enter operand B numerator: ");
	scanf("%d", &val1);

	printf("Enter operand B divisor: ");
	scanf("%d", &val2);

	temp->Assign((__int64)0);
	temp->Add1(val2);

	pmfB->Add1(val1);
	pmfB->DivPM(temp);

	cout << "pmfB = " << pmfB->Print(10) << endl;
	cout << "pmfB raw:" << pmfB->Prints() << endl;
	cout << "pmfB int:" << pmfB->PPM::Print(DEC) << endl;

//	MRN *mrn = new MRN(pmf);
//	printf("the mixed radix conversion of pmf:\r\n");
//	mrn->Print();
//	printf("\r\n");
//	delete mrn;

	position = pmfA->scalePM(pmfA, pmfB, width);			// scale value less than 0.5
//	position = pmf->scalePM(pmf2, width);
//	printf("point pos: %d, 2's width: %d\r\n", position, width);
//	pmf->Rn[0]->PowerValid = width;
//	pmf2->Rn[0]->PowerValid = width;

//	pmf->NumFractDigits = position;
//	pmf2->NumFractDigits = position;

	printf("pmfA = ");
	pmfA->PrintFPM();
	printf("\r\n");

	printf("pmfB = ");
	pmfB->PrintFPM();
	printf("\r\n");

	cout << "pmfA = " << pmfA->Print(10) << endl;
	cout << "pmfB = " << pmfB->Print(10) << endl;
	wait_key();

	delete pmfA;
	delete pmfB;
	delete temp;

}

#define CD_EQUAL 0
#define CD_GREATER 1
#define CD_LESSER 2
void compare_demo(void)
{
//char c[100];
int i, val1, val2;
int digitA, digitB, mr_digcnt;
int power = 0;	
int flag = CD_EQUAL;

	printf("COMPARE (DUAL CONVERSION) DEMO\r\n");
	printf("This demo routine compares two unsigned RNS integers.\n");
	printf("See Patent US 20130311532, figures 9A, 9B, 9C\n");

	printf("This demo compares two unsigned integers ...\n\n");
	printf("Enter UNSIGNED INTEGER Operand A: ");
	scanf("%d", &val1);
	printf("Operand A: %d\r\n", val1);

	printf("Enter UNSIGNED INTEGER Operand B: ");
	scanf("%d", &val2);
	printf("Operand B: %d\r\n", val2);
	
	PPM *ppm_tmp = new PPM(0);
	PPM *ppmA = new PPM((__int64) val1);
	PPM *ppmB = new PPM((__int64) val2);
	printf("\nRNS A value is:\r\n");
	cout << "A value = " << ppmA->Print(10) << endl;
	printf("\n");
	ppmA->PrintDemo();
	printf("\r\n");
	printf("RNS B value is:\r\n");
	cout << "A value = " << ppmB->Print(10) << endl;
	printf("\n");
	ppmB->PrintDemo();
	printf("\r\n");
	printf("This demo will now convert both RNS numbers simultaneously,\n");
	printf("comparing each converted (mixed radix) digit as they are generated.\n");
	printf("press a key to start...\n");
	wait_key();


//	cout << "ppm at enter: " << ppm->Prints() << endl;
//	ppm->Backup();

	mr_digcnt = 0;
	i = 0;
	while((i < ppmA->NumDigits) && (wait_key() != 'q')) {

		if(!ppmA->Rn[i]->Skip && !ppmB->Rn[i]->Skip) {

			printf("Step 1: subtract the current digits from each RNS value ...\n");
			wait_key();
			
			if(digitA=ppmA->Rn[i]->Digit) {
				printf("subtracting %d from digit position %d (A)\r\n", digitA, i);
				ppmA->Sub(digitA);
			}
			else {
				printf("digit position %d already zero\r\n", i);
			}

			ppmA->PrintDemo();
			printf("\r\n\r\n");

			if(digitB=ppmB->Rn[i]->Digit) {
				printf("subtracting %d from digit position %d (B)\r\n", digitB, i);
				ppmB->Sub(digitB);
			}
			else {
				printf("digit position %d already zero\r\n", i);
			}

			ppmB->PrintDemo();
			printf("\r\n\r\n");

//			mrn[i] = digit;
//			mr_digcnt += 1;
			printf("press a key\n");
			wait_key();
			printf("Step 2: comparing the subtracted digits and track result\n");

			if(digitA > digitB) {
				printf("A > B, since %d > %d \r\n", digitA, digitB);
				flag = CD_GREATER;
			}
			else if(digitB > digitA) {
				printf("A < B, since %d < %d \r\n", digitA, digitB);
				flag = CD_LESSER;
			}
			else {
				printf("no equality change\r\n");
				if(flag == CD_EQUAL) {
					printf("so A = B so far\n");
				}
				else if(flag == CD_GREATER) {
					printf("so A > B\n");
				}
				else if(flag == CD_LESSER) {
					printf("so A < B\n");
				}
			}
			printf("press a key\n");
			wait_key();

			printf("\nStep 3: Dividing both RNS numbers by the subtracted digit modulus...\n");

			power = ppmA->Rn[i]->GetFullPowMod();

			ppm_tmp->Assign(power);
//			cout << ppm_tmp->Prints() << endl;
			ppmA->ModDiv(ppm_tmp);
			ppmB->ModDiv(ppm_tmp);
//			cout << ppm->Prints() << endl;
			printf("dividing A by digit modulus %d\r\n", power);
			ppmA->PrintDemo();
			printf("\r\n");
			printf("dividing B by digit modulus %d\r\n", power);
			ppmB->PrintDemo();
			printf("\r\n");
			ppmA->Rn[i]->Skip = 1;
			ppmB->Rn[i]->Skip = 1;
						
		}
		else {
			printf("digit position %d marked as skipped\r\n", i); 
			wait_key();
		}

		printf("\npress a key\n");
		wait_key();
		printf("perform a termination state check...\n");
		if((ppmA->Zero(i)) && (!ppmB->Zero(i))) {		// leave asap to eliminate leading zeros and wasted time.
			
			printf("A < B by early termination\r\n");
			break;
		}
		else if((!ppmA->Zero(i)) && (ppmB->Zero(i))) {

			printf("A > B by early termination\r\n");
			break;
		}
		else if((ppmA->Zero(i)) && (ppmB->Zero(i))) {

			if(flag == CD_GREATER) {
				printf("A > B by digit compare\r\n");
			}
			else if(flag == CD_LESSER) {
				printf("A < B by digit compare\r\n");
			}
			else {
				printf("A = B\r\n");
			}

			break;
		}
		else {
			printf("\ncomparison not complete... \n\npress a key\n");

		}

		i++;
	}

	printf("\ncomparison COMPLETE\n");
	wait_key();


//	ppm->Restore();
	delete(ppm_tmp);
	delete(ppmA);
	delete(ppmB);

}

// demo illustrating fractional RNS product summation
void fprod_sum(void)
{
unsigned int i, num;
char val[100];
SPMF *ppm_array[20];
PPM *prod = new PPM(0);
PPM *sum = new PPM(0);
SPMF *final = new SPMF(0);

	printf("PRODUCT SUMMATION DEMO\n");
	printf("This demo routine computes fractional RNS product summations.\n");
	printf("This illustrates the strength of separating PAC from normalization steps.\n");
	printf("More can be learned in Patent US 20130311532, figure 16D\n\n");

	while(1) {
	printf("FOLLOW DIRECTIONS CAREFULLY, press 'c' to continue, 'q' to quit\n");
	scanf("%s", val);
	if(!strcmp(val, "c")) {
		while(1) {

			printf("enter the number of product sums to calculate (max 10): ");
			scanf("%d", &num);

			if(num > 10) {
				printf("too many product sums selected, try again\n");
				wait_key();
			}
			else {
				break;
			}
		}
	}
	else {
		return;
	}

	// initialize all the members of the ppm_array
	for(i=0; i<num*2; i++) {
		ppm_array[i] = new SPMF(0);
	}

	printf("\n");
	for(i=0; i<num; i++) {

		printf("enter a fractional value for product %d(A): ", i);
		scanf("%s", val);
		if(!ppm_array[i*2]->AssignFP(val)) {
			do {
				printf("Error: invalid format, try again\n");
				printf("enter a fractional value for product %d(A): ", i);
				scanf("%s", val);
			} while(!ppm_array[i*2]->AssignFP(val));
		}

		printf("enter a fractional value for product %d(B): ", i);
		scanf("%s", val);
		if(!ppm_array[i*2+1]->AssignFP(val)) {
			do {
				printf("Error: invalid format, try again\n");
				printf("enter a fractional value for product %d(B): ", i);
				scanf("%s", val);
			} while(!ppm_array[i*2+1]->AssignFP(val));

		}

		printf("\n");

	}

	printf("printing the entered value pairs: \n\n");

	for(i=0; i<num; i++) {
		cout << ppm_array[i*2]->Print(DEC) << " , " << ppm_array[i*2+1]->Print(DEC) << endl;

	}

	printf("hit a key to process the product summation:\n");
	wait_key();
	wait_key();

	sum->Assign(0);
	for(i=0; i<num; i++) {

		prod->Assign(ppm_array[i*2]);		//  this is PAC
		prod->Mult(ppm_array[i*2+1]);		// this is PAC
		sum->Add(prod);						// this is PAC

	}

	final->PPM::Assign(sum);
	final->I2N_Convert();			// this is the final normalization step

	cout << "The product summation is: " << final->Print(DEC) << endl;
	wait_key();
	}

	for(i=0; i<num*2; i++) {
		delete ppm_array[i];
	}

}


// print menu for RNS demo example
void print_menu(void)
{

	printf("RNS Demo Menu\n\n");
	printf("1. increment an RNS number			BASIC\r\n");
	printf("2. decrement an RNS number			BASIC\r\n");
	printf("3. add two RNS numbers				BASIC\r\n");
	printf("4. subtract two RNS numbers			BASIC\r\n");
	printf("5. multiply two RNS numbers			BASIC\r\n");

	printf("6. convert RNS to MRN				INTERMEDIATE\r\n");
	printf("7. reconvert MRN to RNS				INTERMEDIATE\r\n");
	printf("8. convert RNS to binary			INTERMEDIATE\r\n");
	printf("9. binary to RNS fractional			INTERMEDIATE\r\n");
	printf("a. compare two RNS numbers			INTERMEDIATE\r\n");
	printf("b. digit extend RNS number			INTERMEDIATE\r\n");

	printf("c. divide two rns integers			ADVANCED\r\n");
	printf("d. fractional multiplication			ADVANCED\r\n");
	printf("e. fractional square root ex			ADVANCED\r\n");
//	printf("f. fractional scaling				ADVANCED\r\n");
	printf("g. fractional division by int divide		ADVANCED\n");
	printf("h. fractional inverse by int divide		ADVANCED\n");
	printf("i. fractional divide by mult (Goldschmidt)	ADVANCED\n");
	printf("j. fractional product summation			ADVANCED\n");
	printf("k. FPI-RAT tests\n");

}


// demo routine to illustrate RNS concepts
void pm_alu_tests(void)
{
char c[100];
char *s;


	c[0] = 0;
	s = &c[0];
	while(strcmp(s, "q")) { 

		cls();

		print_menu();

		printf("\r\nEnter choice: ");
	
		scanf("%s", &c[0]);


		switch (c[0]) {
			case '1':
				cls();
//				printf("You typed zero.\r\n");
				inc_demo();
				wait_key();
				break;
			case '2':
				cls();
				dec_demo();
				wait_key();
				break;
			case '3':
				cls();
				add_demo2();
				wait_key();
				break;
			case '4':
				cls();
				sub_demo2();
				wait_key();
				break;
			case '5':
				cls();
				multiply_demo2();
				wait_key();
				break;
			case '6':
				cls();
				mrconvert_demo();
				wait_key();
				break;
			case '7':
				cls();
				mrconvert2_demo();
				wait_key();
				break;
			case '8':
				cls();
				mrconvert3_demo();
				wait_key();
				break;
			case '9':
				cls();
				bin_mult1();
				wait_key();
				break;				
			case 'a':
				cls();
				compare_demo();
				wait_key();
				break;
			case 'b':
				cls();
				extend_demo();
				wait_key();
				break;
			case 'c':
				cls();
				div_demo2();
				wait_key();
				break;
			case 'd':
				cls();
				fmult_demo2();
				wait_key();
				break;
			case 'e':
				cls();
				square_root_demo();
				wait_key();
				break;
			case 'f':
				cls();
//				pmf_class_test();
//				fscale_demo();
				wait_key();
				break;
			case 'g':
				fdiv_int_demo();
				wait_key();
				break;
			case 'h':
				finv_int_demo();
				wait_key();
				break;
			case 'i':
				fdiv_gold_demo2();
				wait_key();
				break;
			case 'j':
				fprod_sum();
				wait_key();
				break;
			case 'k':
				fpi_rat_demo();
				wait_key();
				break;
			default:
				printf("Invalid menu key, try again\r\n");
				wait_key();
				break;
		}
				
//		wait_key();
	
	}


}

