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

#include "../utils/exception.h"

// InvalidBinomialCoefficients exception class
class InvalidBinomialCoefficients : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Invalid Coefficients for Binomial!";
	}
};

// compute binomial (k,n)
static unsigned int binomial(unsigned int n, unsigned int k)
{
	// check parameters
	if (k > n)
		throwException(InvalidBinomialCoefficients);

	// simple cases
	if (n == 0 || k == 0)
		return 1;

	// simplify if possible
	if (k > (n >> 1))
		return binomial(n, n - k);

	// recursive implementation
	return (n * binomial(n - 1, k - 1)) / k;
}