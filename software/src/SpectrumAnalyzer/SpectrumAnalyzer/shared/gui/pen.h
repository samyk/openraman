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

#include <Windows.h>

#include "gdiobject.h"
#include "property.h"

// GDI pen class
class guiPen : public guiGDIObject
{
public:

	// type of pen rendering
	enum class Type
	{
		None,
		Solid,
		Dash,
		Dot,
		DashDot,
		DashDotDot,
	};

	// default constructor
	guiPen(void)
	{
		this->type = Type::Solid;
		this->thickness = 0;
		this->color = RGB(0, 0, 0);

		// get callback if one of these variables has been changed
		WATCH(guiPen, this->type);
		WATCH(guiPen, this->thickness);
		WATCH(guiPen, this->color);
	}

	// copy constructor
	guiPen(const guiPen& rPen) : guiPen()
	{
		this->operator=(rPen);
	}

	// move constructor
	guiPen(guiPen&& rPen) noexcept : guiPen()
	{
		this->operator=(std::move(rPen));
	}

	// copy assignment
	const guiPen& operator=(const guiPen& rPen)
	{
		this->type = rPen.type;
		this->thickness = rPen.thickness;
		this->color = rPen.color;

		return *this;
	}

	// move assignment
	const guiPen& operator=(guiPen&& rPen) noexcept
	{
		this->type = std::move(rPen.type);
		this->thickness = std::move(rPen.thickness);
		this->color = std::move(rPen.color);

		return *this;
	}

	// these property can be changed by the user
	guiProperty<Type> type;
	guiProperty<unsigned int> thickness;
	guiProperty<COLORREF> color;

private:

	// this function is called when a property is being changed
	bool update(void)
	{
		// clear anyway
		clear();

		// dispatch style
		switch (this->type)
		{
		case Type::Solid:
			return set(CreatePen(PS_SOLID, (int)this->thickness, this->color));

		case Type::Dash:
			return set(CreatePen(PS_DASH, (int)this->thickness, this->color));

		case Type::Dot:
			return set(CreatePen(PS_DOT, (int)this->thickness, this->color));

		case Type::DashDot:
			return set(CreatePen(PS_DASHDOT, (int)this->thickness, this->color));

		case Type::DashDotDot:
			return set(CreatePen(PS_DASHDOTDOT, (int)this->thickness, this->color));

		case Type::None:
			return true;
		}

		// unhandled cases
		return false;
	}
};

// acquire and release pen using the current scope
#define AUTOPEN(dc, pen)	AutoGDIObject<guiPen> __autopen__##__LINE__(dc, pen);