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

#include <vector>
#include <Windows.h>

#include "brush.h"
#include "pen.h"
#include "font.h"
#include "signal.h"
#include "axis.h"
#include "text.h"
#include "signal.h"

// default rendering for plot
#define	DEFAULT_PLOT_BORDER_THICKNESS		2
#define DEFAULT_PLOT_BORDER_COLOR			RGB(0,0,0)
#define DEFAULT_PLOT_BACKGROUND_COLOR		RGB(255,255,255)
#define DEFAULT_WINDOW_BACKGROUND_COLOR		RGB(192,192,192)

#define DEFAULT_MARGIN						5

// guiPlot class
class guiPlot
{
public:

	// default constructor
	guiPlot(void)
	{
		// set default plot styles
		this->window.background.color = DEFAULT_WINDOW_BACKGROUND_COLOR;
		this->plot.background.color = DEFAULT_PLOT_BACKGROUND_COLOR;
		this->plot.border.color = DEFAULT_PLOT_BORDER_COLOR;
		this->plot.border.thickness = DEFAULT_PLOT_BORDER_THICKNESS;

		// set margin
		this->margin.left = DEFAULT_MARGIN;
		this->margin.right = DEFAULT_MARGIN;
		this->margin.top = DEFAULT_MARGIN;
		this->margin.bottom = DEFAULT_MARGIN;

		// no data display
		this->nodata.text.text = "WAITING FOR DATA";
		this->nodata.text.color = RGB(128, 128, 128);
		this->nodata.text.font.bold = true;
		this->nodata.text.font.family = "Calibri";
		this->nodata.text.font.size = 64;

		this->nodata.lines.thickness = 1;
		this->nodata.lines.color = RGB(128, 128, 128);
		this->nodata.lines.type = guiPen::Type::Dot;
		
		this->nodata.brush.color = RGB(128, 128, 128);
		this->nodata.brush.type = guiBrush::Type::HatchDiagonal;
	}

	// return true if plot has data to show
	bool hasData(void) const
	{
		// return false if no series
		if (this->series.size() == 0)
			return false;

		// return true if at least one series has render_enable
		for (auto& v : this->series)
			if (v.render_enable)
				return true;

		// otherelse return false
		return false;
	}

	// render plot
	void render(HDC hDC, const RECT &rClientRect) const
	{
		// compute dimensions
		RECT sFrameRect, sWindowRect;

		sWindowRect.left = rClientRect.left + this->margin.left;
		sWindowRect.right = rClientRect.right - this->margin.right;
		sWindowRect.top = rClientRect.top + this->margin.top;
		sWindowRect.bottom = rClientRect.bottom - this->margin.bottom;

		sFrameRect = sWindowRect;

		// set background mode and color
		SetBkMode(hDC, TRANSPARENT);
		SetBkColor(hDC, RGB(255, 255, 255));

		// fill window background
		this->window.background.fillRect(hDC, rClientRect);

		// has data to display
		auto bHasData = hasData();

		// display title
		if (bHasData && this->title.text.length() > 0)
		{
			RECT title_rect;

			int w = 0, h = 0;
			this->title.calcRect(hDC, w, h);

			title_rect.left = sFrameRect.left;
			title_rect.right = sFrameRect.right;
			title_rect.top = sFrameRect.top;
			title_rect.bottom = sFrameRect.top + h;

			sFrameRect.top += h;

			this->title.render(hDC, title_rect);
		}

		// compute axis titles rects
		int tmp;

		int haxis1_title_ofs = 0;
		int haxis2_title_ofs = 0;
		int vaxis1_title_ofs = 0;
		int vaxis2_title_ofs = 0;

		if (bHasData && this->haxis1.render_enable && this->haxis1.title.text.length() > 0)
			this->haxis1.title.calcRect(hDC, tmp, haxis1_title_ofs);

		if (bHasData && this->haxis2.render_enable && this->haxis2.title.text.length() > 0)
			this->haxis2.title.calcRect(hDC, tmp, haxis2_title_ofs);

		if (bHasData && this->vaxis1.render_enable && this->vaxis1.title.text.length() > 0)
			this->vaxis1.title.calcRect(hDC, vaxis1_title_ofs, tmp);

		if (bHasData && this->vaxis2.render_enable && this->vaxis2.title.text.length() > 0)
			this->vaxis2.title.calcRect(hDC, vaxis2_title_ofs, tmp);

		// shrink frame rect
		RECT sTitleFrameRect = sFrameRect;

		sFrameRect.left += vaxis1_title_ofs;
		sFrameRect.right -= vaxis2_title_ofs;
		sFrameRect.top += haxis2_title_ofs;
		sFrameRect.bottom -= haxis1_title_ofs;

		// compute final frame rect size from axis label size
		RECT sFrameAxisRect = sFrameRect;

		if (bHasData && this->haxis1.render_enable)
		{
			RECT labels_rect;

			int tmp = this->haxis1.calcLabelsMargin(hDC);

			labels_rect.left = sFrameRect.left;
			labels_rect.right = sFrameRect.right;
			labels_rect.top = sFrameRect.bottom - tmp;
			labels_rect.bottom = sFrameRect.bottom;

			sFrameRect.bottom -= tmp;
		}

		if (bHasData && this->haxis2.render_enable)
		{
			RECT labels_rect;

			int tmp = this->haxis2.calcLabelsMargin(hDC);

			labels_rect.left = sFrameRect.left;
			labels_rect.right = sFrameRect.right;
			labels_rect.top = sFrameRect.top;
			labels_rect.bottom = sFrameRect.top + tmp;

			sFrameRect.bottom += tmp;
		}

		if (bHasData && this->vaxis1.render_enable)
		{
			RECT labels_rect;

			int tmp = this->vaxis1.calcLabelsMargin(hDC);

			labels_rect.left = sFrameRect.left;
			labels_rect.right = sFrameRect.left + tmp;
			labels_rect.top = sFrameRect.top;
			labels_rect.bottom = sFrameRect.bottom;

			sFrameRect.left += tmp;
		}

		if (bHasData && this->vaxis2.render_enable)
		{
			RECT labels_rect;

			int tmp = this->vaxis2.calcLabelsMargin(hDC);

			labels_rect.left = sFrameRect.right - tmp;
			labels_rect.right = sFrameRect.right;
			labels_rect.top = sFrameRect.top;
			labels_rect.bottom = sFrameRect.bottom;

			sFrameRect.right -= tmp;
		}

		// add offset due to axis ticks
		RECT sFrameRectWithTicks = sFrameRect;

		if (bHasData && this->vaxis1.render_enable)
			sFrameRect.left += max(this->vaxis1.ticks.major.extend, this->vaxis1.ticks.minor.extend);

		if (bHasData && this->vaxis2.render_enable)
			sFrameRect.right -= max(this->vaxis2.ticks.major.extend, this->vaxis2.ticks.minor.extend);

		if (bHasData && this->haxis1.render_enable)
			sFrameRect.bottom -= max(this->haxis1.ticks.major.extend, this->haxis1.ticks.minor.extend);

		if (bHasData && this->haxis2.render_enable)
			sFrameRect.top += max(this->haxis2.ticks.major.extend, this->haxis2.ticks.minor.extend);

		// display axis titles
		if (bHasData && this->haxis1.render_enable && this->haxis1.title.text.length() > 0)
		{
			RECT title_rect;

			title_rect.left = sFrameRect.left;
			title_rect.right = sFrameRect.right;
			title_rect.top = sTitleFrameRect.bottom - haxis1_title_ofs;
			title_rect.bottom = sTitleFrameRect.bottom;

			this->haxis1.title.render(hDC, title_rect);
		}

		if (bHasData && this->haxis2.render_enable && this->haxis2.title.text.length() > 0)
		{
			RECT title_rect;

			title_rect.left = sFrameRect.left;
			title_rect.right = sFrameRect.right;
			title_rect.top = sTitleFrameRect.top;
			title_rect.bottom = sTitleFrameRect.top + haxis2_title_ofs;

			this->haxis2.title.render(hDC, title_rect);
		}

		if (bHasData && this->vaxis1.render_enable && this->vaxis1.title.text.length() > 0)
		{
			RECT title_rect;

			title_rect.left = sTitleFrameRect.left;
			title_rect.right = sTitleFrameRect.left + vaxis1_title_ofs;
			title_rect.top = sFrameRect.top;
			title_rect.bottom = sFrameRect.bottom;

			this->vaxis1.title.render(hDC, title_rect);
		}

		if (bHasData && this->vaxis2.render_enable && this->vaxis2.title.text.length() > 0)
		{
			RECT title_rect;

			title_rect.left = sTitleFrameRect.right - vaxis2_title_ofs;
			title_rect.right = sTitleFrameRect.right;
			title_rect.top = sFrameRect.top;
			title_rect.bottom = sFrameRect.bottom;

			this->vaxis2.title.render(hDC, title_rect);
		}

		// display labels on axis
		if (bHasData && this->haxis1.render_enable)
		{
			RECT labels_rect;

			labels_rect.left = sFrameRect.left;
			labels_rect.right = sFrameRect.right;
			labels_rect.top = sFrameRectWithTicks.bottom;
			labels_rect.bottom = sFrameAxisRect.bottom;

			this->haxis1.renderLabels(hDC, labels_rect);
		}

		if (bHasData && this->haxis2.render_enable)
		{
			RECT labels_rect;

			labels_rect.left = sFrameRect.left;
			labels_rect.right = sFrameRect.right;
			labels_rect.top = sFrameAxisRect.top;
			labels_rect.bottom = sFrameRectWithTicks.top;

			this->haxis2.renderLabels(hDC, labels_rect);
		}

		if (bHasData && this->vaxis1.render_enable)
		{
			RECT labels_rect;

			labels_rect.left = sFrameAxisRect.left;
			labels_rect.right = sFrameRectWithTicks.left;
			labels_rect.top = sFrameRect.top;
			labels_rect.bottom = sFrameRect.bottom;

			this->vaxis1.renderLabels(hDC, labels_rect);
		}

		if (bHasData && this->vaxis2.render_enable)
		{
			RECT labels_rect;

			labels_rect.left = sFrameRectWithTicks.right;
			labels_rect.right = sFrameAxisRect.right;
			labels_rect.top = sFrameRect.top;
			labels_rect.bottom = sFrameRect.bottom;

			this->vaxis2.renderLabels(hDC, labels_rect);
		}

		// fill render zone background
		this->plot.background.fillRect(hDC, sFrameRect);

		if (bHasData)
		{
			// render minor grids
			this->haxis1.renderMinorGrid(hDC, sFrameRect);
			this->haxis2.renderMinorGrid(hDC, sFrameRect);
			this->vaxis1.renderMinorGrid(hDC, sFrameRect);
			this->vaxis2.renderMinorGrid(hDC, sFrameRect);

			// render major grids
			this->haxis1.renderMajorGrid(hDC, sFrameRect);
			this->haxis2.renderMajorGrid(hDC, sFrameRect);
			this->vaxis1.renderMajorGrid(hDC, sFrameRect);
			this->vaxis2.renderMajorGrid(hDC, sFrameRect);

			// render minor ticks
			this->haxis1.renderMinorTicks(hDC, sFrameRect);
			this->haxis2.renderMinorTicks(hDC, sFrameRect);
			this->vaxis1.renderMinorTicks(hDC, sFrameRect);
			this->vaxis2.renderMinorTicks(hDC, sFrameRect);

			// render major ticks
			this->haxis1.renderMajorTicks(hDC, sFrameRect);
			this->haxis2.renderMajorTicks(hDC, sFrameRect);
			this->vaxis1.renderMajorTicks(hDC, sFrameRect);
			this->vaxis2.renderMajorTicks(hDC, sFrameRect);

			// render series
			auto hClipRect = CreateRectRgn(sFrameRect.left, sFrameRect.top, sFrameRect.right, sFrameRect.bottom);
			SelectClipRgn(hDC, hClipRect);

			for(auto it=this->series.begin();it!=this->series.end();it++)
				it->render(hDC, sFrameRect);

			SelectClipRgn(hDC, NULL);
			DeleteObject(hClipRect);
		}
		// display no data stuff
		else
		{
			// render background brush
			this->nodata.brush.fillRect(hDC, sFrameRect);

			// render diagonals
			{
				AUTOPEN(hDC, this->nodata.lines);

				drawLine(hDC, sFrameRect.left, sFrameRect.top, sFrameRect.right, sFrameRect.bottom);
				drawLine(hDC, sFrameRect.left, sFrameRect.bottom, sFrameRect.right, sFrameRect.top);
			}

			// render text
			this->nodata.text.render(hDC, sFrameRect);
		}

		// draw render zone border
		if (this->plot.border.type.get() != guiPen::Type::None)
		{
			AUTOPEN(hDC, this->plot.border);

			drawRect(hDC, sFrameRect.left, sFrameRect.top, sFrameRect.right - sFrameRect.left, sFrameRect.bottom - sFrameRect.top);
		}
	}

	// data that can be changed by the user
	struct
	{
		guiBrush background;
	} window;

	struct
	{
		guiBrush background;
		guiPen border;
	} plot;

	struct
	{
		guiBrush brush;
		guiPen lines;
		guiText text;
	} nodata;

	std::vector<guiAxis> axis_ex;

	guiPrimaryHorizontalAxis haxis1;
	guiPrimaryVerticalAxis vaxis1;

	guiSecondaryHorizontalAxis haxis2;
	guiSecondaryVerticalAxis vaxis2;

	guiText title;

	RECT margin;

	std::vector<guiSignal> series;	
};