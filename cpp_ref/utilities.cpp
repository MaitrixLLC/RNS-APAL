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

//  Utilities for common use
//

#if defined(__linux__) && !defined(_WIN32)
#include <inttypes.h>
typedef int64_t __int64;
#else

#endif
#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "config.h"
#include "utilities.h"
#include <stdlib.h>
#include <time.h>

#include "ppm.h" // 1/10/2020 added to retrieve the requested number of digits from ModTable


// local prototypes
int prime_index(int prime, int *primes);
void print_primes(int start, int end);
char wait_key(void);
int power(int base, int exp);
int BinaryWidth(__int64 x, __int64 &BinaryPower);


void init_rand(void)
{

	int t = time(NULL);

	srand(t);

	rand();			// burn one !, same number keeps coming up at beginning

}

// returns a random integer from min to max
int get_rand(int min, int max)
{

	int u = (double)rand() / (RAND_MAX + 1) * (max - min + 1) + min;
	return(u);

}


// This function returns the index of the prime number
// Should be replaced by look-up for efficiency
// WORKS ONLY ON FIXED LENGTH PRIME TABLES !!!
int prime_index(int prime, int *primes)
{
int i;

	for(i=0; i < ModTable::returnNumDigits(); i++) {

		if(prime == primes[i]) {
			return(i);
		}
	}

	printf("ERROR detected\r\n");
	wait_key();
	return(-1);		// error, return -1

}

// Print x in binary format, numbits number of digits formatting, including feature to suppress leading '0' digits
void print_bin(__int64 x, int numbits, int suppress_leading)
{
unsigned long long mask = 0x01;

	mask = mask << (numbits - 1);

	do {

		if(x & mask) {
			printf("1");
			suppress_leading = 0;
		}
		else {
			if(!suppress_leading) {
				printf("0");
			}
			else if(numbits == 1) {
				printf("0");
			}

		}

		mask = mask >> 1;
		numbits -= 1;

	} while(numbits);

}


char wait_key(void)
{
	char c[2] = {0,0};

	scanf("%c", &c[0]);

	return(c[0]);	


}

// get the binary mask with a one set in the number of rightmost 'bits' number of bits
unsigned int get_binary_mask(int bits)
{
unsigned int i;
unsigned int mask = 0;

	
	if(bits <= 0) {
		return (0);
	}

	for(i=0; i<bits; i++) {

		mask = (mask << 1) | 1;

	}

	return(mask);

}

void init_prime(int *primes)
{
int i, j, num_primes, pflag;

	i = 2;
	num_primes = 0;
	while(num_primes < ModTable::returnNumDigits()) {	// PUT THIS IN STATIC MEMBER LATER!!
		
		j = 2;
		pflag = 1;				// assume the number is prime

		while((j*j) <= i) {		// test until the denominator squared > test value

			if(!(i % j)) {
				pflag = 0;
				break;
			}

			j++;
			
		}

		if(pflag) {
		
			*(primes+num_primes) = i;
			num_primes++;

		}

		i += 1;

	}		// end while

}


// this version adds the num arg for new PPM class design
void init_prime(int *primes, int num)
{
int i, j, num_primes, pflag;

	i = 2;
	num_primes = 0;
	while(num_primes < num) {	// PUT THIS IN STATIC MEMBER LATER!!
		
		j = 2;
		pflag = 1;				// assume the number is prime

		while((j*j) <= i) {		// test until the denominator squared > test value

			if(!(i % j)) {
				pflag = 0;
				break;
			}

			j++;
			
		}

		if(pflag) {
		
//			*(primes+num_primes) = i;			num_primes++;
			primes[num_primes] = i;
			num_primes++;

		}

		i += 1;

	}		// end while

}

void print_primes(__int64 start, __int64 end)
{
__int64 i, j, num_primes;
int pflag;

	i = start;
	num_primes = 1;
	while(i <= end) {	// Print all primes between start and end
		
		j = 2;
		pflag = 1;				// assume the number is prime

		while((j*j) <= i) {		// test until the denominator squared > test value

			if(!(i % j)) {
				pflag = 0;
				break;
			}

			j++;
			
		}

		if(pflag) {
		
			printf("%I64d: %I64d\n", num_primes, i);
			num_primes += 1;
		}

		i += 1;

	}		// end while

}


// tests for primality the brute force way
int is_prime(__int64 val)
{
__int64 div_test = 2;

	
	if(val < 2) {
		printf("test value out of range\n");
		return(0);
	}
	else {
//		printf("value testing: %I64d\n", val);
	}

	while(((val % div_test) != 0) && (div_test*div_test < val)) {

		div_test = get_next_prime(div_test);

	}

	if((val % div_test) == 0) {
//		printf("value is NOT prime, factor = %I64d\n", div_test);
		return(0);
	}
	else {
//		printf("the value IS prime\n");
		return(1);
	}


}


// return the next prime starting from start
__int64 get_next_prime(__int64 start)
{
__int64 i, j, num_primes;
int pflag;

	i = start+1;
	num_primes = 1;
	while(1) {	// Print all primes between start and end
		
		j = 2;
		pflag = 1;				// assume the number is prime

		while((j*j) <= i) {		// test until the denominator squared > test value

			if(!(i % j)) {
				pflag = 0;
				break;
			}

			j++;
			
		}

		if(pflag) {

			return(i);
//			printf("%d: %d\n", num_primes, i);
//			num_primes += 1;
		}

		i += 1;

	}		// end while

}

// return the previous prime starting from start
__int64 get_prev_prime(__int64 start)
{
__int64 i, j, num_primes;
int pflag;

	i = start-1;
	num_primes = 1;
	while(1) {	// Print all primes between start and end
		
		j = 2;
		pflag = 1;				// assume the number is prime

		while((j*j) <= i) {		// test until the denominator squared > test value

			if(!(i % j)) {
				pflag = 0;
				break;
			}

			j++;
			
		}

		if(pflag) {

			return(i);
//			printf("%d: %d\n", num_primes, i);
//			num_primes += 1;
		}

		i -= 1;

		if(i < 2) return 0;

	}		// end while

}

// return the next power prime starting from last prime or power prime
__int64 get_next_power_prime(__int64 start, int q)
{
__int64 next_prime, power_prime;
__int64 BinaryPower;
int width;

	if(start > 1) {
		next_prime = 2;
		while(next_prime < start) {
		
			if(!(start % next_prime)) {
				break;
			}

			next_prime = get_next_prime(next_prime);
		}
	}
	else {
		next_prime = 1;
	}

	power_prime = next_prime = get_next_prime(next_prime);

	if((width = BinaryWidth(next_prime, BinaryPower) > q)) {
		return(0);
	}

	if(power_prime == 2) q += 1;

	width = BinaryWidth(power_prime*next_prime, BinaryPower);

	while(width <= q) {
		power_prime *= next_prime;
		width = BinaryWidth(power_prime*next_prime, BinaryPower);
	}

	return(power_prime);

}


int power(int base, int exp)
{
int power = 1;


	while(exp) {
		power *= base;
		exp--;
	}
	
	return(power);
}

// returns how many times val divides power successively.
int get_num_of_powers(int power, int val)
{
int i = 0;

	while(power = power/val) i += 1;

	return(i);
}

// return the minimum binary data width required to store the value x
// return the binary power in the BinaryPower argument
int BinaryWidth(__int64 x, __int64 &BinaryPower)
{
int width = 0;


	BinaryPower = 1;
	while(BinaryPower <= x) {
										// ebo changed to only less than, to accomodate binary modulus
		BinaryPower *= 2;
		width += 1;

	}

	return(width);
}

// return the minimum binary data width required to store the value x
// return the binary power in the BinaryPower argument
int BinaryWidthD(double x, double &BinaryPower)
{
int width = 1;


	BinaryPower = 2.0;
	while(BinaryPower <= x) {
	
		BinaryPower *= 2;
		width += 1;

	}

	return(width);
}

// return the value of the range of a digit_num length RNS number
// and return in double format, also returns most significant rns digit value
// via the rns_msd argument
long double get_range(int digit_num, int &rns_msd)
{
long double range = 2.0;
int prime_num = 2;

	if(digit_num == 1) {
		rns_msd = 2;
		return(2.0);
	}

	for(int i=1; i<digit_num; i++) {

		range *= (prime_num = get_next_prime(prime_num));

	}

	rns_msd = prime_num;

	return(range);
}

// return the value of the range of a digit_num length RNS number
// and return in double format, also returns most significant rns digit value
// via the rns_msd argument
__int64 get_power_range(int digit_num, int &rns_msd, int &q)
{
__int64 range = 1.0;
__int64 prime_num = 1;

	if(digit_num == 1) {
		rns_msd = 2;
		return(2.0);
	}

	for(int i=0; i<digit_num; i++) {
 
		prime_num = get_next_power_prime(prime_num, q);
		range *= prime_num;
	
	}

	rns_msd = prime_num;

	return(range);
}



double get_fact(int digit_num)
{
double fact = 1.0;


	for(int i=1; i<=digit_num; i++) {

		fact *= (double)(i);

	}

	return(fact);
}

/* Modular inverse using extended euclidean

function inverse(a, n)
t := 0;     newt := 1;
r := n;     newr := a;
while newr != 0
quotient := r div newr
(t, newt) := (newt, t - quotient * newt)
(r, newr) := (newr, r - quotient * newr)
if r > 1 then return "a is not invertible"
if t < 0 then t := t + n
return t
*/

// calculate the modular inverse using extended Euclid GCD
// routine may need a large division at first iteration, then evens out to trivial divisions

__int64 get_inv(__int64 a, __int64 mod)
{
	__int64 inv, new_inv;
	__int64 r, new_r;
	__int64 quot, temp;


	inv = 0;
	new_inv = 1;

	r = mod;			// start out the Euclidean args (This is not reversible)
	new_r = a;

	quot = 0;

	while (new_r != 0) {

		//		printf("r: %I64d, new_r: %I64d, quot: %I64d\n", r, new_r, quot);

		quot = r / new_r;

		temp = r;				// parallel assignment
		r = new_r;							// looks to be previous remainder
		new_r = temp - quot * new_r;		// new_r looks to be the remainder

		temp = inv;				// parallel assignment for inverse invariant
		inv = new_inv;
		new_inv = temp - quot * new_inv;


	}

	if (r > 1) {
		printf("a is not invertible\n");
		return(0);
	}

	if (inv < 0) inv += mod;

	return(inv);

}

