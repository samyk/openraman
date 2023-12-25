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

// GDI brush class
class guiBrush : public guiGDIObject
{
public:

	// type of brush rendering
	enum class Type
	{
		None,
		Solid,
		HatchDiagonal,
	};

	// default constructor
	guiBrush(void)
	{
		this->type = Type::Solid;
		this->color = RGB(0, 0, 0);

		// get callback if one of these variables has been changed
		WATCH(guiBrush, this->type);
		WATCH(guiBrush, this->color);
	}

	// copy constructor
	guiBrush(const guiBrush& rBrush) : guiBrush()
	{
		this->operator=(rBrush);
	}

	// copy constructor
	guiBrush(guiBrush&& rBrush) noexcept : guiBrush()
	{
		this->operator=(std::move(rBrush));
	}

	// copy assignment
	const guiBrush& operator=(const guiBrush& rBrush)
	{
		this->type = rBrush.type;
		this->color = rBrush.color;

		return *this;
	}

	const guiBrush& operator=(guiBrush&& rBrush) noexcept
	{
		this->type = std::move(rBrush.type);
		this->color = std::move(rBrush.color);

		return *this;
	}

	// fill rect using the current brush
	void fillRect(HDC hDC, const RECT& rRect) const
	{
		if(this->type.get() != Type::None)
			FillRect(hDC, &rRect, (HBRUSH)get());
	}

	// these property can be changed by the user
	guiProperty<Type> type;
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
			return set(CreateSolidBrush(this->color));

		case Type::HatchDiagonal:
			return set(CreateHatchBrush(HS_BDIAGONAL, this->color));

		case Type::None:
			return true;
		}

		// unhandled cases return false
		return false;
	}
};

// acquire and release brush using the current scope
#define AUTOBRUSH(dc, brush)	AutoGDIObject<guiBrush> __autobrush__##__LINE__(dc, brush);