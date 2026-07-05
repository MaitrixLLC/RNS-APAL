#if defined(__linux__) && !defined(_WIN32)
#include <inttypes.h>
typedef int64_t __int64;
#else

#endif

#include "stdafx.h"
#include "stdio.h"
#include "math.h"
#include "utilities.h"

#include <iostream>
#include <cstring>

#include "ppm.h"
#include "sppm.h"
#include "spmf.h"
#include "RNSMath.h"
#include "RNSMathMenu.h"

using std::cout;
using std::endl;


// ------------------------------------------------------------
// Local screen clear helper.
// Kept local so we do not disturb the existing main menu file.
// ------------------------------------------------------------
static void rns_math_cls(void)
{
    for (int i = 0; i < 25; i++)
        printf("\r\n");
}


static void rns_math_pause(void)
{
    printf("\nPress any key to continue...\n");
    wait_key();
    wait_key();
}

// This routine was added since there seems to be trouble in the input routine of the SPMF value
static int assign_spmf_signed_input(SPMF* x, const char* input)
{
    if (input == NULL)
        return 0;

    if (input[0] == '-')
    {
        //
        // Workaround:
        // Some SPMF::AssignFP() paths appear to mishandle direct
        // negative fractional strings such as "-0.5".
        //
        // Therefore:
        //   1. assign the positive magnitude string
        //   2. explicitly negate using inherited SPPM::Negate()
        //
        if (!x->AssignFP((char*)&input[1]))
            return 0;

        x->Negate();
        return 1;
    }

    if (input[0] == '+')
    {
        return x->AssignFP((char*)&input[1]);
    }

    return x->AssignFP((char*)input);
}

static void print_spmf_signed_pm10(SPMF* x)
{
    SPMF temp(0);

    //
    // Preserve signed value and sign metadata.
    //
    temp.Assign(x);

    //
    // Ensure sign metadata is valid before display.
    //
    temp.CalcSetSign();

    if (temp.GetSignFlag() == NEGATIVE)
    {
        temp.Abs();

        cout << "-";
        cout << temp.PrintAbsPM(10) << endl;
    }
    else
    {
        cout << temp.PrintAbsPM(10) << endl;
    }
}

// ------------------------------------------------------------
// Bounds / method-description helpers
// ------------------------------------------------------------

static void print_exp_taylor_bounds(void)
{
    printf("\n");
    printf("Algorithm note for direct Exp_Taylor(x):\n");
    printf("  exp(x) Taylor series converges for all finite x.\n");
    printf("  However, this direct prototype is intended for small |x|.\n");
    printf("  Recommended early-test range: -1.0 <= x <= +1.0\n");
    printf("  Larger positive x may create large intermediate terms.\n");
    printf("  Larger negative x may cause cancellation.\n");
    printf("  Future versions should use argument reduction.\n");
    printf("\n");
}


static void print_e_taylor_bounds(void)
{
    printf("\n");
    printf("Algorithm note for E_Taylor():\n");
    printf("  This computes e as exp(1) using the Taylor series.\n");
    printf("  The direct series is algorithmically valid, and x = 1 is\n");
    printf("  at the upper edge of the recommended prototype exp(x) range.\n");
    printf("  Accuracy depends on term count and the active SPMF fractional basis.\n");
    printf("\n");
}


static void print_pi_machin_bounds(void)
{
    printf("\n");
    printf("Algorithm note for Pi_Machin():\n");
    printf("  Uses Machin's formula:\n");
    printf("      pi = 16 atan(1/5) - 4 atan(1/239)\n");
    printf("  This is well suited to the atan Taylor series because both\n");
    printf("  arguments are small and safely inside the convergence range.\n");
    printf("  atan(x) Taylor convergence requirement: |x| <= 1\n");
    printf("  Practical recommendation: |x| << 1 for fast convergence.\n");
    printf("\n");
}


// ------------------------------------------------------------
// RNSMath submenu
// ------------------------------------------------------------
static void print_rns_math_menu(void)
{
    printf("RNSMath Test Menu\n\n");

    printf("1. Compute e using RNSMath::E_Taylor\n");
    printf("2. Compute e using legacy SPMF::AssignTaylor_E\n");
    printf("3. Compute exp(x) using RNSMath::Exp_Taylor\n");
    printf("4. Compute pi using RNSMath::Pi_Machin\n");
    printf("5. Compute sin(x) using RNSMath::Sin_Taylor\n");
    printf("6. Compute cos(x) using RNSMath::Cos_Taylor\n");

    printf("\n");
    printf("q. Return to main menu\n");
}


// ------------------------------------------------------------
// Test 1: e using new RNSMath layer
// ------------------------------------------------------------
static void rns_math_test_e_taylor(void)
{
    int terms = 32;

    printf("\n");
    printf("========================================\n");
    printf(" RNSMath Taylor-Series Test: e\n");
    printf("========================================\n\n");

    print_e_taylor_bounds();

    printf("Enter number of Taylor terms, e.g. 16, 32, 64: ");
    scanf("%d", &terms);

    RNSMath::Options opts;
    opts.max_terms = terms;

    SPMF* e_val = new SPMF(0);

    RNSMath::E_Taylor(*e_val, opts);

    printf("\n");
    printf("e computed using RNSMath::E_Taylor:\n");
    cout << e_val->PrintPM(10) << endl;

    printf("\n");
    printf("Double display estimate only:\n");
    printf("%.20f\n", e_val->PrintFPM());

    printf("\n");
    printf("Reference double e:\n");
    printf("%.20f\n", exp(1.0));

    printf("\n");
    printf("Raw RNS representation:\n");
    e_val->PrintDemo();

    printf("\n========================================\n\n");

    rns_math_pause();

    delete e_val;

    wait_key();
}


// ------------------------------------------------------------
// Test 2: e using the older SPMF member function
// ------------------------------------------------------------
static void rns_math_test_legacy_taylor_e(void)
{
    SPMF* e_val = new SPMF(0);

    printf("\n");
    printf("========================================\n");
    printf(" Legacy SPMF Taylor-Series Test: e\n");
    printf("========================================\n\n");

    print_e_taylor_bounds();

    e_val->AssignTaylor_E();

    printf("\n");
    printf("e computed using SPMF::AssignTaylor_E:\n");
    cout << e_val->PrintPM(10) << endl;

    printf("\n");
    printf("Double display estimate only:\n");
    printf("%.20f\n", e_val->PrintFPM());

    printf("\n");
    printf("Reference double e:\n");
    printf("%.20f\n", exp(1.0));

    printf("\n");
    printf("Raw RNS representation:\n");
    e_val->PrintDemo();

    printf("\n========================================\n\n");

    rns_math_pause();

    delete e_val;

    wait_key();
}


// ------------------------------------------------------------
// Test 3: exp(x) using new RNSMath layer
// ------------------------------------------------------------
static void rns_math_test_exp_taylor(void)
{
    char input[100];
    int terms = 32;

    printf("\n");
    printf("========================================\n");
    printf(" RNSMath Taylor-Series Test: exp(x)\n");
    printf("========================================\n\n");

    print_exp_taylor_bounds();

    printf("Enter x, e.g. 1.0, 0.5, -0.25, or q to quit: ");
    scanf("%s", input);

    while (strcmp(input, "q"))
    {
        printf("\n");
        printf("Term-count note:\n");
        printf("  More terms generally improve the approximation until the\n");
        printf("  term size falls below the active SPMF fractional resolution.\n");
        printf("  For |x| <= 1, 16 to 64 terms is a reasonable starting range.\n");
        printf("\n");

        printf("Enter number of Taylor terms, e.g. 16, 32, 64: ");
        scanf("%d", &terms);

        RNSMath::Options opts;
        opts.max_terms = terms;

        SPMF* x = new SPMF(0);
        SPMF* y = new SPMF(0);

        if (!x->AssignFP(input))
        {
            printf("ERROR: invalid SPMF fractional string format.\n");
            delete x;
            delete y;

            print_exp_taylor_bounds();

            printf("\nEnter x, e.g. 1.0, 0.5, -0.25, or q to quit: ");
            scanf("%s", input);
            continue;
        }

        printf("\n");
        printf("Input x:\n");
        cout << x->PrintPM(10) << endl;

        printf("\nRaw input x:\n");
        x->PrintDemo();

        RNSMath::Exp_Taylor(*x, *y, opts);

        printf("\n");
        printf("exp(x) computed using RNSMath::Exp_Taylor:\n");
        cout << y->PrintPM(10) << endl;

        printf("\n");
        printf("Double display estimate only:\n");
        printf("%.20f\n", y->PrintFPM());

        printf("\n");
        printf("Reference double exp(x):\n");
//        printf("%1.18f\n", exp(x->PrintFPM()));
        double x_double = atof(input);
        printf("%.20f\n", exp(x_double));

        printf("\n");
        printf("Raw exp(x) RNS representation:\n");
        y->PrintDemo();

        rns_math_pause();

        delete x;
        delete y;

        printf("\nTry another exp(x)?\n");
        print_exp_taylor_bounds();

        printf("Enter x or q to quit: ");
        scanf("%s", input);
    }

    printf("\nRNSMath exp(x) Taylor test terminated\n");

    rns_math_pause();

    wait_key();
}


// ------------------------------------------------------------
// Test 4: pi using Machin's formula
// ------------------------------------------------------------
static void rns_math_test_pi_machin(void)
{
    int terms = 32;

    printf("\n");
    printf("========================================\n");
    printf(" RNSMath Machin Formula Test: pi\n");
    printf("========================================\n\n");

    print_pi_machin_bounds();

    printf("Enter number of atan Taylor terms, e.g. 8, 16, 32, 64: ");
    scanf("%d", &terms);

    RNSMath::Options opts;
    opts.max_terms = terms;

    SPMF* pi_val = new SPMF(0);

    RNSMath::Pi_Machin(*pi_val, opts);

    printf("\n");
    printf("pi computed using RNSMath::Pi_Machin:\n");
    cout << pi_val->PrintAbsPM(10) << endl;

    printf("\n");
    printf("Double display estimate only:\n");
    printf("%.20f\n", pi_val->PrintFPM());

    printf("\n");
    printf("Reference double pi:\n");
    printf("%.20f\n", 4.0 * atan(1.0));

    printf("\n");
    printf("Raw pi RNS representation:\n");
    pi_val->PrintDemo();

    printf("\n========================================\n\n");

    rns_math_pause();

    delete pi_val;

    wait_key();
}

static void rns_math_test_cos_taylor(void)
{
    char input[100];
    int terms = 32;

    printf("\n");
    printf("========================================\n");
    printf(" RNSMath Taylor-Series Test: cos(x)\n");
    printf("========================================\n\n");

    printf("Algorithm note for direct Cos_Taylor(x):\n");
    printf("  Direct Taylor cos(x) is best for small |x|.\n");
    printf("  Recommended early-test range: -1.0 <= x <= +1.0 radians\n");
    printf("  Larger angles should eventually use argument reduction.\n\n");

    printf("Enter x in radians, e.g. 1.0, 0.5, -0.25, or q to quit: ");
    scanf("%s", input);

    while (strcmp(input, "q"))
    {
        printf("Enter number of Taylor terms, e.g. 16, 32, 64: ");
        scanf("%d", &terms);

        RNSMath::Options opts;
        opts.max_terms = terms;

        SPMF* x = new SPMF(0);
        SPMF* y = new SPMF(0);

        if (!x->AssignFP(input))
        {
            printf("ERROR: invalid SPMF fractional string format.\n");
            delete x;
            delete y;

            printf("\nEnter x in radians or q to quit: ");
            scanf("%s", input);
            continue;
        }

        printf("\nInput x:\n");
        cout << x->PrintPM(10) << endl;

        RNSMath::Cos_Taylor(*x, *y, opts);

        printf("\n");
        printf("cos(x) computed using RNSMath::Cos_Taylor:\n");
        cout << y->PrintPM(10) << endl;

        printf("\nDouble display estimate only:\n");
        printf("%.20f\n", y->PrintFPM());

        printf("\nReference double cos(x):\n");
//        printf("%.20f\n", cos(x->PrintFPM()));
        double x_double = atof(input);
        printf("%.20f\n", cos(x_double));

        printf("\nRaw cos(x) RNS representation:\n");
        y->PrintDemo();

        rns_math_pause();

        delete x;
        delete y;

        printf("\nTry another cos(x)?\n");
        printf("Enter x or q to quit: ");
        scanf("%s", input);
    }

    printf("\nRNSMath cos(x) Taylor test terminated\n");
    rns_math_pause();
}

static void rns_math_test_sin_taylor(void)
{
    char input[100];
    int terms = 32;

    printf("\n");
    printf("========================================\n");
    printf(" RNSMath Taylor-Series Test: sin(x)\n");
    printf("========================================\n\n");

    printf("Algorithm note for direct Sin_Taylor(x):\n");
    printf("  Direct Taylor sin(x) is best for small |x|.\n");
    printf("  Recommended early-test range: -1.0 <= x <= +1.0 radians\n");
    printf("  Larger angles should eventually use argument reduction.\n\n");

    printf("Enter x in radians, e.g. 1.0, 0.5, -0.25, or q to quit: ");
    scanf("%s", input);

    while (strcmp(input, "q"))
    {
        printf("Enter number of Taylor terms, e.g. 16, 32, 64: ");
        scanf("%d", &terms);

        RNSMath::Options opts;
        opts.max_terms = terms;

        SPMF* x = new SPMF(0);
        SPMF* y = new SPMF(0);

//        if (!x->AssignFP(input))          // this routine has an issue with -0.5
        if (!assign_spmf_signed_input(x, input))
        {
            printf("ERROR: invalid SPMF fractional string format.\n");
            delete x;
            delete y;

            printf("\nEnter x in radians or q to quit: ");
            scanf("%s", input);
            continue;
        }

        printf("\nInput x:\n");
        cout << x->PrintPM(10) << endl;
        cout << "using prints: " << x->Prints() << endl;

        RNSMath::Sin_Taylor(*x, *y, opts);

        printf("\n");
        printf("sin(x) computed using RNSMath::Sin_Taylor:\n");
        cout << y->PrintPM(10) << endl;

        printf("\nDouble display estimate only:\n");
        printf("%.20f\n", y->PrintFPM());

 //       printf("\nReference double sin(x):\n");
 //       printf("%.20f\n", sin(x->PrintFPM()));

        double x_double = atof(input);
        printf("\nDouble value (from input string): %.20f\n", sin(x_double));

        printf("\nRaw sin(x) RNS representation:\n");
        y->PrintDemo();

        rns_math_pause();

        delete x;
        delete y;

        printf("\nTry another sin(x)?\n");
        printf("Enter x or q to quit: ");
        scanf("%s", input);
    }

    printf("\nRNSMath sin(x) Taylor test terminated\n");
    rns_math_pause();
}

// ------------------------------------------------------------
// Public RNSMath submenu launcher
// ------------------------------------------------------------
void rns_math_tests(void)
{
    char c[100];
    char* s;

    c[0] = 0;
    s = &c[0];

    while (strcmp(s, "q"))
    {
        rns_math_cls();

        print_rns_math_menu();

        printf("\nEnter RNSMath choice: ");
        scanf("%s", &c[0]);

        switch (c[0])
        {
        case '1':
            rns_math_cls();
            rns_math_test_e_taylor();
            break;

        case '2':
            rns_math_cls();
            rns_math_test_legacy_taylor_e();
            break;

        case '3':
            rns_math_cls();
            rns_math_test_exp_taylor();
            break;

        case '4':
            rns_math_cls();
            rns_math_test_pi_machin();
            break;

        case '5':
            rns_math_cls();
            rns_math_test_sin_taylor();
            break;

        case '6':
            rns_math_cls();
            rns_math_test_cos_taylor();
            break;

        case 'q':
            break;

        default:
            printf("Invalid RNSMath menu key, try again\n");
            wait_key();
            break;
        }
    }
}