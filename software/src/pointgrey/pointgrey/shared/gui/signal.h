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

#include <memory>
#include <vector>
#include <Windows.h>

#include "../utils/utils.h"
#include "../math/vector.h"

#include "pen.h"
#include "axis.h"
#include "marker.h"
#include "text.h"

// default rendering style for guiSignal
#define SIGNAL_PRECISION_AUTO			0

#define DEFAULT_SIGNAL_PRECISION		SIGNAL_PRECISION_AUTO
#define DEFAULT_SIGNAL_LINE_THICKNESS	2
#define DEFAULT_SIGNAL_LINE_COLOR		RGB(0,0,0)

// guiSignal class
class guiSignal
{
public:
	class DataLabel : public guiText
	{
	public:
		DataLabel(void)
		{
			this->offset_x = 0;
			this->offset_y = 0;
		}

		int offset_x, offset_y;
	};

	// default constructor
	guiSignal(void)
	{
		this->pHorizontalAxis = nullptr;
		this->pVerticalAxis = nullptr;

		this->precision = DEFAULT_SIGNAL_PRECISION;

		this->line.thickness = DEFAULT_SIGNAL_LINE_THICKNESS;
		this->line.color = DEFAULT_SIGNAL_LINE_COLOR;

		this->render_enable = true;
	}

	// render
	void render(HDC hDC, const RECT& rRect) const
	{
		// skip if rendering is disabled
		if (!this->render_enable)
			return;

		// skip if no axis
		if (this->pHorizontalAxis == nullptr || this->pVerticalAxis == nullptr)
			return;

		// total number of points
		const size_t n = min(this->x.size(), this->y.size());

		if (n == 0)
			return;

		// default precision to linewidth
		int iPrecision = this->precision;

		if (iPrecision <= 0)
			iPrecision = (int)this->line.thickness.get();

		// create smoothed graph
		std::vector<struct acc_s> acc_data;

		struct acc_s curr_acc = { 0, 0, 0 };

		int pivot_x;

		for (size_t i = 0; i < n; i++)
		{
			// map current point
			int curr_x, curr_y;

			if (!this->pHorizontalAxis->map(rRect, this->x[i], curr_x))
				continue;

			if (!this->pVerticalAxis->map(rRect, this->y[i], curr_y))
				continue;

			// special case for first point
			if (i == 0)
			{
				curr_acc.x = (double)curr_x;
				curr_acc.y = (double)curr_y;

				curr_acc.n = 1;

				pivot_x = curr_x;

				continue;
			}

			// check distance to previous point and commit accumulator if necessary
			if (abs(curr_x - pivot_x) > iPrecision)
			{
				if (curr_acc.n != 0)
					acc_data.emplace_back(curr_acc);

				curr_acc.x = (double)curr_x;
				curr_acc.y = (double)curr_y;

				curr_acc.n = 1;

				pivot_x = curr_x;
			}
			// otherelse update acc
			else
			{
				curr_acc.x += (double)curr_x;
				curr_acc.y += (double)curr_y;

				curr_acc.n++;
			}
		}

		if (curr_acc.n != 0)
			acc_data.emplace_back(curr_acc);

		// render smoothed graph
		POINT* pPoints = nullptr;

		try
		{
			size_t nAccSize = acc_data.size();

			pPoints = new POINT[nAccSize];

			for (size_t i = 0; i < nAccSize; i++)
			{
				pPoints[i].x = (LONG)round(acc_data[i].x / acc_data[i].n);
				pPoints[i].y = (LONG)round(acc_data[i].y / acc_data[i].n);
			}

			// render line
			if(this->line.type.get() != guiPen::Type::None)
			{
				AUTOPEN(hDC, this->line);

				Polyline(hDC, pPoints, (int)acc_data.size());
			}

			// render markers
			if (this->markers.type != guiMarker::Type::None)
			{
				size_t nAccSize = acc_data.size();

				for (size_t i = 0; i < nAccSize; i++)
					this->markers.render(hDC, pPoints[i].x, pPoints[i].y);
			}
		}
		catch (...)
		{

		}

		// free points
		if(pPoints != nullptr)
			delete[] pPoints;

		pPoints = nullptr;

		// render data labels if existing
		if (this->labels.size() > 0 && this->labels.size() >= n)
		{
			// browse all points
			for (size_t i = 0; i < n; i++)
			{
				// map current point
				int curr_x, curr_y;

				if (!this->pHorizontalAxis->map(rRect, this->x[i], curr_x))
					continue;

				if (!this->pVerticalAxis->map(rRect, this->y[i], curr_y))
					continue;

				// compute label width & height
				int w = 0, h = 0;

				if (!this->labels[i].calcRect(hDC, w, h))
					continue;

				// display text
				RECT rect;

				rect.left = this->labels[i].offset_x + curr_x - w / 2;
				rect.right = rect.left + w;
				rect.top = this->labels[i].offset_y + curr_y - h / 2;
				rect.bottom = rect.top + h;

				this->labels[i].render(hDC, rect);
			}
		}
	}

	// data that can be changed by the user
	bool render_enable;

	guiAxis *pHorizontalAxis;
	guiAxis *pVerticalAxis;

	guiPen line;
	guiMarker markers;

	int precision;

	vector_t x, y;

	std::vector<DataLabel> labels;

private:
	struct acc_s
	{
		double x, y, n;
	};
};