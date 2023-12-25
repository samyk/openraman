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

// types of baseline removal algorithm
enum class BaselineRemovalAlgorithm
{
	Schulze,
};

// NoBaselineFoundException class
class NoBaselineFoundException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "No baseline could be found!";
	}
};

// baseline correction based on Schulze, H. Georg, et al. "A small-window moving average-based fully automated baseline estimation method for Raman spectra." Applied spectroscopy 66.7 (2012): 757-764.
static vector_t baseline_schulze(const vector_t& vec)
{
	// trapezoidal integration
	auto trapz = [](const vector_t& vec)
	{
		double fIntegral = 0;

		if (vec.size() == 0)
			return (double)0;

		for (size_t i = 0; i < vec.size()-1; i++)
			fIntegral += 0.5 * (vec[i] + vec[i + 1]);

		return fIntegral;
	};

	// initialize filtered spectrum to input vector
	auto S = vec;

	// keep last 3 results into circular buffer
	struct
	{
		vector_t baseline;
		double cost;
	} data[3];

	// loop until solution has been found
	size_t i = 0;

	while ((i + 1) * 2 < vec.size())
	{
		auto& curr = data[i % 3];

		// remove peaks
		curr.baseline = minvec(S, boxcar(S, (i + 1) * 2));

		// compute cost and add to list
		curr.cost = trapz(S - curr.baseline);

		// set new filtered spectrum
		S = curr.baseline;

		// end condition is middlepoint having the lowest cost
		if (i >= 3 && data[(i - 1) % 3].cost < data[(i - 2) % 3].cost && data[(i - 1) % 3].cost < curr.cost)
			return data[(i - 1) % 3].baseline;

		// increment iteration
		i++;
	}

	throwException(NoBaselineFoundException);
}

// generic baseline removal dispatch
static vector_t baseline(const vector_t& vec, BaselineRemovalAlgorithm eAlgorithm)
{
	switch (eAlgorithm)
	{
	case BaselineRemovalAlgorithm::Schulze:
		return baseline_schulze(vec);

	default:
		throwException(NoBaselineFoundException);
	}
}
