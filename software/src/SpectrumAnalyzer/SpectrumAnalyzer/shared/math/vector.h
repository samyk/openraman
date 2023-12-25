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

#include <math.h>

#include <vector>

#include "../utils/utils.h"
#include "../utils/exception.h"

// InvalidSizeException class
class InvalidSizeException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "";
	}
};

// vector_t type is std::vector<double>
using vector_t = std::vector<double>;

// return vector full of zeros
static auto zeros(size_t nSize)
{
	vector_t ret(nSize);

	for (size_t i = 0; i < nSize; i++)
		ret[i] = 0;

	return ret;
}

// multiply vector by constant
static auto operator*(const vector_t &vec, double fScale)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = vec[i] * fScale;

	return ret;
}

// multiply vector by constant
static auto operator*(double fScale, const vector_t& vec)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = fScale * vec[i];

	return ret;
}

// power function
static auto pow(const vector_t& vec, double fPow)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = pow(vec[i], fPow);

	return ret;
}

// sqrt function
static auto sqrt(const vector_t& vec)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = sqrt(vec[i]);

	return ret;
}

// add vector to vector
static auto operator+(const vector_t& vec1, const vector_t& vec2)
{
	// return vec2 if vec1 is null
	if (vec1.size() == 0 && vec2.size() != 0)
		return vec2;

	// return vec1 if vec2 is null
	if (vec1.size() != 0 && vec2.size() == 0)
		return vec1;

	// throw error is vector are not the same size
	if (vec1.size() != vec2.size())
		throw InvalidSizeException();

	vector_t ret(vec1.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = vec1[i] + vec2[i];

	return ret;
}

// add constant to vector
static auto operator+(const vector_t& vec, double fOffset)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = vec[i] + fOffset;

	return ret;
}

// add constant to vector
static auto operator+(double fOffset, const vector_t& vec)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = fOffset + vec[i];

	return ret;
}

// subtract vector from vector
static auto operator-(const vector_t& vec1, const vector_t& vec2)
{
	// return -vec2 if vec1 is null
	if (vec1.size() == 0 && vec2.size() != 0)
	{
		vector_t ret(vec2.size());

		for (size_t i = 0; i < ret.size(); i++)
			ret[i] = -vec2[i];

		return ret;
	}

	// return +vec1 if vec2 is null
	if (vec1.size() != 0 && vec2.size() == 0)
		return vec1;

	// throw error is vector are not the same size
	if (vec1.size() != vec2.size())
		throw InvalidSizeException();

	vector_t ret(vec1.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = vec1[i] - vec2[i];

	return ret;
}

// subtract constant to vector
static auto operator-(const vector_t& vec, double fOffset)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = vec[i] - fOffset;

	return ret;
}

// subtract constant to vector
static auto operator-(double fOffset, const vector_t& vec)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = fOffset - vec[i];

	return ret;
}

// divide vector by constant
static auto operator/(const vector_t &vec, double fScale)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = (fScale == 0) ? 0 : vec[i] / fScale;

	return ret;
}

// add two vectors
static const vector_t& operator+=(vector_t& vec1, const vector_t& vec2)
{
	// if vec1 is null, copy vec2 into it
	if (vec1.size() == 0)
		vec1 = vec2;

	// otherelse add vectors
	else
	{
		// throw error if vectors are not the same size
		if (vec1.size() != vec2.size())
			throw InvalidSizeException();

		// add each element
		for (size_t i = 0; i < vec1.size(); i++)
			vec1[i] += vec2[i];
	}

	// return vector
	return vec1;
}

static const vector_t& operator-=(vector_t& vec1, const vector_t& vec2)
{
	// if vec1 is null, copy -vec2 into it
	if (vec1.size() == 0)
	{
		vec1.resize(vec2.size());

		for (size_t i = 0; i < vec1.size(); i++)
			vec1[i] = -vec2[i];
	}
	// otherelse subtract vectors
	else
	{
		// throw error if vectors are not the same size
		if (vec1.size() != vec2.size())
			throw InvalidSizeException();

		// subtract each elements
		for (size_t i = 0; i < vec1.size(); i++)
			vec1[i] -= vec2[i];
	}

	return vec1;
}

// return vector if 'nSize' elements that goes linearly from fMin to fMax
static auto linspace(double fMin, double fMax, size_t nSize)
{
	// require at least 2 elements
	if (nSize <= 1)
		throw InvalidSizeException();

	// return linear interpolation
	vector_t ret(nSize);

	for (size_t i = 0; i < nSize; i++)
	{
		double p = (double)i / (double)(nSize - 1);

		ret[i] = p * fMax + (1 - p) * fMin;
	}

	return ret;
}

// return maximum of vector
static double maxof(const vector_t& rArray)
{
	// throw exception if size is null
	if (rArray.size() == 0)
		throw InvalidSizeException();

	// get maximum
	double fRet = rArray[0];

	for (size_t i = 1; i < rArray.size(); i++)
		fRet = max(fRet, rArray[i]);

	return fRet;
}

// return minimum of vector
static double minof(const vector_t& rArray)
{
	// throw exception if size is null
	if (rArray.size() == 0)
		return 0;

	// get minimum
	double fRet = rArray[0];

	for (size_t i = 1; i < rArray.size(); i++)
		fRet = min(fRet, rArray[i]);

	return fRet;
}

// maximum of two vectors
static auto maxvec(const vector_t& vec1, const vector_t& vec2)
{
	// return vec2 if vec1 is null
	if (vec1.size() == 0 && vec2.size() != 0)
		return vec2;

	// return vec1 if vec2 is null
	if (vec1.size() != 0 && vec2.size() == 0)
		return vec1;

	// throw error is vector are not the same size
	if (vec1.size() != vec2.size())
		throw InvalidSizeException();

	vector_t ret(vec1.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = max(vec1[i], vec2[i]);

	return ret;
}

// minimum of two vectors
static auto minvec(const vector_t& vec1, const vector_t& vec2)
{
	// return vec2 if vec1 is null
	if (vec1.size() == 0 && vec2.size() != 0)
		return vec2;

	// return vec1 if vec2 is null
	if (vec1.size() != 0 && vec2.size() == 0)
		return vec1;

	// throw error is vector are not the same size
	if (vec1.size() != vec2.size())
		throw InvalidSizeException();

	vector_t ret(vec1.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = min(vec1[i], vec2[i]);

	return ret;
}

// convolution of vector with kernel
static auto conv(const vector_t& rInput, const vector_t& rKernel)
{
	// create temp vector
	vector_t ret(rInput.size());

	// get size of input kernel
	size_t nKernelSize = rKernel.size();

	// scan all element of original vector
	for (size_t i = 0; i < rInput.size(); i++)
	{
		// initialize element to zero
		ret[i] = 0;

		// browse vector
		for (size_t j = 0; j < nKernelSize; j++)
			ret[i] += rKernel[j] * rInput[bound((int)(i+j) - (int)(nKernelSize>>1), 0, (int)rInput.size() - 1)];
	}

	// return temp vector
	return ret;
}

// boxcar lowpass filter on vector
static auto boxcar(const vector_t& vec, size_t nKernelSize)
{
	// return original vec if size is lower or equal to 1 (1: no effect, 0: undefined behavior)
	if (nKernelSize <= 1)
		return vec;

	// create kernel
	vector_t kernel(nKernelSize);

	for (auto& v : kernel)
		v = 1.0 / (double)nKernelSize;

	// return convolution
	return conv(vec, kernel);
}

// power
static auto power(const vector_t& vec, double fPower)
{
	vector_t ret(vec.size());

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] = pow(vec[i], fPower);

	return ret;
}

// sum
static auto sum(const vector_t& vec)
{
	double fSum = 0;

	for (auto& v : vec)
		fSum += v;

	return fSum;
}

// mean value
static auto mean(const vector_t& vec)
{
	if (vec.size() == 0)
		throw InvalidSizeException();

	return sum(vec) / vec.size();
}