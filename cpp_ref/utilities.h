#ifndef UTILITIES
#define UTILITIES


extern void init_rand(void);
extern int get_rand(int min, int max);

extern int prime_index(int prime, int *primes);
extern char wait_key(void);
extern void init_prime(int *primes);
extern void print_primes(__int64 start, __int64 end);
extern __int64 get_next_prime(__int64 start);
extern int power(int base, int exp);					// need integer version, not doubles, etc.
extern int get_num_of_powers(int power, int val);
extern void init_prime(int *primes, int num);

extern int BinaryWidth(__int64 x,__int64 &BinaryPower);		// returns minimum binary width to represent the value x, used to build LUT
extern long double get_range(int digit_num, int &rns_msd);
extern int BinaryWidthD(double x, double &BinaryPower);
extern double get_fact(int digit_num);
extern unsigned int get_binary_mask(int bits);

extern __int64 get_next_power_prime(__int64 start, int q);
extern __int64 get_power_range(int digit_num, int &rns_msd, int &q);

extern int is_prime(__int64 val);
extern __int64 get_prev_prime(__int64 start);
extern void print_bin(__int64 x, int numbits, int suppress_zero);

extern __int64 get_inv(__int64 a, __int64 mod);
extern int calc_num_adders(int modulus, int mod_width, int table_width);


// used to analyze CFR properties
struct cfr_data {

	double ave_decs;
	double ave_extends;
	double ave_moddivs;

	long num_decs;
	long num_extends;
	long num_moddivs;

};


#endif

