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
#include <vector>

#include <Windows.h>

#include "exception.h"

// useful constants
#define __PI		3.1415926535897932384626433832795

// lock mutex in scope
#define AUTOLOCK(handle)		std::lock_guard<std::mutex> lock##__LINE__(handle);

// get time
inline double getTime(void)
{
	return 1e-3 * (double)GetTickCount();
}

// randomize 64bits number
static size_t rand64(void)
{
	size_t ret = 0;

	for (size_t i = 0; i < 64; i += 8)
		ret |= ((size_t)(rand() % 0xFF)) << i;

	return ret;
}

// randomize number
static double randomize(double fMin, double fMax)
{
	const int granularity = 1000000000;

	double p = (double)(rand64() % granularity) / (double)granularity;

	return p * fMin + (1 - p) * fMax;
}

// return true if string ends with
static bool strEndsWith(const char* pszString, const char* pszEndsWith)
{
	size_t n1 = strlen(pszString);
	size_t n2 = strlen(pszEndsWith);

	if (n1 < n2)
		return false;

	return _strnicmp(pszString + n1 - n2, pszEndsWith, n2) == 0;
}

// convert degrees to radians
inline double deg2rad(double fAngle)
{
	return fAngle * (__PI / 180.0);
}

// convert radians to degrees
inline double rad2deg(double fAngle)
{
	return fAngle * (180.0 / __PI);
}

// return -1 or +1 depending on the sign of 'fValue'
inline double sign(double fValue)
{
	return (fValue < 0) ? -1 : +1;
}

// return ceil(a/b) for ints
inline int iceildiv(int a, int b)
{
	return (a + b - 1) / b;
}

// return max(minval, ofs - i) for size_t types
static size_t lobound(size_t i, size_t ofs, size_t minval)
{
	return (size_t)max((int)minval, (int)i - (int)ofs);
}

// return min(maxval, ofs + i) for size_t types
static size_t hibound(size_t i, size_t ofs, size_t maxval)
{
	return min(maxval, i + ofs);
}

// bound items to [minval, maxval]
static size_t bound(int index, int minval, int maxval)
{
	if (index < minval)
		return (size_t)minval;

	if (index > maxval)
		return (size_t)maxval;

	return (size_t)index;
}

// generic template to swap data
template<typename Type> void myswap(Type& a, Type& b)
{
	Type temp = a;
	a = b;
	b = temp;
}

// return median of array
static double median(double* pData, size_t nData)
{
	if (pData == nullptr || nData == 0)
		return 0;

	// sort array
	for (size_t i = 0; i < nData; i++)
		for (size_t j = i + 1; j < nData; j++)
			if (pData[i] > pData[j])
				myswap(pData[i], pData[j]);

	// get median
	size_t pivot = nData >> 1;

	if ((nData % 2) == 1)
		return pData[pivot];
	else
		return 0.5 * (pData[pivot] + pData[pivot + 1]);
}

// tokenize string
static std::vector<std::string> tokenize(std::string s, char cToken)
{
	std::vector<std::string> ret;

	// return if no string
	if (s.length() == 0)
		return ret;

	// loop until no token found
	size_t nCurrIndex = 0;

	do
	{
		// save previous index
		size_t nPrevIndex = nCurrIndex;

		// add size of token if not null
		if (nPrevIndex > 0)
			nPrevIndex++;

		// find next index
		nCurrIndex = s.find(cToken, nPrevIndex);

		// add substring to ret
		if (nCurrIndex != nPrevIndex)
			ret.emplace_back(s.substr(nPrevIndex, nCurrIndex - nPrevIndex));

	} while (nCurrIndex != std::string::npos);

	// return vector
	return ret;
}

// joind string with separator between elements
static std::string concat_string(const std::vector<std::string>& vec, const std::string& sep)
{
	std::string ret = "";

	for (auto it = vec.begin(); it < vec.end(); it++)
	{
		if (it != vec.begin())
			ret += sep;

		ret += *it;
	}

	return ret;
}

// convert vector to array
template<typename Type, size_t N> std::array<Type, N> vector2array(const std::vector<Type>& rVector)
{
	// check size
	if (rVector.size() != N)
	{
		class WrongSizeException : public IException
		{
		public:
			WrongSizeException(const std::string& rType, size_t nSize1, size_t nSize2)
			{
				this->m_type = rType;
				this->m_nSize1 = nSize1;
				this->m_nSize2 = nSize2;
			}

			virtual std::string toString(void) const override
			{
				char szSize1[32], szSize2[32];

				sprintf_s(szSize1, "%zu", this->m_nSize1);
				sprintf_s(szSize2, "%zu", this->m_nSize2);

				return std::string("Cannot convert std::vector<") + this->m_type + std::string(">(") + szSize1 + (") to std::array<") + this->m_type + std::string(",") + szSize2 + std::string("> !");
			}

		private:
			std::string m_type;
			size_t m_nSize1, m_nSize2;
		};

		throw WrongSizeException(typeid(Type).name(), rVector.size(), N);
	}

	// create copy
	std::array<Type, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = rVector[i];

	return ret;
}

// convert array to vector
template<typename Type, size_t N> std::vector<Type> array2vector(const std::array<Type, N>& rArray)
{
	return std::vector<Type>(rArray.begin(), rArray.end());
}

// compute simple checksum
static unsigned char checksum8(unsigned char* pData, size_t nSize)
{
	unsigned char ret = 0;

	while (nSize--)
		ret += ~*(pData++);

	return ~ret;
}

// split path into directory + file
static auto splitFilePath(std::string sFilename)
{
	// return struct
	struct
	{
		std::string sDirectory, sFile;
	} ret_s;

	// find last backslash
	auto pos = sFilename.find_last_of("/\\");

	// divide string
	if (pos == std::string::npos)
		ret_s.sFile = sFilename;
	else
	{
		ret_s.sDirectory = sFilename.substr(0, pos);
		ret_s.sFile = sFilename.substr(pos + 1);
	}

	// return
	return ret_s;
}

// split path into file + ext
static auto splitFileExt(std::string sFilename)
{
	// return struct
	struct
	{
		std::string sFile, sExt;
	} ret_s;

	// find last backslash
	auto pos = sFilename.find_last_of(".");

	// divide string
	if (pos == std::string::npos)
		ret_s.sFile = sFilename;
	else
	{
		ret_s.sFile = sFilename.substr(0, pos);
		ret_s.sExt = sFilename.substr(pos + 1);
	}

	// return
	return ret_s;
}

// split path into directory + file + ext
static auto splitFileParts(std::string sFilename)
{
	// return struct
	struct
	{
		std::string sDirectory, sFile, sExt;
	} ret_s;

	// first split filename into directory and filename
	auto tmp1 = splitFilePath(sFilename);

	ret_s.sDirectory = std::move(tmp1.sDirectory);

	// then split filename itself into file and extension
	auto tmp2 = splitFileExt(tmp1.sFile);

	ret_s.sFile = std::move(tmp2.sFile);
	ret_s.sExt = std::move(tmp2.sExt);

	// return result
	return ret_s;

}