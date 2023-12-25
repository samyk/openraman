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

#include "vector.h"

// InvalidInterpolationDataException exception class
class InvalidInterpolationDataException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "invalid interpolation data!";
    }
};

// linear interpolation
static vector_t linterp(const vector_t& indices, const vector_t& vec)
{
	vector_t ret(indices.size());

    for (size_t i = 0; i < indices.size(); i++)
    {
        double lo = floor(indices[i]);
        double hi = ceil(indices[i]);

        if (lo < 0 || hi >= vec.size())
            throw InvalidInterpolationDataException();

        double p = indices[i] - lo;

        ret[i] = p * vec[(size_t)hi] + (1.0 - p) * vec[(size_t)lo];
    }

    return ret;
}