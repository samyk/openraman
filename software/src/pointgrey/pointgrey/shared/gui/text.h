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
#include <Windows.h>

#include "font.h"

// default rendering style for text
#define DEFAULT_FONT_FAMILY				"Calibri"
#define DEFAULT_FONT_SIZE				20
#define DEFAULT_FONT_COLOR				RGB(0,0,0)
#define DEFAULT_FONT_BOLD				true
#define DEFAULT_FONT_ITALIC				false
#define DEFAULT_FONT_UNDERLINE			false
#define DEFAULT_FONT_STRIKETHROUGH		false
#define DEFAULT_FONT_ANGLE				0.0
#define DEFAULT_FONT_HORIZONTAL_ALIGN	guiText::HorizontalAlign::Center
#define DEFAULT_FONT_VERTICAL_ALIGN		guiText::VerticalAlign::Center
#define DEFAULT_FONT_MARGIN				5
#define DEFAULT_FONT_PADDING			0

// guiText class
class guiText
{
public:

	// types of horizontal alignments
	enum class HorizontalAlign
	{
		Left,
		Center,
		Right,
		Justify,
	};

	// types of vertical alignments
	enum class VerticalAlign
	{
		Top,
		Center,
		Bottom,
	};

	// default constructor
	guiText(void)
	{
		this->font.family = DEFAULT_FONT_FAMILY;
		this->font.size = DEFAULT_FONT_SIZE;
		this->font.bold = DEFAULT_FONT_BOLD;
		this->font.italic = DEFAULT_FONT_ITALIC;
		this->font.underline = DEFAULT_FONT_UNDERLINE;
		this->font.strike_through = DEFAULT_FONT_STRIKETHROUGH;
		this->font.angle = DEFAULT_FONT_ANGLE;

		this->color = DEFAULT_FONT_COLOR;

		this->halign = DEFAULT_FONT_HORIZONTAL_ALIGN;
		this->valign = DEFAULT_FONT_VERTICAL_ALIGN;

		this->margin = { DEFAULT_FONT_MARGIN, DEFAULT_FONT_MARGIN ,DEFAULT_FONT_MARGIN ,DEFAULT_FONT_MARGIN };
		this->padding = { DEFAULT_FONT_PADDING, DEFAULT_FONT_PADDING, DEFAULT_FONT_PADDING, DEFAULT_FONT_PADDING };
	}

	// compute size of text using font
	bool calcRect(HDC hDC, int &rWidth, int &rHeight) const
	{
		// select font
		AUTOFONT(hDC, this->font);

		int w, h;

		if (!getTextDim(hDC, w, h))
			return false;

		rWidth = abs(w);
		rWidth += this->margin.left + this->margin.right;
		rWidth += this->padding.left + this->padding.right;
	
		rHeight = abs(h);
		rHeight += this->margin.top + this->margin.bottom;
		rHeight += this->padding.top + this->padding.bottom;
		
		return true;
	}

	// render
	void render(HDC hDC, const RECT &rRect) const
	{
		// skip if no text to display
		if (this->text.length() == 0)
			return;

		// select font
		AUTOFONT(hDC, this->font);

		// set text color
		SetTextColor(hDC, this->color);

		// compute width/height box
		int w, h;

		if (!getTextDim(hDC, w, h))
			return;

		// get x position from alignment type
		int x = 0;

		switch (this->halign)
		{
		case HorizontalAlign::Left:
			x = rRect.left + this->margin.left;
			break;

		case HorizontalAlign::Center:
			x = (rRect.right + rRect.left) / 2 - w / 2;
			break;

		case HorizontalAlign::Right:
			x = rRect.right - w - this->margin.right;
			break;
		}

		// get y position from alignment type
		int y = 0;

		switch (this->valign)
		{
		case VerticalAlign::Top:
			y = rRect.top + this->margin.top;
			break;

		case VerticalAlign::Center:
			y = (rRect.top + rRect.bottom) / 2 - h / 2;
			break;

		case VerticalAlign::Bottom:
			y = rRect.bottom - h - this->margin.bottom;
			break;
		}

		TextOutA(hDC, x, y, this->text.c_str(), (int)this->text.length());
	}

	// data that can be changed by the user
	RECT margin;
	RECT padding;
	
	/*
	guiPen border;
	guiBrush background;
	*/

	HorizontalAlign halign;
	VerticalAlign valign;

	guiFont font;

	std::string text;

	COLORREF color;

private:

	// get text dimension using font angle, return false in case of problems
	bool getTextDim(HDC hDC, int& rWidth, int& rHeight) const
	{
		// get size without rotation
		SIZE size;

		if (!GetTextExtentPoint32A(hDC, this->text.c_str(), (int)this->text.length(), &size))
			return false;

		// apply rotation formula
		double fAngleRad = -deg2rad(this->font.angle.get());

		rWidth = (int)((double)size.cx * cos(fAngleRad) - (double)size.cy * sin(fAngleRad));
		rHeight = (int)((double)size.cx * sin(fAngleRad) + (double)size.cy * cos(fAngleRad));

		// return true
		return true;
	}
};