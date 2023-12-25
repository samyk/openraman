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

#include <functional>

#include "../utils/exception.h"
#include "../utils/thread.h"
#include "../utils/safe.h"

#include "vector.h"
#include "optfuncs.h"
#include "legendre.h"
#include "peaks.h"

// type of reference peaks
enum class CalibrationData
{
    Neon,
    MercuryArgon,
};

// UnknownCalibrationData exception class
class UnknownCalibrationData : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unknown calibration data!";
    }
};

// return reference peaks
static vector_t getCalibrationData(CalibrationData eCalibrationData)
{
    switch (eCalibrationData)
    {
    case CalibrationData::Neon:
        return { 585.249, 588.189, 594.483, 597.553, 603.000, 607.434, 609.616, 614.306, 616.359, 621.728, 626.649, 630.479, 633.443, 638.299, 640.225, 650.653, 653.288, 659.895, 667.828, 671.704, 692.947, 703.241, 717.394, 724.517, 743.890 };

    case CalibrationData::MercuryArgon:
        return { 253.65, 296.73, 302.15, 313.16, 334.15, 365.01, 404.66, 435.84, 546.08, 576.96, 579.07, 696.54, 738.40, 750.39, 763.51, 772.40, 794.82, 800.62, 811.53, 826.45, 842.46, 912.30 };

    default:
		throwException(UnknownCalibrationData);
    }
}

// project indices into wavelengths according to polynomial a[0] + a[1] * x + a[2] * x² + ...
template<size_t N> double index2wavelength(const std::array<double, N>& rModelCoeffs, double fIndex)
{
    double fProj = 0;

    for (size_t i = 0; i < N; i++)
        fProj += rModelCoeffs[i] * legendre((unsigned int)i, fIndex);
        // fProj += rModelCoeffs[i] * quickpow(fIndex, (unsigned int)i);

    return fProj;
}

// return model coefficient of determination
template<size_t N> double getCalibrationModelR2(const std::array<double, N>& rModelCoeffs, const vector_t& rPeakIndices, const vector_t& rPeakWavelengths)
{
    // skip if no coefficients
    if (rPeakIndices.size() == 0 || rPeakWavelengths.size() == 0)
        return 0;

    vector_t proj(rPeakIndices.size()), closest(rPeakIndices.size());

    // project and attribute peaks from list
    for (size_t i = 0; i < rPeakIndices.size(); i++)
    {
        // project index to wavelength
        proj[i] = index2wavelength(rModelCoeffs, rPeakIndices[i]);

        // get closest peak to projection
        closest[i] = getClosestPeak(rPeakWavelengths, proj[i]);
    }

    // compute total and resiue
    double res = sum(power(closest - proj, 2));
    double tot = sum(power(closest - mean(closest), 2));

    // return coefficient of determination
    return 1.0 - res / tot;
}

// return RMS error
template<size_t N> double getCalibrationModelRMS(const std::array<double, N>& rModelCoeffs, const vector_t& rPeakIndices, const vector_t& rPeakWavelengths)
{
    // skip if no coefficients
    if (rPeakIndices.size() == 0 || rPeakWavelengths.size() == 0)
        return 0;

    vector_t proj(rPeakIndices.size()), closest(rPeakIndices.size());

    // project and attribute peaks from list
    for (size_t i = 0; i < rPeakIndices.size(); i++)
    {
        // project index to wavelength
        proj[i] = index2wavelength(rModelCoeffs, rPeakIndices[i]);

        // get closest peak to projection
        closest[i] = getClosestPeak(rPeakWavelengths, proj[i]);
    }

    // compute rms
    return sqrt(mean(power(closest - proj, 2)));
}

// calibrate peaks based on a 'N' degree polynomial
template<size_t N> std::array<double, N> calibratePeaks(const vector_t& rPeakIndices, const vector_t& rPeakWavelengths, const std::array<double, N>& rMinVector, const std::array<double, N>& rMaxVector, size_t nNumSamples, std::function< bool(const std::array<double, N>&)> pConstraintsFunction)
{
    // cost function
    auto cost = [&](const std::array<double, N>& coeffs)
    {
#if 1
        // initialize cost to zero
        double fCost = 0;

        // compute distance of all peaks
        for (auto& v : rPeakIndices)
            fCost += distPeaks(rPeakWavelengths, index2wavelength(coeffs, v));

        // return cost
        return fCost;
#else
        return 1.0 - getCalibrationModelR2(coeffs, rPeakIndices, rPeakWavelengths);
#endif
    };

    // use fminsearch to optimize cost function
    return globalsearch<N>(std::bind(fminsearch<N>, std::placeholders::_1, std::placeholders::_2, 0, 1e-4, 1e-4), cost, pConstraintsFunction, rMinVector, rMaxVector, nNumSamples);
}

// interface to make global optimization generic
class IGlobalOptimizationThread : public IThread
{
public:
	IGlobalOptimizationThread()
	{
		this->m_bSolutionFound = false;
		this->m_numTests = 0;
		this->m_maxTests = 0;
	}

	virtual vector_t getSolution(size_t nFinalSize) const = 0;

	// return true if solution has been found
	bool hasSolution(void) const
	{
		return this->m_bSolutionFound;
	}

	// return number of processed tests
	size_t getProcessedSamples(void) const
	{
		return this->m_numTests;
	}

	// return number of tests to be processed
	size_t getMaxSamples(void) const
	{
		return this->m_maxTests;
	}

protected:

	// start processing
	virtual void onStart(void) override
	{
		// set no solution flag
		this->m_bSolutionFound = false;

		// clear number of tests
		this->m_numTests = 0;
	}

	// stop processing
	virtual void onStop(void) override
	{

	}

	vector_t m_peaks, m_calibration_data;

	std::atomic<bool> m_bSolutionFound;
	std::atomic<int> m_numTests, m_maxTests;
};

// static solution
class StaticSolution : public IGlobalOptimizationThread
{
public:
	StaticSolution(const vector_t& rSolution)
	{
		this->m_solution = rSolution;
		this->m_bSolutionFound = true;
	}

	virtual vector_t getSolution(size_t nFinalSize) const override
	{
		return this->m_solution;
	}

	virtual void run(void) override
	{
		// do nothing
	}

protected:

	// always stop, nothing to do
	virtual bool stopCondition(void) const override
	{
		return true;
	}

private:
	vector_t m_solution;
};

// global optimization thread class
template<size_t N> class GlobalOptimizationThread : public IGlobalOptimizationThread
{
public:

	// default constructor
	GlobalOptimizationThread(void)
	{
		// clear boundaries by default
		for (size_t i = 0; i < N; i++)
		{
			this->m_minBounds[i] = 0;
			this->m_maxBounds[i] = 0;
		}
	}

	using array_t = std::array<double, N>;

	// retrieve solution
	virtual vector_t getSolution(size_t nFinalSize) const override
	{
		// wait for thread to be finished
		if (isRunning())
			wait();

		// return error if no solution found
		if (!hasSolution())
			throw;

		// prepare solution
		vector_t ret(this->m_bestCoefficients.begin(), this->m_bestCoefficients.end());

		for (size_t i = ret.size(); i < nFinalSize; i++)
			ret.emplace_back(0);

		// otherelse return solution
		return ret;
	}

protected:
	virtual bool inConstraints(const array_t& coeffs) const = 0;

	array_t m_minBounds, m_maxBounds;

private:

	// cost function
	double cost(const array_t& coeffs)
	{
#if 1
		// initialize cost to zero
		double fCost = 0;

		// compute distance of all peaks
		for (auto& v : this->m_peaks)
			fCost += distPeaks(this->m_calibration_data, index2wavelength(coeffs, v));

		// return cost
		return fCost;
#else
		return 1.0 - getCalibrationModelR2(coeffs, rPeakIndices, rPeakWavelengths);
#endif
	}

	// stop condition
	virtual bool stopCondition(void) const override
	{
		return (this->m_numTests > this->m_maxTests);
	}

	// process solution
	virtual void run(void) override
	{
		// increment number of tests
		this->m_numTests++;

		// randomize starting position
		auto start_coeffs = randomize(this->m_minBounds, this->m_maxBounds);

		// use local search method
		auto fit_coeffs = fminsearch<N>(std::bind(&GlobalOptimizationThread<N>::cost, this, std::placeholders::_1), start_coeffs);

		// skip if not in bounds
		if (!inConstraints(fit_coeffs))
			return;

		// compute cost
		auto fCost = cost(fit_coeffs);

		// save best
		if (!this->m_bSolutionFound || fCost < this->m_fBestCost)
		{
			this->m_bestCoefficients = fit_coeffs;
			this->m_fBestCost = fCost;

			this->m_bSolutionFound = true;
		}
	}

	// members
	array_t m_bestCoefficients;

	double m_fBestCost;
};

// linear model
class LinearModelThread : public GlobalOptimizationThread<2>
{
public:
	LinearModelThread(const vector_t& rPeaks, const vector_t& rCalibrationData, double fMinRange, double fMaxRange, double fMinSpan, double fMaxSpan, size_t nNumSampling) : GlobalOptimizationThread()
	{
		this->m_range.fMin = fMinRange;
		this->m_range.fMax = fMaxRange;

		this->m_span.fMin = fMinSpan;
		this->m_span.fMax = fMaxSpan;

		this->m_maxTests = (int)(nNumSampling * nNumSampling);

		this->m_minBounds = { fMinRange, 0.5 * fMinSpan };
		this->m_maxBounds = { fMaxRange, 0.5 * fMaxSpan };

		this->m_peaks = rPeaks;
		this->m_calibration_data = rCalibrationData;
	}

protected:

	// return true if solution respect constraints
	virtual bool inConstraints(const array_t& coeffs) const override
	{
		// plot must range from min range to max range
		if ((coeffs[0] - coeffs[1]) < this->m_range.fMin || (coeffs[0] + coeffs[1]) > this->m_range.fMax)
			return false;

		// check span
		if (coeffs[1] < (0.5 * this->m_span.fMin) || coeffs[1] > (0.5 * this->m_span.fMax))
			return false;

		return true;
	}

private:
	struct
	{
		double fMin, fMax;
	} m_range, m_span;
};

// cubic model
class CubicModelThread : public GlobalOptimizationThread<4>
{
public:
	CubicModelThread(const vector_t& rPeaks, const vector_t& rCalibrationData, double fMinRange, double fMaxRange, double fMinSpan, double fMaxSpan, double fMinDistortion, double fMaxDistortion, size_t nNumSampling) : GlobalOptimizationThread()
	{
		this->m_range.fMin = fMinRange;
		this->m_range.fMax = fMaxRange;

		this->m_span.fMin = fMinSpan;
		this->m_span.fMax = fMaxSpan;

		this->m_distortion.fMin = fMinDistortion;
		this->m_distortion.fMax = fMaxDistortion;

		this->m_maxTests = (int)(nNumSampling * nNumSampling * nNumSampling * nNumSampling);

		this->m_minBounds = { fMinRange, 0.5 * fMinSpan, -fMaxDistortion, -fMaxDistortion };
		this->m_maxBounds = { fMaxRange, 0.5 * fMaxSpan, +fMaxDistortion, +fMaxDistortion };

		this->m_peaks = rPeaks;
		this->m_calibration_data = rCalibrationData;
	}

protected:

	// return true if solution respect constraints
	virtual bool inConstraints(const array_t& coeffs) const override
	{
		// plot must range from min range to max range
		if ((coeffs[0] - coeffs[1]) < this->m_range.fMin || (coeffs[0] + coeffs[1]) > this->m_range.fMax)
			return false;

		// check distortions
		double fDistortion = fabs(coeffs[2]) + fabs(coeffs[3]);

		if (fDistortion < this->m_distortion.fMin || fDistortion > this->m_distortion.fMax)
			return false;

		// check span
		if (coeffs[1] < (0.5 * this->m_span.fMin) || coeffs[1] > (0.5 * this->m_span.fMax))
			return false;

		return true;
	}

private:

	struct
	{
		double fMin, fMax;
	} m_range, m_span, m_distortion;
};