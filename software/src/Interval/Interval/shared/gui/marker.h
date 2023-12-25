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

#include "brush.h"
#include "pen.h"
#include "property.h"
#include "gui.h"

#define DEFAULT_MARKER_SIZE					10
#define DEFAULT_MARKER_TYPE					guiMarker::Type::None

#define DEFAULT_MARKER_BORDER_THICKNESS		2
#define DEFAULT_MARKER_BORDER_COLOR			RGB(0,0,0)
#define DEFAULT_MARKER_BORDER_TYPE			guiPen::Type::Solid

#define DEFAULT_MARKER_FILL_COLOR			RGB(255,255,255)
#define DEFAULT_MARKER_FILL_TYPE			guiBrush::Type::Solid

// guiMarker class
class guiMarker
{
public:

	// types of markers
	enum class Type
	{
		None,
		Box,
		Disk,
		Triangle,
		InvTriangle,
		Diamond,
		Plus,
		Cross,
	};

	// constructor
	guiMarker(void)
	{
		this->type = DEFAULT_MARKER_TYPE;
		this->size = DEFAULT_MARKER_SIZE;

		this->border.color = DEFAULT_MARKER_BORDER_COLOR;
		this->border.thickness = DEFAULT_MARKER_BORDER_THICKNESS;
		this->border.type = DEFAULT_MARKER_BORDER_TYPE;

		this->fill.color = DEFAULT_MARKER_FILL_COLOR;
		this->fill.type = DEFAULT_MARKER_FILL_TYPE;
	}

	guiMarker(const guiMarker& rMarker)
	{
		this->operator=(rMarker);
	}

	guiMarker(guiMarker&& rMarker) noexcept
	{
		this->operator=(std::move(rMarker));
	}

	const guiMarker& operator=(const guiMarker& rMarker)
	{
		this->type = rMarker.type;

		this->border = rMarker.border;
		this->fill = rMarker.fill;

		this->size = rMarker.size;

		return *this;
	}

	const guiMarker& operator=(guiMarker&& rMarker) noexcept
	{
		this->type = rMarker.type;

		this->border = rMarker.border;
		this->fill = rMarker.fill;

		this->size = rMarker.size;

		return *this;
	}

	// render marker
	void render(HDC hDC, int cx, int cy) const
	{
		// switch between different types
		switch (this->type)
		{
		case Type::Box:
			renderBox(hDC, cx, cy);
			break;

		case Type::Disk:
			renderDisk(hDC, cx, cy);
			break;

		case Type::Diamond:
			renderDiamond(hDC, cx, cy);
			break;

		case Type::Triangle:
			renderTriangle(hDC, cx, cy);
			break;

		case Type::InvTriangle:
			renderInvTriangle(hDC, cx, cy);
			break;

		case Type::Plus:
			renderPlus(hDC, cx, cy);
			break;

		case Type::Cross:
			renderCross(hDC, cx, cy);
			break;
		}
	}

	// these properties are accessible to user
	Type type;

	guiPen border;
	guiBrush fill;

	int size;

private:

	// render box
	void renderBox(HDC hDC, int cx, int cy) const
	{
		// create point struct
		POINT points[4];

		points[0] = { cx - this->size / 2, cy - this->size / 2 };
		points[1] = { points[0].x + this->size, points[0].y };
		points[2] = { points[0].x + this->size, points[0].y + this->size };
		points[3] = { points[0].x, points[0].y + this->size };

		// render
		renderPolygon(hDC, points, sizeof(points) / sizeof(POINT));
	}

	// render diamond
	void renderDiamond(HDC hDC, int cx, int cy) const
	{
		// create point struct
		POINT points[4];

		points[0] = { cx, cy - this->size / 2 };
		points[1] = { points[0].x - this->size / 2, cy };
		points[2] = { cx, points[0].y + this->size };
		points[3] = { points[1].x + this->size, cy };

		// render
		renderPolygon(hDC, points, sizeof(points) / sizeof(POINT));
	}

	// render triangle
	void renderTriangle(HDC hDC, int cx, int cy) const
	{
		// computer coordinates
		int left = cx - this->size / 2;
		int top = cy - this->size / 2;
		int right = left + this->size;
		int bottom = top + this->size;

		// create point struct
		POINT points[3];

		points[0] = { cx, top };
		points[1] = { left, bottom };
		points[2] = { right, bottom };

		// render
		renderPolygon(hDC, points, sizeof(points) / sizeof(POINT));
	}

	// render inverted triangle
	void renderInvTriangle(HDC hDC, int cx, int cy) const
	{
		// computer coordinates
		int left = cx - this->size / 2;
		int top = cy - this->size / 2;
		int right = left + this->size;
		int bottom = top + this->size;

		// create point struct
		POINT points[3];

		points[0] = { cx, bottom };
		points[1] = { left, top };
		points[2] = { right, top };

		// render
		renderPolygon(hDC, points, sizeof(points) / sizeof(POINT));
	}

	// render disk
	void renderDisk(HDC hDC, int cx, int cy) const
	{
		// computer coordinates
		int left = cx - this->size / 2;
		int top = cy - this->size / 2;
		int right = left + this->size;
		int bottom = top + this->size;

		// fill background if not "none"
		if (this->fill.type.get() != guiBrush::Type::None)
		{
			AUTOBRUSH(hDC, this->fill);

			Ellipse(hDC, left, top, right, bottom);
		}

		// render border if not "none"
		if (this->border.type.get() != guiPen::Type::None)
		{
			AUTOPEN(hDC, this->border);

			Ellipse(hDC, left, top, right, bottom);
		}
	}

	// render plus
	void renderPlus(HDC hDC, int cx, int cy) const
	{
		// skip render border is "none"
		if (this->border.type.get() == guiPen::Type::None)
			return;

		// computer coordinates
		int left = cx - this->size / 2;
		int top = cy - this->size / 2;
		int right = left + this->size;
		int bottom = top + this->size;

		// render
		AUTOPEN(hDC, this->border);

		drawLine(hDC, left, cy, right, cy);
		drawLine(hDC, cx, top, cx, bottom);
	}

	// render cross
	void renderCross(HDC hDC, int cx, int cy) const
	{
		// skip render border is "none"
		if (this->border.type.get() == guiPen::Type::None)
			return;

		// computer coordinates
		int left = cx - this->size / 2;
		int top = cy - this->size / 2;
		int right = left + this->size;
		int bottom = top + this->size;

		// render
		AUTOPEN(hDC, this->border);

		drawLine(hDC, left, top, right, bottom);
		drawLine(hDC, right, top, left, bottom);
	}

	// render polygon
	void renderPolygon(HDC hDC, POINT* pPoints, size_t nNumPoints) const
	{
		// skip if no points
		if (pPoints == nullptr || nNumPoints == 0)
			return;

		// fill background if not "none"
		if (this->fill.type.get() != guiBrush::Type::None)
		{
			AUTOBRUSH(hDC, this->fill);

			Polygon(hDC, pPoints, (int)nNumPoints);
		}

		// render border if not "none"
		if (this->border.type.get() != guiPen::Type::None)
		{
			AUTOPEN(hDC, this->border);

			Polyline(hDC, pPoints, (int)nNumPoints);
		}
	}
};