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
#include "shared/gui/plot.h"

#include "data.h"
#include "winmain.h"
#include "exception.h"

// FileDataDisplay class
class FileDataBuilder : public IPlotBuilder
{
public:

    // construct from data
    FileDataBuilder(const vector_t& rData, const vector_t& rBlank, const std::string& rUID)
    {
        this->m_data = rData;
        this->m_blank = rBlank;
        this->m_uid = rUID;
    }

    // create spectre file
    virtual SpectreFile createSpectreFile(void) const override
    {
        return SpectreFile(this->m_data, this->m_blank, this->m_uid);
    }

    // enable saturation button
    virtual bool hasSaturationOpt(void) const override
    {
        return false;
    }

    // enable axis change
    virtual bool hasAxisChangeOpt(void) const override
    {
        return true;
    }

    // do not enable blank option
    virtual bool hasBlankOpt(void) const override
    {
        return false;
    }

    // return blank data
    virtual vector_t getBlankData(void) const override
    {
        return this->m_blank;
    }

    // update plot
    virtual void buildPlot(std::shared_ptr<guiPlot> pPlot) const override
    {
        // skip if no plot
        if (pPlot == nullptr)
            return;

        // resize data to 1
        pPlot->series.clear();
        pPlot->series.resize(1);

        // hide all axis by default
        pPlot->haxis1.render_enable = false;
        pPlot->haxis2.render_enable = false;

        pPlot->vaxis1.render_enable = false;
        pPlot->vaxis2.render_enable = false;

        pPlot->series[0].render_enable = false;

        do
        {
            // set primary axis
            pPlot->series[0].pHorizontalAxis = &pPlot->haxis1;
            pPlot->series[0].pVerticalAxis = &pPlot->vaxis1;

            // set y data
            pPlot->series[0].y = format(this->m_data);

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

private:
    vector_t m_data, m_blank;
    std::string m_uid;
};