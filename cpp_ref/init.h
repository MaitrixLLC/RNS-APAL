#pragma once


// Bit mapped feature flag words used for mode
#define AUTO_GEN		0x00
#define CUSTOM			0x01
#define NO_POWERS		0x02


// Choices for routine parameter
#define USE_MODDIV_LUT	0
#define USE_BRUTE_LUT	1
#define USE_EXT_EUCL	2

#define WIN10_WIDTH		128
#define WIN78_WIDTH		80

#define MAX_DIGIT   9999999

extern int console_width;

extern bool print_the_remainder;

void init_RNS_APAL(int mode, int routine, int num_digs, int num_frac_digs, int* mod_array, const int* powers_array);
void pm_alu_tests(void);
void print_remainder(bool req_print_remainder);
void change_console_width(int new_console_width);
//void print_modulus(PPM* ppm_base, int num_digs);

bool validate_modulus(int mode, int* mod_array, int num_digs);
bool check_for_negative_numbers(int* mod_array, int num_digs);
bool verify_pairwise_prime(int* mod_array, int num_digs);
bool check_for_mod_power_of_two(int* mod_array, int num_digs);
bool check_for_digs(int* mod_array, int num_digs);
bool check_for_max_dig(int* mod_array, int num_digs);
bool validate_modulus(int mode, int* mod_array, int num_digs);

int gcd(int num_1, int num_2);
