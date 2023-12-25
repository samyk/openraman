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
#include <memory>

#include "shared/utils/exception.h"
#include "shared/math/vector.h"
#include "shared/math/sgolay.h"
#include "shared/math/acc.h"
#include "shared/math/calibration.h"
#include "shared/gui/plot.h"

#include "app.h"
#include "state.h"
#include "camconfig.h"
#include "winmain.h"
#include "spc.h"
#include "exception.h"

// IPlotBuilder interface class
class IPlotBuilder : public SpectrumAnalyzerChild
{
public:

    // build method
    void build(std::shared_ptr<guiPlot> pPlot)
    {
        // skip if no plot
        if (pPlot == nullptr)
            return;

        // clear series
        pPlot->series.clear();

        // call virtual function
        buildPlot(pPlot);

        // add annotations
        pPlot->series.insert(pPlot->series.end(), this->m_annotations.begin(), this->m_annotations.end());
    }

    // clear all annotations
    void clearAnnotations(void)
    {
        // clear annotations
        this->m_annotations.clear();
    }

    // add annotations
    size_t addAnnotation(const guiSignal &rSignal)
    {
        size_t nSize = this->m_annotations.size();

        this->m_annotations.push_back(rSignal);

        return nSize;
    }

    // get annotation
    guiSignal& getAnnotation(size_t nIndex)
    {
        return this->m_annotations[nIndex];
    }

    // export to string
    std::string toString(std::shared_ptr<guiPlot> pPlot, char cSeparator=',') const
    {
        // skip if plot is null
        if (pPlot == nullptr || pPlot->series.size() < 1)
            throw NullDataException();

        char szTmp[64];

        sprintf_s(szTmp, "%c", cSeparator);

        std::string ret, sep = std::string(szTmp), lf = std::string("\r\n");

        // write header
        ret += pPlot->haxis1.title.text + sep + pPlot->vaxis1.title.text + lf;

        // write data
        size_t n = max(pPlot->series[0].x.size(), pPlot->series[0].y.size());

        for (size_t i = 0; i < n; i++)
        {
            // write x
            if (i < pPlot->series[0].x.size())
            {
                sprintf_s(szTmp, "%.5e", pPlot->series[0].x[i]);

                ret += std::string(szTmp);
            }

            // write separator
            ret += sep;

            // write y
            if (i < pPlot->series[0].y.size())
            {
                sprintf_s(szTmp, "%.5e", pPlot->series[0].y[i]);

                ret += std::string(szTmp);
            }

            // write line feed
            if (i + 1 < n)
                ret += lf;
        }

        // return text
        return ret;
    }

    // create spectre file function
    virtual SpectreFile createSpectreFile(void) const = 0;

    // build funnction
    virtual void buildPlot(std::shared_ptr<guiPlot> pPlot) const = 0;

    // return true is saturation button should be enable
    virtual bool hasSaturationOpt(void) const = 0;

    // return true if axis button should be enable
    virtual bool hasAxisChangeOpt(void) const = 0;

    // return true if blank button should be enable
    virtual bool hasBlankOpt(void) const = 0;

    // get blank data
    virtual vector_t getBlankData(void) const = 0;

    // automatic axis span based on vector, return false in case of failure, force min/max
    bool span_ex(guiIRenderAxis& rAxis, const vector_t& vec, double fMin, double fMax, double fNumMajorTicks = 5.0, double fNumMinorTicks = 25.0) const
    {
        // set min/max
        rAxis.minval = fMin;
        rAxis.maxval = fMax;

        // return error if max = min
        if (fabs(rAxis.maxval - rAxis.minval) < 1e-10)
            return false;

        // set major/minor ticks marks
        rAxis.major = fabs(rAxis.maxval - rAxis.minval) / fNumMajorTicks;
        rAxis.minor = fabs(rAxis.maxval - rAxis.minval) / fNumMinorTicks;

        // return true for success
        return true;
    }

    // return true if showing saturation data
    bool showSaturationData(void) const
    {
        return isShowSaturationDataEnabled();
    }

    // automatic axis span based on vector, return false in case of failure
    bool span(guiIRenderAxis& rAxis, const vector_t& vec, double fNumMajorTicks=5.0, double fNumMinorTicks=25.0) const
    {
        // call extended span function
        return span_ex(rAxis, vec, minof(vec), maxof(vec), fNumMajorTicks, fNumMinorTicks);
    }

    // format data
    vector_t format(const vector_t& vec) const
    {
        // copy vector first
        auto y = vec;

        // remove blank if any
        if (isBlankRemovalEnabled() && hasBlank())
            y -= getBlank();

        // apply lowpass
        y = boxcar(y, getSmoothing());

        // apply baseline removal if enabled
        if (isBaselineRemovalEnabled())
        {
            try
            {
                y -= baseline(y, BaselineRemovalAlgorithm::Schulze);
            }
            catch (...) {}
        }

        // apply sgolay if enabled
        if (isSGolayEnable())
        {
            try
            {
                y = sgolay(y, getSGolayWindowSize(), getSGolayOrder(), getSGolayDerivative());
            }
            catch (...) {}
        }

        // return output vector
        return y;
    }

    // generate axis data
    auto genAxis(size_t n) const
    {
        // return struct
        struct
        {
            double fMin, fMax;

            vector_t vec;
            std::function<std::string(double)> format;
            std::string title;
        } ret;

        // get axis type
        auto eAxisType = getAxisType();

        if (!hasCalibrationData())
            eAxisType = AxisType::Pixels;

        // resize output vector
        ret.vec.resize(n);

        // dispatch axis type
        switch (eAxisType)
        {
        case AxisType::Wavelengths:

            // set title
            ret.title = AXIS_WAVELENGTHS;

            // generate axis vector
            for (size_t i = 0; i < n; i++)
                ret.vec[i] = index2wavelength(getSolution(), 2 * (double)i / (double)(n - 1) - 1);

            // set minimum and maximum
            ret.fMin = minof(ret.vec);
            ret.fMax = maxof(ret.vec);

            // display formula for labels
            ret.format = [](double val)
            {
                char szTmp[64];

                sprintf_s(szTmp, "%.0f", val);

                return std::string(szTmp);
            };
            break;

        case AxisType::RamanShifts:

            // set title
            ret.title = AXIS_RAMANSHIFTS;

            // generate axis vector
            for (size_t i = 0; i < n; i++)
                ret.vec[i] = 1e7 * (1 / getRamanWavelength() - 1 / index2wavelength(getSolution(), 2 * (double)i / (double)(n - 1) - 1));

            // set minimum and maximum, specific case for Raman spectroscopy
            ret.fMin = 3500;
            ret.fMax = 500;

            // display formula for labels
            ret.format = [](double val)
            {
                char szTmp[64];

                sprintf_s(szTmp, "%.0f", val);

                return std::string(szTmp);
            };
            break;

        default:
        case AxisType::Pixels:

            // set title
            ret.title = AXIS_PIXELS;

            // generate axis vector
            for (size_t i = 0; i < n; i++)
                ret.vec[i] = (double)i;

            // set minimum and maximum
            ret.fMin = minof(ret.vec);
            ret.fMax = maxof(ret.vec);

            // display formula for labels
            ret.format = [](double val)
            {
                char szTmp[64];

                sprintf_s(szTmp, "%.0f", val);

                return std::string(szTmp);
            };
            break;
        }

        return ret;
    }

private:
    std::vector<guiSignal> m_annotations;
};
