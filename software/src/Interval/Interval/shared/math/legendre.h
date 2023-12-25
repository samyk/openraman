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

 // disable stupid warnings
#pragma warning(disable:26451)

#include <math.h>

#include "binomial.h"
#include "power.h"

// legendre polynomial
static double legendre(unsigned int n, double x)
{
    // precomputed cases for faster access
    switch (n)
    {
    case 0:
        return 1;

    case 1:
        return x;

    case 2:
        return 0.5 * (3.0 * x * x - 1.0);

    case 3:
        return 0.5 * (5.0 * x * x * x - 3.0 * x);
    }

    // generic case
    double y = 0;

    for (unsigned int k = 0; k <= n; k++)
    {
        auto b = binomial(n, k);

        y += b * b * quickpow(x - 1.0, n - k) * quickpow(x + 1.0, k);
    }

    return y / (double)(1 << n);
}