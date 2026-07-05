#ifndef PPM_CLASS_MODULE
#define PPM_CLASS_MODULE

#include <vector>
using std::vector;
#include <iostream>
#include <string>
#include "config.h"
#include "init.h"
#include "utilities.h"

using namespace std;

class MRN;		// use this instead of mrn.h to solve the circular reference problem


#define EQUAL		0		// used right now for integrated comparison within some functions
#define GREATER		1
#define LESSER		2

#define DIVIDE_BY_ZERO	 -1
#define INTERNAL_ERROR	 -2
#define SQRT_OF_NEGATIVE -3

#define INVALID_MODULUS -1

// use these for radix specifier argument for print routines
#define BIN	2
#define DEC	10
#define HEX	16

#define TRUE	1
#define FALSE	0


enum COUNTERS {DIV_COUNT, SUB_COUNT, ADD_COUNT, MULT_COUNT, DEC_COUNT, LOOP_COUNT, BASE_EXTEND, EXTEND_CLK, COMPARE_COUNT, COMPARE_CLK, ADJUST_TYPE, TOTAL_CLK};



// create a class that contains the static members I need, so memory doesn't leak
class ModTable {


public:

	ModTable(int mode, int rountine, int num_digs, int num_frac_digs, int* modulus, const int* num_powers);		// constructor

	static int ModTableInit;
	static vector<vector<vector<short>>> arrayDivTbl;
	static vector<vector<vector<short>>> ModDivTbl;				// efficient method for ModDiv LUT using multiplicative inverses
	static int *primes;
	static int q;

	int BaseClass;


	friend class PPM;
	friend class PPMDigit;

	static int GetDigitMod(int index, int &pwr, int q);		// General method which calls one of the two version below based upon #define Custom_Powers
															// Get the custom Digit Modulus (when using #define CUSTOM_MODULUS 1)
	static int GetCustDigitMod(int index, int &pwr);		// returns the initializing digit modulus (i.e. including power)
	static int GetAutoDigitMod(int index, int &pwr, int q);		// returns the initializing digit modulus using automatically generated modulus
															// which is the largest power that fits into the largest primes number digit width

	static int GetMaxDigWidth(long long &maxdigit, int req_num_digs);

	static int returnNumDigits(void);
	static int returnNumFracDigits(void);
	static int returnMode(void);
	static int returnRoutine(void);

private:

	static int req_num_digs;			// this is passed in to ModTable, and then it's stored here
	static int req_num_frac_digs;       // this is passed in to Modtable, and then stored here
	static int req_num_ppm_digs;
	static int req_mode;				  
	static int req_routine;
	

	vector<int> req_modulus;			// requested modulus array that has been copied into this class
	vector<int> req_powers;				// requested power array

};



// create a RNS digit class, RNS number will be a vector of these
class PPMDigit {

public:
	
	int Index;			// PPMDigit object index, also used to index the arrayDivTable and prime number tables  (starting with zero)
	
	int Digit;			// the actual PPM digit
	int DigitCopy;		// used to retore the value during destructive operations
	int Skip;			// if non-zero, the digit is skipped in certain operations	

	int Modulus;		// Base modulus of the digit;  if power greater than 1, then actual modulus is Modulus^Power
	int Power;			// how many powers supported, start with one, through N.  DEFINES THE NORMAL POWER (If reduced, this defines a new derived number system)
	int PowerValid;		// how many powers are valid, remaining powers are skipped or undefined, partial modulus defined if (Power - PowerValid) is no-zero, but digit not skipped
						// once PowerValid goes to zero, the skip digit flag is automatically set, and a base extend is required to recover digit value and full (normal) power
	int NormalPower;	// this is the base class normalized power, that is assigned at creation, and is used to revert back to a DERIVED normalized format only (effective constant)


public:

	PPMDigit(int digval, int mod_index, int power);
	~PPMDigit(void);

	int Add(int digval);			// modular digit add
	int Sub(int digval);			// modular digit subtract
	int Mult(int digval);			// modular digit multiply
	int Div(int digval);			// modular digit divide, now supports variable power modulus, choice of table driven or calculation driven
	int Div2(int digval);			// new Div routine for development of more efficient inverse multiplication
	int DivPM(int digval);			// table driven Div method

	int GetDigit(void);			// returns value of digit, handles the partial power digit case
	int GetModulus(int &power);		// returns the value of the base modulus, as well as passes back the total digit power via the power reference argument
	int GetFullPowMod(void);		// returns the full (normal) modulus of the digit, = Modulus^Power
	int GetPowMod2(void);			// returns the present modulus of the digit = Modulus^PowerValid, i.e., valid if skip digit is not true (defines partial digit)
	void CopyDigit(PPMDigit *digit_src);	// copies the source digit to the destination digit

	int GetPowOffset(int power);	// returns the offset value required to round to nearest power
	void SkipDigit(void);			// sets the digit to be skipped
	void ClearSkip(void);			// clears the skip flag
	void ResetPowerValid(void);		// sets (set to power) the powervalid counters for all the digits
	void SetPowerNormal(void);		// sets the powerValid counter to the normal (constant) value of base class
	int TruncMod(unsigned int num_pwrs);			//truncates the PPM digit modulus by the number of powers specified

	int IsZero(int &power);			// returns true if the digit is zero, and passes back specific power that is zero via the reference argument
	int IsZeroPM(int &powref);		// used to test DivPMT, support variable power based modulus using PowerValid
	int IsZero2(int &power);		// for DivPM2 testing
	int IsZeroPM_1pwr(int &powref);

};


class PPM {

//	int BaseClass;				// this variable is only set for the first class that instantiated the static configuration variables, such as primes

public:

//	static int initialized;
	
	int NumDigits;		// number of digits of the PPM object
	int PowerBased;		// mirror copy of the ModTable::PowerBased
	int Mod2_index;		// index of the modulus 2 for print10 functions
	int Mod5_index;		// index of the modulus 5 for print10 functions
	
//	static vector<vector<vector<int>>> arrayDivTbl;
	
	vector<PPMDigit *> Rn;		// the actual Power based RNS number, implemented as array of PPMDigits

	int counter[NUM_PPM_COUNTERS];					// counters are used to count hypothetical clock cycles for study and optimization!
	enum COUNTERS {DIV_COUNT, SUB_COUNT, ADD_COUNT, MULT_COUNT, DEC_COUNT, LOOP_COUNT, BASE_EXTEND, EXTEND_CLK, COMPARE_COUNT, COMPARE_CLK, ADJUST_TYPE, TOTAL_CLK};
	int ena_dplytrc;					// set to one to enable integer divide display trace feature, set to zero to turn off

	// more advanced range checking
	static PPM *Mult_range;		// building strategy to detect some types of overflow, input range check or otherwise


	PPM(__int64 x);
	PPM(PPM *copyval, __int64 x);			// create a PPM and copy all parameters of the PPM argument, make derived if Powervalid != Power
	PPM(void);								// for testing
	~PPM(void);

	friend class ModTable;
	// NEED TO USE THE OVERLOADED OPERATOR METHOD IN C++ FOR THE FOLLOWING FUNCTIONS!!! 
	void Assign(PPM *ppm);		// assigns a PPM to the PPM, but does not alter the "normal" powers of the variable, but does copy the PowerValid counts
	void Assign(int x);			// assigns a integer x to the PPM, mostly a helper function to assign a digit value, resets PPM back to normal
	void Assign(__int64 x);		// assigns an __int64 value to the PPM variable, resets PPM back to normal format
	int Assign(string sval);	// accepts a variable length string representation of the RNS in decimal or hexadecimal number string format
	void AssignRnd(int num_digs);		// assigns a random number to PPM using a random decimal number of num_digs long

	void AssignPM(PPM *ppm);		// assigns value of PPM, and creates new derived modulus format based upon the PowerValid settings of the argument
	void AssignPM(int x, PPM *ppm);	// assigns the power valid structure of ppm, and assigns the value of x, used to derive a new modulus format = x
	void AssignPM(__int64 x);		// NEED !  This should assign __int64 but NOT reset value back to normal PPM format


	void Print(void);			// prints the RNS number using C printfs
	void PrintDemo(void);		// prints the RNS number using C printfs and modulus header
	void PrintDemo(int radix);
	void PrintDemoPM(int radix);			// a print that supports the variable power modulus VPM
	int PrintDigPM(unsigned int digit);		// prints a digit in it's BCFR format, and returns a digit count
	void PrintPM(void);			// prints the raw PPM value in BCFR power based format

	string Prints(void);		// firt pass, comment out wether you want decimal or hex
	string Prints(int radix);	// of just use this one, and pass 10 or 16

	string Print2(void);		// new print design for use with C++ iostream, (gets rid of printf)
	string Print2_NoHdr(void);	// same as above but without b: header
	string Print10(void);		// this method prints decimal conversion to string, bypassing limitations of 64 bit data type
	string Print16(void);		// this method prints hexadecimal conversion to string
	string Print16_NoHdr(void);	// same as above, but without 0x header
	string Print(int radix);	// This is main print string routine, calls methods above, use this for most general purpose
	string Print_NoHdr(int radix);	// This is main print string routine, calls the No_Hdr methods above, use this for formatted printing routines



	__int64 Convert(void);		// converts PPM to __int64 for testing and printing
	unsigned long long uConvert(void);	// converts PPM to unsigned long long integer, otherwise, same as Convert above
	
	int DecimalB(void);			// a simple brute force decimal conversion, for historical reference only!

								// THESE ARE ARITHMETIC METHODS WHICH USE "PAC", OR PARALLEL ARRAY COMPUTATION.  OF COURSE, THE SOFTWARE LIB DOES NOT DO THEM IN PARALLEL
	void Add(PPM *ppm);			// basic arithmetic operations, future version might consider dual argument arithmetic methods, and possibly operator overloading
	void Add(int x);			// (integer argument versions are helper functions, and are used mostly to apply arithmetic using a digit value argument)	
	void Add(__int64 x);
	void Sub(PPM *ppm);
	void Sub(int x);
	void Sub(__int64 x);
	void Mult(PPM *ppm);
	void Mult(int x);
	void Mult(__int64 x);
	void ModDiv(PPM *ppm);		// Modulo divide (division by a single RNS Modulus)
	void ModDiv(int x);
	void ModDiv2(int x);

	void Complement(void);		// calculates the arithmetic complement
	void Increment(void);		// increments the RNS number
	void Decrement(void);		// decrements the RNS number
										// DIVISION PROTOTYPES, USE DivStd() in application code, which will call any of the following:
	int DivStd(PPM *divisor, PPM *rem);	// CONTAINER FUNCTION WHICH CALLS THE MOST STABLE VERSION FOR THE APPLICATION CODE, CURRENTLY DIVPM5
	void DivPM3(PPM *divisor, PPM *rem); // research divide routine for the new variable modulus capability of the upgraded PPM, based on DivPM() (ebo, 3/1/2014)
	void DivPM4(PPM *divisor, PPM *rem);  // created to make debug session started on 4/10/2016
	int DivPM5(PPM *divisor, PPM *rem);  // new method increments divisor, and supports full PPM range without redundant modulus, PROTOTYPED 4/10/2016
	int DivPM5T(PPM *divisor, PPM *rem);  // debug version startd on 4/10/2016, new method increments divisor, and supports full range without redundant modulus
	int DivPM6(PPM *divisor, PPM *rem);  // derived from DivPM5, TEST: removes compare on iteration
	int DivPM7(PPM *divisor, PPM *rem);  // derived from DivPM6, TEST: removes more base extends
	void DivPM_Rez9A(PPM *ppm, PPM *rm);	// simulation for first Rez9 integer divider
	int DivCheck(PPM *dividend, PPM *divisor, PPM *quotient, PPM *rem);	// checks to see if the divide result is correct, if so, return true
	void GoldDiv(PPM *divisor, PPM *rem);	// NOT DEVELOPED, new idea to convert integers directly to smallest fraction allowable, then use GoldSchmidt division, and correct
	void Clear_Counters(void);	// clear the clock performance counters 
	int PrintCounters(void);	// formatted print of RNS clock counters, returns total clocks	

	//	Some prototype library routines
	void Sqrt(void);		// return the unsigned integer square root
	void Factorial(int x);
	
	void Cfr(PPM *d, cfr_data *s);		// routine for testing CFR reduction
	int AnyZero(void);					// routine to check if any zero exists in the PPM number, was created for CFR test procedure

	// basic value testing routines
	int Zero(void);				// returns true if the Rn is zero	(currently handles skip flags)
	int Zero(int index);		// returns true if all non skipped digits starting at index are zero
	int One(void);				// returns true if the Rn is one	(always handles skip flags?)

	// comparison routines
	int Compare(PPM *ppm);					// compare the RNS number against another RNS number  (all digits)	
	int Compare(PPM *ppm, int &clocks);		// compare the RNS number against another RNS number, returns clocks
	int ComparePart(PPM *ppm, int numdigs);  // compare the first numdigs worth of digits, used for rounding detection
	int CompareDif(PPM *ppm, int &clocks);	// compare the difference, i.e., is (ppm1-ppm2)>(ppm2-ppm1)?, experimental method for divide, etc.		
	int IsEqual(PPM *ppm);					// perform equality check against the PPM
	int IsEqualPart(PPM *ppm, int numdigs);	// determine if the first numdigs number of digits is equal to the same digits of ppm arg

	// base extension routines
	int Extend(void);			// this is a "do all" method, it simply calls ExtendPart2Norm().
	int ExtendNorm(void);		// base extend a Normalized PPM value only, all digits flagged as skipped are extended, returns clocks required
	int ExtendPart2Norm(void);	// base extend with partial power digit support, converting to normal format when complete
	int Normalize(void);		// base extend back to the standard normalized format from derived base number type.
	void TruncateFirst(PPM *ppm, int numdigs);	// routine to extend first n digits.  Used to perform ExtendRange
	
	// format testing routines
	int chk_PM_format(PPM *ppm);	// checks the (variable) PM format of this versus the argument ppm, if modulus are not same, return 0, else return 1

	// range generation and range checking routines
	// being able to check and assess range of unsigned integers
	int CompareRange(PPM *ppm);			// PROPOSED - compare the RNS number against another RNS number  (Range digits)
	int CompareMultRange(PPM *ppm);		// PROPOSED - compare the RNS number to square root of Range (Multiplicative Range) 
	int CompareConstant(MRN *mrn);		// PROPOSED - simulates a compare by a constant, that is, the constant is pre-converted to MRN

	int GetRange(PPM *ppm, int num_digits);		// Returns full range of the first numdigs of PPM variable
	void GetMultRange(PPM *ppm);		// stores the largest format of integer type if multply is not to overflow the system.
	void GetFullRange(PPM *ppm);	// returns the largest absolute PPM integer in the RNS system

	void Backup(void);			// save and restore helper functions, to be deprecated.  USE ASSIGN() METHOD INSTEAD
	void Restore(void);

private:
										// The following routines are helper routines for the integer divide routine
	int Divisible_by_n_pmd(int index);		// return true if the PPM is divisible by any of the first n PPM digit modulus
	int Divisible_by_n_pmd_PM(int index);	// return True if the PPM is divisible, including partial divides of Power based digits
	int Divisible_by_n_pmd_PM2(int index);	// for DivPM2 development
	int Dec_by_next_fact(PPM *ppm, int &dig_val, int &start_index, int end_index);
	int Dec_by_next_fact_PM(PPM *ppm, int &dig_val, int &start_index, int &pwr, int end_index);
	int Dec_by_next_fact_PM_1pwr(PPM *ppm, int &dig_val, int &start_index, int &pwr, int end_index);
	
	void Clear_Skips(void);			// clears all digit skip flags
	void Reset_PowerSkips(void);	// resets all digit powervalid flags to modulus power (PowerValid = Power)
	void Reset_Normal(void);		// resets each digit to it's normal  modulus power  (PowerValid = NormalPower)
	
	int AnySkips(void);			// returns true if any digits are flagged as skipped
	int AnyPartSkips(void);		// returns TRUE if any digits are flagged as partial or fully skipped

	int IsNormal(void);			// check if the PPM argument is normalized (are all digit modulus normal?, i.e. PowerValid = Power)



};


#endif