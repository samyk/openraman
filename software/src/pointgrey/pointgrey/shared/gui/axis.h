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

 // disable stupid warnings
#pragma warning(disable:26451)

#include <string>
#include <functional>
#include <windows.h>

#include "../utils/utils.h"

#include "gui.h"
#include "brush.h"
#include "pen.h"
#include "text.h"

// defaults parameters when creating an axis
#define DEFAULT_MAJOR_GRID_LINE_THICKNESS		2
#define DEFAULT_MAJOR_GRID_LINE_COLOR			RGB(128,128,128)

#define DEFAULT_MINOR_GRID_LINE_THICKNESS		1
#define DEFAULT_MINOR_GRID_LINE_COLOR			RGB(192,192,192)

#define DEFAULT_MAJOR_TICKS_LINE_THICKNESS		2
#define DEFAULT_MAJOR_TICKS_LINE_EXTEND			7
#define DEFAULT_MAJOR_TICKS_LINE_COLOR			RGB(0,0,0)

#define DEFAULT_MINOR_TICKS_LINE_THICKNESS		1
#define DEFAULT_MINOR_TICKS_LINE_EXTEND			5
#define DEFAULT_MINOR_TICKS_LINE_COLOR			RGB(128,128,128)

// guiAxis class
class guiAxis
{
public:
	// types of axis
	enum class Type
	{
		Horizontal,
		Vertical,
	};

	// default constructor, must specify type of axis
	guiAxis(Type eType, double fMin=0, double fMax=0)
	{
		this->minval = fMin;
		this->maxval = fMax;

		this->m_eType = eType;
	}

	// map value to screen coordinates, return false in case of failure
	bool map(const RECT& rRect, double fValue, int& rRetValue) const
	{
		// skip if min = max
		if (fabs(this->minval - this->maxval) < 1e-15)
			return false;

		// formula depends on axis type (vertical or horizontal)
		switch (this->m_eType)
		{
		case Type::Horizontal:
			rRetValue = rRect.left + (int)round((double)(rRect.right - rRect.left) * (fValue - this->minval) / (this->maxval - this->minval));

			return true;

		case Type::Vertical:
			rRetValue = rRect.top + (int)round((double)(rRect.bottom - rRect.top) * (this->maxval - fValue) / (this->maxval - this->minval));

			return true;

			// unhandled case, return false
		default:
			return false;
		}
	}

	double minval, maxval;

protected:

	// convert an axis step size into screen coordinates, return false in case of failure
	double computeUIStepSize(const RECT& rRect, double fAxisStepSize, double& fRetValue) const
	{
		// skip if min = max
		if (fabs(this->minval - this->maxval) < 1e-15)
			return false;

		// return depend on the axis being either vertical or horizontal
		switch (this->m_eType)
		{
		case Type::Horizontal:
			fRetValue = (double)(rRect.right - rRect.left) * fAxisStepSize / (this->maxval - this->minval);

			// return true if the value is not zero (this would cause infinite loops in some functions)
			return fabs(fRetValue) > 1e-15;

		case Type::Vertical:
			fRetValue = (double)(rRect.bottom - rRect.top) * fAxisStepSize / (this->maxval - this->minval);

			// return true if the value is not zero (this would cause infinite loops in some functions)
			return fabs(fRetValue) > 1e-15;

			// unhandled cases return false
		default:
			return false;
		}
	}

	Type m_eType;
};

// guiIRenderAxis interface class
class guiIRenderAxis : public guiAxis
{
public:

	// special pen type for ticks with how much the ticks extends
	class guiTicksPen : public guiPen
	{
	public:
		guiTicksPen(void)
		{
			this->extend = 0;
		}

		int extend;
	};

	// special text type for labels with a format function to convert double to strings
	class guiLabels : public guiText
	{
	public:
		std::function<std::string(double)> format;
	};

	// default constructor, must specify type of axis
	guiIRenderAxis(Type eType) : guiAxis(eType)
	{
		this->major = 0;
		this->minor = 0;

		this->grid.render_enable = true;

		this->grid.major.thickness = DEFAULT_MAJOR_GRID_LINE_THICKNESS;
		this->grid.major.color = DEFAULT_MAJOR_GRID_LINE_COLOR;

		this->grid.minor.thickness = DEFAULT_MINOR_GRID_LINE_THICKNESS;
		this->grid.minor.color = DEFAULT_MINOR_GRID_LINE_COLOR;

		this->ticks.major.thickness = DEFAULT_MAJOR_TICKS_LINE_THICKNESS;
		this->ticks.major.extend = DEFAULT_MAJOR_TICKS_LINE_EXTEND;
		this->ticks.major.color = DEFAULT_MAJOR_TICKS_LINE_COLOR;

		this->ticks.minor.thickness = DEFAULT_MINOR_TICKS_LINE_THICKNESS;
		this->ticks.minor.extend = DEFAULT_MINOR_TICKS_LINE_EXTEND;
		this->ticks.minor.color = DEFAULT_MINOR_TICKS_LINE_COLOR;

		// do not render by default
		this->render_enable = false;
	}

	// render major grid lines
	void renderMajorGrid(HDC hDC, const RECT& rRect) const
	{
		// skip if render is disabled
		if (!this->render_enable || !this->grid.render_enable)
			return;

		// skip if no draw
		if (this->grid.major.type.get() == guiPen::Type::None)
			return;

		// set pen
		AUTOPEN(hDC, this->grid.major);

		// compute grid step size in screen coordinates and render
		double fUIStepSize;

		if (computeUIStepSize(rRect, this->major, fUIStepSize))
			renderGrid(hDC, rRect, fUIStepSize);
	}

	// render minor grid lines
	void renderMinorGrid(HDC hDC, const RECT& rRect) const
	{
		// skip if render is disabled
		if (!this->render_enable || !this->grid.render_enable)
			return;

		// skip if line thickness is zero
		if (this->grid.minor.type.get() == guiPen::Type::None)
			return;

		// set pen
		AUTOPEN(hDC, this->grid.minor);

		// compute grid step size in screen coordinates and render
		double fUIStepSize;

		if (computeUIStepSize(rRect, this->minor, fUIStepSize))
			renderGrid(hDC, rRect, fUIStepSize);
	}

	// render major ticks line
	void renderMajorTicks(HDC hDC, const RECT& rRect) const
	{
		// skip if render is disabled
		if (!this->render_enable)
			return;

		// skip if line thickness is zero
		if (this->ticks.major.type.get() == guiPen::Type::None)
			return;

		// set pen
		AUTOPEN(hDC, this->ticks.major);

		// compute grid step size in screen coordinates and render
		double fUIStepSize;

		if (computeUIStepSize(rRect, this->major, fUIStepSize))
			renderTicks(hDC, rRect, fUIStepSize, this->ticks.major.extend);
	}

	// render minor ticks line
	void renderMinorTicks(HDC hDC, const RECT& rRect) const
	{
		// skip if render is disabled
		if (!this->render_enable)
			return;

		// skip if line thickness is zero
		if (this->ticks.minor.type.get() == guiPen::Type::None)
			return;

		// set pen
		AUTOPEN(hDC, this->ticks.minor);

		// compute grid step size in screen coordinates and render
		double fUIStepSize;

		if (computeUIStepSize(rRect, this->minor, fUIStepSize))
			renderTicks(hDC, rRect, fUIStepSize, this->ticks.minor.extend);
	}

	// compute either the height (horizontal axis) or width (vertical axis) taken by the labels
	int calcLabelsMargin(HDC hDC) const
	{
		// set font
		AUTOFONT(hDC, this->labels.font);

		// skip if no formating function has been set
		if (!this->labels.format)
			return 0;

		// real minimum and maximum of axis (minval could be greater than maxval!)
		double fMin = min(this->minval, this->maxval);
		double fMax = max(this->minval, this->maxval);

		// store output data in 'results'
		SIZE results = { 0, 0 };

		// scan range from min to max and step by major increments
		for (double i = fMin; i <= fMax; i += fabs(this->major))
		{
			// convert current value to string
			this->labels.text = this->labels.format(i);

			// compute rect for the text using the current font
			int  w, h;

			if (!this->labels.calcRect(hDC, w, h))
				continue;

			// store maximum in 'results'
			results.cx = max(results.cx, w);
			results.cy = max(results.cy, h);
		}

		// return either width or height depending on the axis type
		switch (this->m_eType)
		{
		case Type::Horizontal:
			return results.cy;

		case Type::Vertical:
			return results.cx;
		}

		// unhandled cases return 0
		return 0;
	}

	// render labels
	virtual void renderLabels(HDC hDC, const RECT& rRect) const = 0;

	// parameters that can be changed by user
	struct
	{
		guiPen major, minor;

		bool render_enable;
	} grid;

	struct
	{
		guiTicksPen major, minor;
	} ticks;

	mutable guiLabels labels;
	guiText title;

	double major, minor;
	bool render_enable;

protected:

	// render grid lines using the currently set pen
	virtual void renderGrid(HDC hDC, const RECT& rRect, double fUIStepSize) const = 0;

	// render tick lines using the currently set pen
	virtual void renderTicks(HDC hDC, const RECT& rRect, double fUIStepSize, int iTickSize) const = 0;
};

// guiPrimaryHorizontalAxis class
class guiPrimaryHorizontalAxis : public guiIRenderAxis
{
public:

	// default constructor, must specify type of axis
	guiPrimaryHorizontalAxis(void) : guiIRenderAxis(Type::Horizontal) { }

	// render labels
	virtual void renderLabels(HDC hDC, const RECT& rRect) const override
	{
		// skip if no formating function
		if (!this->labels.format)
			return;

		// set font
		AUTOFONT(hDC, this->labels.font);

		// compute major step size
		double fUIStepSize;

		if (!computeUIStepSize(rRect, this->major, fUIStepSize))
			return;

		// scan axis by major ticks
		for (double x = (double)rRect.left, x2 = this->minval; x < (double)(rRect.right+1); x += fabs(fUIStepSize), x2 += this->major * sign(this->maxval - this->minval))
		{
			// convert current value to string
			this->labels.text = this->labels.format(x2);

			// compute current label dimensions
			int  w, h;

			if (!this->labels.calcRect(hDC, w, h))
				continue;

			// create rect and display
			RECT rect;

			rect.left = (int)x - w / 2;
			rect.right = rect.left + w;

			rect.top = rRect.top;
			rect.bottom = rRect.bottom;

			this->labels.render(hDC, rect);
		}
	}

protected:

	// render grid lines using the currently set pen
	virtual void renderGrid(HDC hDC, const RECT &rRect, double fUIStepSize) const override
	{
		for (double x = (double)rRect.left; x < (double)(rRect.right+1); x += fabs(fUIStepSize))
			drawLine(hDC, (int)x, rRect.top, (int)x, rRect.bottom);
	}

	// render tick lines using the currently set pen
	virtual void renderTicks(HDC hDC, const RECT &rRect, double fUIStepSize, int iTickSize) const override
	{
		for (double x = (double)rRect.left; x < (double)(rRect.right+1); x += fabs(fUIStepSize))
			drawLine(hDC, (int)x, rRect.bottom, (int)x, rRect.bottom + iTickSize);
	}
};

// guiPrimaryVerticalAxis class
class guiPrimaryVerticalAxis : public guiIRenderAxis
{
public:

	// default constructor, must specify type of axis
	guiPrimaryVerticalAxis(void) : guiIRenderAxis(Type::Vertical) { }

	// render labels
	virtual void renderLabels(HDC hDC, const RECT& rRect) const override
	{
		// skip if no formating function
		if (!this->labels.format)
			return;

		// set font
		AUTOFONT(hDC, this->labels.font);

		// compute major step size
		double fUIStepSize;

		if (!computeUIStepSize(rRect, this->major, fUIStepSize))
			return;

		// scan axis by major ticks
		for (double y = (double)rRect.bottom, y2 = this->minval; y > (double)(rRect.top + 1); y -= fabs(fUIStepSize), y2 += this->major * sign(this->maxval - this->minval))
		{
			// convert current value to string
			this->labels.text = this->labels.format(y2);

			// compute current label dimensions
			int  w, h;

			if (!this->labels.calcRect(hDC, w, h))
				continue;

			// create rect and display
			RECT rect;

			rect.left = rRect.left;
			rect.right = rRect.right;

			rect.top = (int)y - h / 2;
			rect.bottom = rect.top + h;

			this->labels.render(hDC, rect);
		}
	}

protected:

	// render grid lines using the currently set pen
	virtual void renderGrid(HDC hDC, const RECT& rRect, double fUIStepSize) const override
	{
		for (double y = (double)rRect.bottom; y > (double)(rRect.top + 1); y -= fabs(fUIStepSize))
			drawLine(hDC, rRect.left, (int)y, rRect.right, (int)y);
	}

	// render tick lines using the currently set pen
	virtual void renderTicks(HDC hDC, const RECT& rRect, double fUIStepSize, int iTickSize) const override
	{
		for (double y = (double)rRect.bottom; y >= (double)(rRect.top + 1); y -= fabs(fUIStepSize))
			drawLine(hDC, rRect.left - iTickSize, (int)y, rRect.left, (int)y);
	}
};

// guiSecondaryHorizontalAxis class
class guiSecondaryHorizontalAxis : public guiIRenderAxis
{
public:

	// default constructor, must specify type of axis
	guiSecondaryHorizontalAxis(void) : guiIRenderAxis(Type::Horizontal) { }

	// render labels
	virtual void renderLabels(HDC hDC, const RECT& rRect) const override
	{
		// skip if no formating function
		if (!this->labels.format)
			return;

		// set font
		AUTOFONT(hDC, this->labels.font);

		// compute major step size
		double fUIStepSize;

		if (!computeUIStepSize(rRect, this->major, fUIStepSize))
			return;

		// scan axis by major ticks
		for (double x = (double)rRect.left, x2 = this->minval; x < (double)(rRect.right + 1); x += fabs(fUIStepSize), x2 += this->major * sign(this->maxval - this->minval))
		{
			// convert current value to string
			this->labels.text = this->labels.format(x2);

			// compute current label dimensions
			int  w, h;

			if (!this->labels.calcRect(hDC, w, h))
				continue;

			// create rect and display
			RECT rect;

			rect.left = (int)x - w / 2;
			rect.right = rect.left + w;

			rect.top = rRect.top;
			rect.bottom = rRect.bottom;

			this->labels.render(hDC, rect);
		}
	}

protected:

	// render grid lines using the currently set pen
	virtual void renderGrid(HDC hDC, const RECT& rRect, double fUIStepSize) const override
	{
		for (double x = (double)rRect.left; x < (double)(rRect.right + 1); x += fabs(fUIStepSize))
			drawLine(hDC, (int)x, rRect.top, (int)x, rRect.bottom);
	}

	// render tick lines using the currently set pen
	virtual void renderTicks(HDC hDC, const RECT& rRect, double fUIStepSize, int iTickSize) const override
	{
		for (double x = (double)rRect.left; x <= (double)(rRect.right + 1); x += fabs(fUIStepSize))
			drawLine(hDC, (int)x, rRect.top, (int)x, rRect.top - iTickSize);
	}
};

// guiSecondaryVerticalAxis class
class guiSecondaryVerticalAxis : public guiIRenderAxis
{
public:

	// default constructor, must specify type of axis
	guiSecondaryVerticalAxis(void) : guiIRenderAxis(Type::Vertical) { }

	// render labels
	virtual void renderLabels(HDC hDC, const RECT& rRect) const override
	{
		// skip if no formating function
		if (!this->labels.format)
			return;

		// set font
		AUTOFONT(hDC, this->labels.font);

		// compute major step size
		double fUIStepSize;

		if (!computeUIStepSize(rRect, this->major, fUIStepSize))
			return;

		// scan axis by major ticks
		for (double y = (double)rRect.bottom, y2 = this->minval; y > (double)(rRect.top + 1); y -= fabs(fUIStepSize), y2 += this->major * sign(this->maxval - this->minval))
		{
			// convert current value to string
			this->labels.text = this->labels.format(y2);

			// compute current label dimensions
			int  w, h;

			if (!this->labels.calcRect(hDC, w, h))
				continue;

			// create rect and display
			RECT rect;

			rect.left = rRect.left;
			rect.right = rRect.right;

			rect.top = (int)y - h / 2;
			rect.bottom = rect.top + h;

			this->labels.render(hDC, rect);
		}
	}

protected:

	// render grid lines using the currently set pen
	virtual void renderGrid(HDC hDC, const RECT& rRect, double fUIStepSize) const override
	{
		for (double y = (double)rRect.bottom; y > (double)(rRect.top + 1); y -= fabs(fUIStepSize))
			drawLine(hDC, rRect.left, (int)y, rRect.right, (int)y);
	}

	// render tick lines using the currently set pen
	virtual void renderTicks(HDC hDC, const RECT& rRect, double fUIStepSize, int iTickSize) const override
	{
		for (double y = (double)rRect.bottom; y >= (double)(rRect.top + 1); y -= fabs(fUIStepSize))
			drawLine(hDC, rRect.right, (int)y, rRect.right + iTickSize, (int)y);
	}
};