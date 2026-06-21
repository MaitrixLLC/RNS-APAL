
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

#include "sppm.h"
#include "spmf.h"
#include "mrn.h"
#include "RNSMath.h"

namespace RNSMath
{
    // ------------------------------------------------------------
    // Atan_Taylor
    // ------------------------------------------------------------
    //
    // atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
    //
    // Recurrence:
    //
    // term_0 = x
    // term_n = term_{n-1} * x^2
    //
    // contribution_n = term_n / (2n + 1)
    //
    // result = contribution_0 - contribution_1
    //        + contribution_2 - contribution_3 + ...
    //
    // This is best for |x| <= 1 and especially good for small
    // values such as 1/5 and 1/239.
    // ------------------------------------------------------------

    void Atan_Taylor(const SPMF& x, SPMF& result, const Options& opts)
    {
        result.Assign((__int64)0);

        SPMF x2(0);
        SPMF term(0);
        SPMF contribution(0);
        SPMF divisor(0);

        //
        // x2 = x * x
        //
        x2.Assign((SPMF*)&x);
        x2.MultStd((SPMF*)&x);

        //
        // term = x
        //
        term.Assign((SPMF*)&x);

        for (int n = 0; n < opts.max_terms; n++)
        {
            int d = 2 * n + 1;

            //
            // contribution = term / d
            //
            contribution.Assign(&term);

            divisor.AssignUnitPM();
            divisor.Mult(d);

            contribution.DivPM(&divisor);

            //
            // Alternating sum:
            //
            // + x
            // - x^3/3
            // + x^5/5
            // - x^7/7
            //
            if ((n & 1) == 0)
            {
                result.Add(&contribution);
            }
            else
            {
                result.Sub(&contribution);
            }

            //
            // term = term * x^2
            //
            term.MultStd(&x2);      // this routine regenerates sign flags
        }
        //
        // result was accumulated using Add/Sub.  Add/Sub may leave the
        // cached sign flag invalid, so regenerate the final result sign
        // before returning.
        //
        result.CalcSetSign();
    }


    void Atan_Taylor(const SPMF& x, SPMF& result)
    {
        Options opts;
        Atan_Taylor(x, result, opts);
    }


    // ------------------------------------------------------------
    // Exp_Taylor
    // ------------------------------------------------------------
    //
    // exp(x) = 1 + x + x^2/2! + x^3/3! + ...
    //
    // Recurrence:
    //
    // term_0 = 1
    // term_n = term_{n-1} * x / n
    //
    // result = sum(term_n)
    //
    // This version uses:
    //
    //   AssignUnitPM()  for 1.0
    //   MultStd()       for preferred/stable fractional multiply
    //   DivPM()         for fractional division
    //   Add()           for fractional accumulation
    //
    // ------------------------------------------------------------

    void Exp_Taylor(const SPMF& x, SPMF& result, const Options& opts)
    {
        result.AssignUnitPM();

        SPMF term(0);
        SPMF divisor(0);

        term.AssignUnitPM();

        for (int n = 1; n <= opts.max_terms; n++)
        {
            //
            // term = term * x
            //
            term.MultStd((SPMF*)&x);

            //
            // term = term / n
            //
            divisor.AssignUnitPM();
            divisor.Mult(n);

            term.DivPM(&divisor);

            //
            // result += term
            //
            result.Add(&term);
        }

        result.CalcSetSign();
    }


    void Exp_Taylor(const SPMF& x, SPMF& result)
    {
        Options opts;
        Exp_Taylor(x, result, opts);
    }


    // ------------------------------------------------------------
    // E_Taylor
    // ------------------------------------------------------------
    //
    // e = exp(1)
    //
    // ------------------------------------------------------------

    void E_Taylor(SPMF& result, const Options& opts)
    {
        SPMF one(0);
        one.AssignUnitPM();

        Exp_Taylor(one, result, opts);
    }


    void E_Taylor(SPMF& result)
    {
        Options opts;
        E_Taylor(result, opts);
    }


    // ------------------------------------------------------------
    // Pi_Machin
    // ------------------------------------------------------------
    //
    // Machin formula:
    //
    // pi = 16 * atan(1/5) - 4 * atan(1/239)
    //
    // This is a good first pi test because both atan arguments
    // are small, causing the Taylor series to converge rapidly.
    //
    // ------------------------------------------------------------

    void Pi_Machin(SPMF& result, const Options& opts)
    {
        SPMF x1(0);
        SPMF x2(0);

        SPMF atan1(0);
        SPMF atan2(0);

        //
        // x1 = 1/5
        // x2 = 1/239
        //
        // AssignRatio() is preferred here because these are exact
        // rational constants.
        //
        x1.AssignRatio(1, 5);
        x2.AssignRatio(1, 239);

        Atan_Taylor(x1, atan1, opts);
        Atan_Taylor(x2, atan2, opts);

        //
        // atan1 = 16 * atan(1/5)
        // atan2 =  4 * atan(1/239)
        //
        atan1.Mult((__int64)16);
        atan2.Mult((__int64)4);

        //
        // result = atan1 - atan2
        //
        result.Assign(&atan1);
        result.Sub(&atan2);

        result.CalcSetSign();
    }


    void Pi_Machin(SPMF& result)
    {
        Options opts;
        Pi_Machin(result, opts);
    }

    void Sin_Taylor(const SPMF& x, SPMF& result, const Options& opts)
    {
        //
        // sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
        //
        // Recurrence:
        //
        // term_0 = x
        //
        // term_{n+1} =
        //     -term_n * x^2 / ((2n+2)(2n+3))
        //
        // result = sum(term_n)
        //
        // Algorithmic prototype range:
        //
        //     |x| <= 1.0 radians recommended
        //
        // Larger angles should eventually use argument reduction.
        //

        result.Assign((__int64)0);

        SPMF x2(0);
        SPMF term(0);
        SPMF divisor(0);

        //
        // x2 = x * x
        //
        x2.Assign((SPMF*)&x);
        x2.MultStd((SPMF*)&x);

        //
        // term = x
        //
        term.Assign((SPMF*)&x);

        for (int n = 0; n < opts.max_terms; n++)
        {
            //
            // result += term
            //
            result.Add(&term);

            //
            // term = term * x^2
            //
            term.MultStd(&x2);

            //
            // term = term / ((2n+2)(2n+3))
            //
            int d = (2 * n + 2) * (2 * n + 3);

            divisor.AssignUnitPM();
            divisor.Mult(d);

            term.DivPM(&divisor);

            //
            // term = -term
            //
            term.Negate();
        }

        result.CalcSetSign();       // accumulation may invalidate the sign valid flag
    }


    void Sin_Taylor(const SPMF& x, SPMF& result)
    {
        Options opts;
        Sin_Taylor(x, result, opts);
    }

    void Cos_Taylor(const SPMF& x, SPMF& result, const Options& opts)
    {
        //
        // cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + ...
        //
        // Recurrence:
        //
        // term_0 = 1
        //
        // term_{n+1} =
        //     -term_n * x^2 / ((2n+1)(2n+2))
        //
        // result = sum(term_n)
        //
        // Algorithmic prototype range:
        //
        //     |x| <= 1.0 radians recommended
        //
        // Larger angles should eventually use argument reduction.
        //

        result.Assign((__int64)0);

        SPMF x2(0);
        SPMF term(0);
        SPMF divisor(0);

        //
        // x2 = x * x
        //
        x2.Assign((SPMF*)&x);
        x2.MultStd((SPMF*)&x);

        //
        // term = 1
        //
        term.AssignUnitPM();

        for (int n = 0; n < opts.max_terms; n++)
        {
            //
            // result += term
            //
            result.Add(&term);

            //
            // term = term * x^2
            //
            term.MultStd(&x2);

            //
            // term = term / ((2n+1)(2n+2))
            //
            int d = (2 * n + 1) * (2 * n + 2);

            divisor.AssignUnitPM();
            divisor.Mult(d);

            term.DivPM(&divisor);

            //
            // term = -term
            //
            term.Negate();
        }

        result.CalcSetSign();
    }


    void Cos_Taylor(const SPMF& x, SPMF& result)
    {
        Options opts;
        Cos_Taylor(x, result, opts);
    }

}