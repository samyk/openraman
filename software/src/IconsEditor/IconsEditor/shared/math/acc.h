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

#include "../utils/safe.h"

#include "vector.h"

// Accumulator class
class Accumulator
{
public:
    Accumulator(void)
    {
        reset();
    }

    // reset accumulator
    void reset(void)
    {
        this->m_nNumData = 0;

        this->m_sum.clear();
        this->m_sumsq.clear();
    }

    // add vector
    void add(const vector_t& vec)
    {
        this->m_sum += vec;
        this->m_sumsq += pow(vec, 2);

        __INC(this->m_nNumData, (size_t)1);
    }

    // return true if accumulator has data
    bool valid(void) const
    {
        return this->m_nNumData > 0;
    }

    // return number of data
    size_t num(void) const
    {
        return this->m_nNumData;
    }

    // get average
    vector_t mean(void) const
    {
        return this->m_sum / (double)this->m_nNumData;
    }

    // get stdev
    vector_t stdev(void) const
    {
        return sqrt((this->m_sumsq / (double)this->m_nNumData) - pow(mean(), 2));
    }
    
private:
    vector_t m_sum, m_sumsq;

    size_t m_nNumData;
};