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

#include "shared/math/vector.h"
#include "shared/math/acc.h"
#include "shared/math/peaks.h"
#include "shared/math/baseline.h"

#include "shared/gui/plot.h"

#include "data.h"
#include "winmain.h"

// CameraDataBuilder class
class CameraDataBuilder : public IPlotBuilder
{
public:
    CameraDataBuilder(const std::string& rUID)
    {
        this->m_uid = rUID;
    }

    // create spectre file
    virtual SpectreFile createSpectreFile(void) const override
    {
        return SpectreFile(this->m_acc_data.mean(), getBlank(), this->m_uid);
    }

    // clear accumulators
    void clear(void)
    {
        this->m_acc_data.reset();
        this->m_acc_sat.reset();
        this->m_acc_roi.reset();
    }

    // add data to data accumulator
    void addSignalData(const vector_t& vec)
    {
        this->m_acc_data.add(vec);
    }

    // add data to saturation accumulator
    void addSaturationData(const vector_t& vec)
    {
        this->m_acc_sat.add(vec);
    }

    // add data to ROI accumulator
    void addROIData(const vector_t& vec)
    {
        this->m_acc_roi.add(vec);
    }

    // enable saturation button
    virtual bool hasSaturationOpt(void) const override
    {
        return true;
    }

    // enable axis change
    virtual bool hasAxisChangeOpt(void) const override
    {
        return true;
    }

    // enable blank option
    virtual bool hasBlankOpt(void) const override
    {
        return true;
    }

    // get blank data
    virtual vector_t getBlankData(void) const override
    {
        return this->m_acc_data.mean();
    }

    // build plot
    virtual void buildPlot(std::shared_ptr<guiPlot> pPlot) const override
    {
        // skip if no plot
        if (pPlot == nullptr)
            return;

        // resize data to 3
        pPlot->series.clear();
        pPlot->series.resize(3);

        // hide all axis by default
        pPlot->haxis1.render_enable = false;
        pPlot->haxis2.render_enable = false;

        pPlot->vaxis1.render_enable = false;
        pPlot->vaxis2.render_enable = false;

        pPlot->series[0].render_enable = false;
        pPlot->series[1].render_enable = false;
        pPlot->series[2].render_enable = false;

        // set series
        setSeries1(pPlot);

        if (showSaturationData())
        {
            setSeries2(pPlot);
            setSeries3(pPlot);
        }

        // select which grid to render
        pPlot->vaxis1.grid.render_enable = pPlot->series[0].render_enable;
        pPlot->vaxis2.grid.render_enable = !pPlot->series[0].render_enable;

        // eventually clear if both fails
        if (!pPlot->series[0].render_enable && !pPlot->series[1].render_enable && !pPlot->series[2].render_enable)
        {
            pPlot->series.clear();

            pPlot->haxis1.render_enable = false;
            pPlot->haxis2.render_enable = false;

            pPlot->vaxis1.render_enable = false;
            pPlot->vaxis2.render_enable = false;
        }
    }

private:

    // set serie1
    void setSeries1(std::shared_ptr<guiPlot> pPlot) const
    {
        // always check plot even if it should be valid
        if (pPlot == nullptr)
            return;

        do
        {
            // set primary axis
            pPlot->series[0].pHorizontalAxis = &pPlot->haxis1;
            pPlot->series[0].pVerticalAxis = &pPlot->vaxis1;

            // set y data
            pPlot->series[0].y = format(this->m_acc_data.mean());

            // skip if no data
            if (pPlot->series[0].y.size() == 0)
                break;

            // generate axis data
            auto axis = genAxis(pPlot->series[0].y.size());

            // set x data
            pPlot->series[0].x = axis.vec;

            // set axis title
            pPlot->haxis1.title.text = axis.title;
            pPlot->vaxis1.title.text = VAXIS_TITLE;

            // set axis display formula
            pPlot->haxis1.labels.format = axis.format;

            pPlot->vaxis1.labels.format = [](double val)
            {
                char szTmp[64];
                sprintf_s(szTmp, "%.2e", val);

                return std::string(szTmp);
            };

            // automatic span of data
            if (!span_ex(pPlot->haxis1, pPlot->series[0].x, axis.fMin, axis.fMax))
                break;

            // display axis anyway
            pPlot->haxis1.render_enable = true;
            pPlot->vaxis1.render_enable = true;

            if (!span(pPlot->vaxis1, pPlot->series[0].y))
                break;

            // do not display series if no y data
            pPlot->series[0].render_enable = true;

            // return successfuly
            return;

        } while (false);

        // something went wrong, do not display data
        pPlot->series[0].render_enable = false;
    }

    // set serie2
    void setSeries2(std::shared_ptr<guiPlot> pPlot) const
    {
        // always check plot even if it should be valid
        if (pPlot == nullptr)
            return;

        do
        {
            // set axis (mixed)
            pPlot->series[1].pHorizontalAxis = &pPlot->haxis1;
            pPlot->series[1].pVerticalAxis = &pPlot->vaxis2;

            // set line color
            pPlot->series[1].line.color = RGB(255, 128, 128);

            // set y data
            pPlot->series[1].y = this->m_acc_sat.mean();

            // skip if no data
            if (pPlot->series[1].y.size() == 0)
                break;

            // generate axis data
            auto axis = genAxis(pPlot->series[1].y.size());

            // set x data
            pPlot->series[1].x = axis.vec;

            // set axis title
            pPlot->vaxis2.title.text = "Camera Saturation (%)";

            // set axis display formula
            pPlot->vaxis2.labels.format = [](double val)
            {
                char szTmp[64];
                sprintf_s(szTmp, "%.0f%%", 100.0 * val);

                return std::string(szTmp);
            };

            // automatic span of data
            if (!span_ex(pPlot->vaxis2, pPlot->series[1].y, 0, 1))
                break;

            // rendering options
            pPlot->series[1].render_enable = true;

            pPlot->vaxis2.render_enable = true;

            // return successfuly
            return;

        } while (false);

        // something went wrong, do not display data
        pPlot->series[1].render_enable = false;

        pPlot->vaxis2.render_enable = false;
    }

    // set serie3
    void setSeries3(std::shared_ptr<guiPlot> pPlot) const
    {
        // always check plot even if it should be valid
        if (pPlot == nullptr)
            return;

        // set line color
        pPlot->series[2].line.color = RGB(128, 255, 128);

        // set y data
        pPlot->series[2].y = this->m_acc_roi.mean();
        pPlot->series[2].x = linspace(0, (double)(pPlot->series[2].y.size() - 1), pPlot->series[2].y.size());

        // skip if no data
        if (pPlot->series[2].y.size() == 0)
            return;

        // create two temporary axis
        pPlot->axis_ex.clear();
        pPlot->axis_ex.emplace_back(guiAxis(guiAxis::Type::Horizontal, minof(pPlot->series[2].x), maxof(pPlot->series[2].x)));
        pPlot->axis_ex.emplace_back(guiAxis(guiAxis::Type::Vertical, 1, 0));

        // set secondary axis
        pPlot->series[2].pHorizontalAxis = &pPlot->axis_ex[0];
        pPlot->series[2].pVerticalAxis = &pPlot->axis_ex[1];

        // rendering options
        pPlot->series[2].render_enable = true;
    }

    // accumulator used for display
    Accumulator m_acc_data, m_acc_sat, m_acc_roi;

    // UID of camera
    std::string m_uid;
};