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

#include <array>
#include <functional>

#include "../utils/exception.h"

template<size_t N> auto operator+(const std::array<double, N>& vec1, const std::array<double, N>& vec2)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec1[i] + vec2[i];

	return ret;
}

template<size_t N> auto operator+(const std::array<double, N>& vec, double fOffset)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec[i] + fOffset;

	return ret;
}

template<size_t N> auto operator+(double fOffset, const std::array<double, N>& vec)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = fOffset + vec[i];

	return ret;
}

template<size_t N> auto operator+=(std::array<double, N>& vec1, const std::array<double, N>& vec2)
{
	for (size_t i = 0; i < N; i++)
		vec1[i] += vec2[i];

	return vec1;
}

template<size_t N> auto operator-(const std::array<double, N>& vec1, const std::array<double, N>& vec2)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec1[i] - vec2[i];

	return ret;
}

template<size_t N> auto operator-(const std::array<double, N>& vec, double fOffset)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec[i] - fOffset;

	return ret;
}

template<size_t N> auto operator-(double fOffset, const std::array<double, N>& vec)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = fOffset - vec[i];

	return ret;
}

template<size_t N> auto operator-=(std::array<double, N>& vec1, const std::array<double, N>& vec2)
{
	for (size_t i = 0; i < N; i++)
		vec1[i] -= vec2[i];

	return vec1;
}

template<size_t N> auto operator*(double fMult, const std::array<double, N>& vec)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = fMult * vec[i];

	return ret;
}

template<size_t N> auto operator*(const std::array<double, N>& vec, double fMult)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec[i] * fMult;

	return ret;
}

template<size_t N> auto operator*=(std::array<double, N>& vec, double fMult)
{
	for (size_t i = 0; i < N; i++)
		vec[i] /= fMult;

	return vec;
}

template<size_t N> auto operator/(const std::array<double, N>& vec, double fDivider)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = vec[i] / fDivider;

	return ret;
}

template<size_t N> auto operator/=(std::array<double, N>& vec, double fDivider)
{
	for (size_t i = 0; i < N; i++)
		vec[i] /= fDivider;

	return vec;
}

template<size_t N> auto randomize(const std::array<double, N>& vec_min, const std::array<double, N>& vec_max)
{
	std::array<double, N> ret;

	for (size_t i = 0; i < N; i++)
		ret[i] = randomize(vec_min[i], vec_max[i]);

	return ret;
}

/*
 *	uncontrained direct simplex optimization
 *
 *	Implemented according to the description of "Convergence Properties of the Nelder-Mead Simplex Method in Low Dimensions" by C. LAGARIAS
 *	This implementation is the same as the one used in Matlab. However, for some reasons, I cannot reproduce the exact same stopping conditions.
 */
template<size_t N> std::array<double, N> fminsearch(std::function< double(const std::array<double, N>&)> pCostFunction, const std::array<double, N>& rInitialVector, size_t nMaxIterations = 0, double fTolX = 1.0e-4, double fTolFun = 1.0e-4)
{
	const double fReflectionCoeff = 1;
	const double fExpansionCoeff = 2;
	const double fContractionCoeff = 0.5;
	const double fShrinkageCoeff = 0.5;

	struct simplex_s
	{
		std::array<double, N> x;
		double cost;
	};

	// InvalidVectorException class
	class InvalidVectorException : public IException
	{
	public:
		virtual std::string toString(void) const override
		{
			return "Invalid initial vector!";
		}
	};

	// check size
	if (N == 0)
		throwException(InvalidVectorException);

	// follow Matlab conventions
	if (nMaxIterations == 0)
		nMaxIterations = __MULT(N, (size_t)200);

	// distance between two vectors
	auto dist = [](const std::array<double, N>& vec1, const std::array<double, N>& vec2)
	{
		double fLength = 0;

		for (size_t i = 0; i < N; i++)
			fLength += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);

		return sqrt(fLength);
	};

	// length of vectors
	auto length = [](const std::array<double, N>& vec)
	{
		double fLength = 0;

		for (size_t i = 0; i < N; i++)
			fLength += vec[i] * vec[i];

		return sqrt(fLength);
	};

	// exit condition following matlab convention
	auto matlab_compare = [=](struct simplex_s& curr, struct simplex_s& prev)
	{
		bool condition1 = dist(curr.x, prev.x) < fTolX * (1.0 + length(prev.x));
		bool condition2 = fabs(curr.cost - prev.cost) < fTolFun * (1.0 + fabs(prev.cost));

		return condition1 & condition2;
	};

	// return best of two solution
	auto best = [](struct simplex_s& a, struct simplex_s& b)
	{
		if (a.cost < b.cost)
			return a.x;

		return b.x;
	};

	// build initial simplex
	std::array<struct simplex_s, N+1> curr_simplex;

	curr_simplex[N].x = rInitialVector;

	for (size_t i = 0; i < N; i++)
	{
		// copy initial vector to current simplex
		curr_simplex[i].x = rInitialVector;

		// set coordinate #i to 0.00025 if null or increase by 5%
		curr_simplex[i].x[i] = (curr_simplex[i].x[i] == 0) ? 0.00025 : rInitialVector[i] * 1.05;
	}

	// evaluate simplex
	for (auto& v : curr_simplex)
		v.cost = pCostFunction(v.x);

	// loop
	for (size_t nIter = 0; nIter < nMaxIterations; nIter++)
	{
		// sort by ascending order
		std::sort(curr_simplex.begin(), curr_simplex.end(), [](struct simplex_s& rA, struct simplex_s& rB)
			{
				return rA.cost < rB.cost;
			});

		// compute mean pos on 'n' best points
		auto xm = curr_simplex[0].x;

		for (size_t i = 1; i < N; i++)
			xm += curr_simplex[i].x;

		xm /= (double)N;

		// reflect
		struct simplex_s reflect;

		reflect.x = xm + fReflectionCoeff * (xm - curr_simplex[N].x);
		reflect.cost = pCostFunction(reflect.x);

		// accept reflection
		if (reflect.cost >= curr_simplex[0].cost && reflect.cost < curr_simplex[N - 1].cost)
		{
			// check for convergence
			if (matlab_compare(reflect, curr_simplex[N]))
				return curr_simplex[0].x;

			// otherelse replace last point
			curr_simplex[N] = reflect;

			continue;
		}

		// expand
		if (reflect.cost < curr_simplex[0].cost)
		{
			struct simplex_s expand;

			expand.x = xm + fExpansionCoeff * (reflect.x - xm);
			expand.cost = pCostFunction(expand.x);

			// accept expansion
			if (expand.cost < reflect.cost)
			{
				// check for convergence
				if (matlab_compare(curr_simplex[N], expand))
					return expand.x;

				curr_simplex[N] = expand;
			}
			// accept reflection
			else
			{
				// check for convergence
				if (matlab_compare(curr_simplex[N], reflect))
					return reflect.x;

				curr_simplex[N] = reflect;
			}

			// otherelse replace last point
			continue;
		}

		// contract
		if (reflect.cost >= curr_simplex[N - 1].cost)
		{
			struct simplex_s contract;

			// inside contraction
			if (reflect.cost >= curr_simplex[N].cost)
			{
				contract.x = xm - fContractionCoeff * (xm - curr_simplex[N].x);
				contract.cost = pCostFunction(contract.x);

				// accept contraction
				if (contract.cost < curr_simplex[N].cost)
				{
					// check for convergence
					if (matlab_compare(curr_simplex[N], contract))
						return curr_simplex[0].x;

					// otherelse replace last point
					curr_simplex[N] = contract;

					continue;
				}
			}
			// outside contraction
			else
			{
				contract.x = xm + fContractionCoeff * (reflect.x - xm);
				contract.cost = pCostFunction(contract.x);

				// accept contraction
				if (contract.cost <= reflect.cost)
				{
					// check for convergence
					if (matlab_compare(curr_simplex[N], contract))
						return curr_simplex[0].x;

					// otherelse replace last point
					curr_simplex[N] = contract;

					continue;
				}
			}

			// shrink
			for (size_t i = 1; i <= N; i++)
			{
				curr_simplex[i].x = curr_simplex[0].x + fShrinkageCoeff * (curr_simplex[i].x - curr_simplex[0].x);
				curr_simplex[i].cost = pCostFunction(curr_simplex[i].x);
			}
		}
	}

	// return best vector
	return curr_simplex[0].x;
}

// global search method using randomly sampled points and local search method
template<size_t N> std::array<double, N> globalsearch(std::function< std::array<double, N>(std::function< double(const std::array<double, N>&)>, const std::array<double, N>&)> pLocalSearchFunction, std::function< double(const std::array<double, N>&)> pCostFunction, std::function< bool(const std::array<double, N>&)> pConstraintsFunction, const std::array<double, N>& rMinBounds, const std::array<double, N>& rMaxBounds, size_t nNumSamples)
{
	// hold best result
	std::array<double, N> bestCoefficients;
	double fBestCost = 0;

	// generate solutions
	for (size_t i = 0; i < nNumSamples; i++)
	{
		// randomize starting position
		auto start_coeffs = randomize(rMinBounds, rMaxBounds);

		// use local search method
		auto fit_coeffs = pLocalSearchFunction(pCostFunction, start_coeffs);

		// skip if not in bounds
		if (pConstraintsFunction && !pConstraintsFunction(fit_coeffs))
			continue;

		// compute cost
		auto fCost = pCostFunction(fit_coeffs);

		// save best
		if (i == 0 || fCost < fBestCost)
		{
			bestCoefficients = fit_coeffs;
			fBestCost = fCost;
		}
	}

	// return best coefficients
	return bestCoefficients;
}