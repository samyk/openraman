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

// CsvDataBuilder class
class CsvDataBuilder : public IPlotBuilder
{
public:
    CsvDataBuilder(const char* pszFilename)
    {
        // open file
        FILE* pFile = nullptr;

        fopen_s(&pFile, pszFilename, "rb");

        if (pFile == nullptr)
            throw FileNotFoundException(pszFilename);

        // lambda to tokenize a line from the file
        auto getline = [](FILE* pFile)
        {
            // read file contents
            char szBuffer[1024];

            fgets(szBuffer, sizeof(szBuffer), pFile);

            // remove \r and \n
            if (strlen(szBuffer) > 0 && szBuffer[strlen(szBuffer) == '\n'])
                szBuffer[strlen(szBuffer)] = '\0';

            if (strlen(szBuffer) > 0 && szBuffer[strlen(szBuffer) == '\r'])
                szBuffer[strlen(szBuffer)] = '\0';

            // return tokenized string
            return tokenize(szBuffer, ',');
        };

        // skip if no data
        if (feof(pFile))
        {
            fclose(pFile);

            throw EmptyFileException(pszFilename);
        }

        // get first line (header)
        auto headers = getline(pFile);

        this->m_x.header = headers[0];
        this->m_y.header = headers[1];

        // easiest case is two-columns file
        if (headers.size() != 2)
        {
            fclose(pFile);

            throw WrongFileTypeException(pszFilename);
        }

        // read all lines
        while (!feof(pFile))
        {
            // get line
            auto data = getline(pFile);

            // skip if not 2 columns
            if (data.size() != 2)
                continue;

            // add to vectors
            if (*data[0].c_str() != '\0')
                this->m_x.data.emplace_back(atof(data[0].c_str()));

            if (*data[1].c_str() != '\0')
                this->m_y.data.emplace_back(atof(data[1].c_str()));
        }

        // close file
        fclose(pFile);
    }

    // create spectre file
    virtual SpectreFile createSpectreFile(void) const override
    {
        return SpectreFile(this->m_y.data, vector_t(0), "");
    }

    // do not enable saturation button
    virtual bool hasSaturationOpt(void) const override
    {
        return false;
    }

    // do not enable axis change
    virtual bool hasAxisChangeOpt(void) const override
    {
        return false;
    }

    // do not enable blank option
    virtual bool hasBlankOpt(void) const override
    {
        return false;
    }

    // get blank
    virtual vector_t getBlankData(void) const override
    {
        return vector_t(0);
    }

    // build plot
    virtual void buildPlot(std::shared_ptr<guiPlot> pPlot) const override
    {
        // skip if no plot
        if (pPlot == nullptr)
            return;

        // resize data to 1
        pPlot->series.clear();
        pPlot->series.resize(1);

        do
        {
            // set primary axis
            pPlot->series[0].pHorizontalAxis = &pPlot->haxis1;
            pPlot->series[0].pVerticalAxis = &pPlot->vaxis1;

            // set data
            pPlot->series[0].x = this->m_x.data;
            pPlot->series[0].y = format(this->m_y.data);

            // set axis title
            pPlot->haxis1.title.text = this->m_x.header;
            pPlot->vaxis1.title.text = this->m_y.header;

            // set axis display formula
            pPlot->haxis1.labels.format = [](double val)
            {
                char szTmp[64];
                sprintf_s(szTmp, "%.2e", val);

                return std::string(szTmp);
            };

            pPlot->vaxis1.labels.format = [](double val)
            {
                char szTmp[64];
                sprintf_s(szTmp, "%.2e", val);

                return std::string(szTmp);
            };

            // automatic span of data
            if (!span(pPlot->haxis1, pPlot->series[0].x))
                break;

            if (!span(pPlot->vaxis1, pPlot->series[0].y))
                break;

            pPlot->haxis1.grid.render_enable = true;
            pPlot->vaxis1.grid.render_enable = true;

            // rendering options
            pPlot->series[0].render_enable = true;

            pPlot->haxis1.render_enable = true;
            pPlot->haxis2.render_enable = false;

            pPlot->vaxis1.render_enable = true;
            pPlot->vaxis2.render_enable = false;

            // return successfuly
            return;

        } while (false);

        // something went wrong, do not display data
        pPlot->series.clear();

        pPlot->haxis1.render_enable = false;
        pPlot->haxis2.render_enable = false;

        pPlot->vaxis1.render_enable = false;
        pPlot->vaxis2.render_enable = false;
    }

private:
    struct
    {
        std::string header;
        vector_t data;
    } m_x, m_y;
};