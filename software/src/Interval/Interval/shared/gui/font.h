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

#include "../utils/utils.h"

#include "gdiobject.h"
#include "property.h"

// GDI font class
class guiFont : public guiGDIObject
{
public:

	// default constructor
	guiFont(void)
	{
		this->family = "";
		this->angle = 0;
		this->size = 0;
		this->bold = false;
		this->italic = false;
		this->underline = false;
		this->strike_through = false;

		// get callback if one of these variables has been changed
		WATCH(guiFont, this->family);
		WATCH(guiFont, this->angle);
		WATCH(guiFont, this->size);
		WATCH(guiFont, this->bold);
		WATCH(guiFont, this->italic);
		WATCH(guiFont, this->underline);
		WATCH(guiFont, this->strike_through);
	}

	// copy constructor
	guiFont(const guiFont& rFont) : guiFont()
	{
		this->operator=(rFont);
	}

	// move constructor
	guiFont(guiFont&& rFont) : guiFont()
	{
		this->operator=(std::move(rFont));
	}

	// copy assignment
	const guiFont& operator=(const guiFont& rFont)
	{
		this->family = rFont.family;
		this->angle = rFont.angle;
		this->size = rFont.size;
		this->bold = rFont.bold;
		this->italic = rFont.italic;
		this->underline = rFont.underline;
		this->strike_through = rFont.strike_through;

		return *this;
	}

	// move assignment
	const guiFont& operator=(guiFont&& rFont) noexcept
	{
		this->family = std::move(rFont.family);
		this->angle = std::move(rFont.angle);
		this->size = std::move(rFont.size);
		this->bold = std::move(rFont.bold);
		this->italic = std::move(rFont.italic);
		this->underline = std::move(rFont.underline);
		this->strike_through = std::move(rFont.strike_through);

		return *this;
	}

	// these property can be changed by the user
	guiProperty<std::string> family;

	guiProperty<double> angle;
	guiProperty<unsigned int> size;
	guiProperty<bool> bold, italic, underline, strike_through;

private:

	// this function is called when a property is being changed
	bool update(void)
	{
		// recreate font object
		return set(CreateFontA(this->size, 0, (int)round(this->angle * 10.0), 0, this->bold ? FW_BOLD : FW_NORMAL, this->italic ? TRUE : FALSE, this->underline ? TRUE : FALSE, this->strike_through ? TRUE : FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, this->family.get().c_str()));
	}
};

// acquire and release font using the current scope
#define AUTOFONT(dc, font)	AutoGDIObject<guiFont> __autofont__##__LINE__(dc, font);