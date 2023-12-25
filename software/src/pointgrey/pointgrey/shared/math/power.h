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

 // quick power function
static double quickpow(double v, unsigned int n)
{
    // v^0 = 1
    if (n == 0)
        return 1;

    // v^1 = v
    if (n == 1)
        return v;

    // v^(2n) = (v^n)²
    if ((n % 2) == 0)
    {
        auto temp = quickpow(v, n >> 1);

        return temp * temp;
    }
    // v^(2n+1) = v^(2n) * v
    else
        return quickpow(v, n - 1) * v;
}