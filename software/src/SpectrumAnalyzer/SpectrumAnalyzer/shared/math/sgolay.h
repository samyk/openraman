/*
 *	2020 (C) The Pulsar Engineering
 *	http://www.thepulsar.be
 *
 *	This document is licensed under the CERN OHL-W v2 (http://ohwr.org/cernohl).
 *
 *	You may redistribute and modify this document under the terms of the
 *	CERN OHL-W v2 only. This document is distributed WITHOUT ANY EXPRESS OR
 *	IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND
 *	FITNESS FOR APARTICULAR PURPOSE. Please refer to the CERN OHL-W v2 for
 *	applicable conditions.
 */
#pragma once

#include <string>

#include "../utils/exception.h"

#include "vector.h"
#include "matrix.h"
#include "power.h"

// InvalidSGolayParameterException exception class
class InvalidSGolayParameterException : public IException
{
public:
    InvalidSGolayParameterException(size_t nWindowSize, size_t nOrder, size_t nDerivative)
    {
        this->m_nWindowSize = nWindowSize;
        this->m_nOrder = nOrder;
        this->m_nDerivative = nDerivative;
    }

    virtual std::string toString(void) const override
    {
        char szTmp[256];

        sprintf_s(szTmp, "Cannot apply Savitzky-Golay to window=%zu, order=%zu, d/dx=%zu!", this->m_nWindowSize, this->m_nOrder, this->m_nDerivative);

        return std::string(szTmp);
    }

private:
    size_t m_nWindowSize, m_nOrder, m_nDerivative;
};

// Savitzky-Golay filter
static vector_t sgolay(const vector_t& rInput, size_t nWindowSize, size_t nOrder, size_t nDerivative=0)
{
    // always include 0th order
    nOrder++;

    // check inputs
    if (nWindowSize < nOrder || nDerivative >= nOrder)
        throwException(InvalidSGolayParameterException, nWindowSize, nOrder, nDerivative);

    // special case for boxcar
    if (nOrder == 1)
        return boxcar(rInput, nWindowSize);

    // compute z vector
    auto zvec = linspace((1.0 - (double)nWindowSize) * 0.5, ((double)nWindowSize - 1.0) * 0.5, nWindowSize);

    // compute J matrix
    Matrix Jmat(nOrder, nWindowSize);

    for (size_t j = 0; j < nWindowSize; j++)
        for (size_t i = 0; i < nOrder; i++)
            Jmat(i, j) = quickpow(zvec[j], (unsigned int)i);

    // compute J matrix transposate
    auto JmatT = Jmat.transpose();

    // compute coefficient matrix according to formula
    auto Cmat = inv(JmatT * Jmat) * JmatT;

    // extract coefficients
    auto coeffs = Cmat.extractColumn(nDerivative);

    // return convolution
    return conv(rInput, coeffs);
}