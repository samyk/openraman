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

#include <stack>
#include <vector>
#include <algorithm>

#include "vector.h"

// refine peak positions to sub-pixels
static auto refinePeaksPosition(const vector_t& x, const vector_t& y, const std::vector<size_t>& peaks)
{
	// output vector
	struct
	{
		vector_t x, y;
	} ret_s;

	ret_s.x = vector_t(peaks.size());
	ret_s.y = vector_t(peaks.size());

	// prefill output vector
	for (size_t i = 0; i < peaks.size(); i++)
	{
		ret_s.x[i] = (double)peaks[i];

		if (peaks[i] >= 0 && peaks[i] < y.size())
			ret_s.y[i] = y[peaks[i]];
		else
			ret_s.y[i] = 0;
	}

	// do nothing is size is less than 3 pixels
	if (x.size() < 3 || x.size() != y.size())
		return ret_s;

	// scan all peaks
	for (size_t i = 0; i < peaks.size(); i++)
	{
		// values for the fit
		size_t pos[3];

		if (peaks[i] == 0)
		{
			pos[0] = 0;
			pos[1] = 1;
			pos[2] = 2;
		}
		else if (peaks[i] == x.size() - 1)
		{
			pos[0] = x.size() - 3;
			pos[1] = x.size() - 2;
			pos[2] = x.size() - 1;
		}
		else
		{
			pos[0] = peaks[i] - 1;
			pos[1] = peaks[i];
			pos[2] = peaks[i] + 1;
		}

		// compute pairs of x and y
		double x1 = (double)x[pos[0]];
		double x2 = (double)x[pos[1]];
		double x3 = (double)x[pos[2]];

		auto y1 = y[pos[0]];
		auto y2 = y[pos[1]];
		auto y3 = y[pos[2]];

		// solve quadratic system
		auto d = (x1 - x2) * (x1 - x3) * (x2 - x3);

		if (fabs(d) < 1e-10)
			continue;

		auto A = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / d;
		auto B = (x1 * x1 * (y2 - y3) + x3 * x3 * (y1 - y2) + x2 * x2 * (y3 - y1)) / d;
		auto C = (x2 * x2 * (x3 * y1 - x1 * y3) + x2 * (x1 * x1 * y3 - x3 * x3 * y1) + x1 * x3 * (x3 - x1) * y2) / d;

		// compute maximum from terms
		ret_s.x[i] = -B / (2 * A);
		ret_s.y[i] = A * ret_s.x[i] * ret_s.x[i] + B * ret_s.x[i] + C;
	}

	// return vector
	return ret_s;
}

// find peaks in vector
static std::vector<size_t> findPeaks(const vector_t& vec, size_t nNumPeaks, double fThreshold)
{
	// find maximum of masked vector
	auto find_max = [](const vector_t& vec, const std::vector<bool>& mask)
	{
		struct
		{
			double fValue;
			size_t nPos;
			bool bError;
		} ret;

		ret.bError = true;

		// loop through vector
		for (size_t i = 0; i < vec.size(); i++)
		{
			// check for maximum
			if (mask[i] && (ret.bError || vec[i] > ret.fValue))
			{
				ret.fValue = vec[i];
				ret.nPos = i;
				ret.bError = false;
			}
		}

		return ret;
	};


	// return vector of positions
	std::vector<size_t> peaks;

	// mask for peaks are initialize true
	std::vector<bool> mask(vec.size());

	for (size_t i = 0; i < mask.size(); i++)
		mask[i] = true;

	// look for nNumPeaks number of peaks
	while (nNumPeaks--)
	{
		// find maximum using mask
		auto m = find_max(vec, mask);

		// skip if error
		if (m.bError)
			continue;

		// add to list
		peaks.emplace_back(m.nPos);

		// mask from right of peak
		{
			bool bHasReachedThreshold = false;
			double fMinValue = 0;

			for (size_t i = m.nPos; i < vec.size(); i++)
			{
				// threshold has been reached
				if (!bHasReachedThreshold && vec[i] < (fThreshold * m.fValue))
				{
					bHasReachedThreshold = true;
					fMinValue = vec[i];
				}

				// record minimum values
				if (bHasReachedThreshold)
					fMinValue = min(fMinValue, vec[i]);

				// break if new maximum
				if (bHasReachedThreshold && (vec[i] * fThreshold) > fMinValue)
					break;

				// invalidate mask there
				mask[i] = false;
			}
		}

		// mask from left of peak
		{
			bool bHasReachedThreshold = false;
			double fMinValue = 0;

			for (size_t i = m.nPos; i >= 0; i--)
			{
				// break if below threshold
				if (!bHasReachedThreshold && vec[i] < (fThreshold * m.fValue))
				{
					bHasReachedThreshold = true;
					fMinValue = vec[i];
				}

				// record minimum values
				if (bHasReachedThreshold)
					fMinValue = min(fMinValue, vec[i]);

				// break if new maximum
				if (bHasReachedThreshold && (vec[i] * fThreshold) > fMinValue)
					break;

				// invalidate mask there
				mask[i] = false;

				// break if i is zero (DO NOT REMOVE THIS LINE OR INFINITE LOOP WILL OCCUR
				if (i == 0)
					break;
			}
		}
	}

	// return vector
	return peaks;
}

// return distance to peaks
static double distPeaks(const vector_t& peaks, double fPos)
{
	// create temp vector
	vector_t temp(peaks.size());

	// compute distance to all peaks
	for (size_t i = 0; i < peaks.size(); i++)
		temp[i] = fabs(peaks[i] - fPos);

	// return minimum of all distances
	return minof(temp);
}

// return closest peak
static double getClosestPeak(const vector_t& peaks, double fPos)
{
	// create temp vector
	double fMinDist = 0;
	size_t nIndex = 0;

	// skip if null vector
	if (peaks.size() == 0)
		return 0;

	// compute distance to all peaks
	for (size_t i = 0; i < peaks.size(); i++)
	{
		double fDist = fabs(peaks[i] - fPos);

		if (i == 0 || fDist < fMinDist)
		{
			nIndex = i;
			fMinDist = fDist;
		}
	}

	// return peak of minimum distance
	return peaks[nIndex];
}