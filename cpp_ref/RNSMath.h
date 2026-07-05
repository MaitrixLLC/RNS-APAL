#pragma once
#ifndef RNSMATH_H
#define RNSMATH_H

#include "spmf.h"

namespace RNSMath
{
    struct Options
    {
        int max_terms;

        Options()
        {
            max_terms = 32;
        }
    };

    void Atan_Taylor(const SPMF& x, SPMF& result, const Options& opts);
    void Pi_Machin(SPMF& result, const Options& opts);

    void Exp_Taylor(const SPMF& x, SPMF& result, const Options& opts);
    void E_Taylor(SPMF& result, const Options& opts);

    void Atan_Taylor(const SPMF& x, SPMF& result);
    void Pi_Machin(SPMF& result);

    void Exp_Taylor(const SPMF& x, SPMF& result);
    void E_Taylor(SPMF& result);

    void Sin_Taylor(const SPMF& x, SPMF& result, const Options& opts);
    void Cos_Taylor(const SPMF& x, SPMF& result, const Options& opts);

    void Sin_Taylor(const SPMF& x, SPMF& result);
    void Cos_Taylor(const SPMF& x, SPMF& result);
}

#endif