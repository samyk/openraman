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

#include "../utils/exception.h"

// GDIObjectNotAcquiredException class
class GDIObjectNotAcquiredException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Cannot release object that was not acquired first!";
	}
};

// GDI object class
class guiGDIObject
{
public:

	// default constructor
	guiGDIObject(void)
	{
		this->m_iResourceCounter = 0;
		this->m_hObject = NULL;
	}

	// GDI objects cannot be copied or moved
	guiGDIObject(const guiGDIObject&) = delete;
	guiGDIObject(guiGDIObject&&) = delete;

	guiGDIObject& operator=(const guiGDIObject&) = delete;
	guiGDIObject& operator=(guiGDIObject&&) = delete;

	// clear on destruct
	~guiGDIObject(void)
	{
		clear();
	}

	// return true if object is valid
	bool valid(void) const
	{
		return this->m_hObject != NULL;
	}

	// set current object and return previously set object
	HGDIOBJ acquire(HDC dc) const
	{
		// increment reference counter
		this->m_iResourceCounter++;

		// select object and return previously set one
		return SelectObject(dc, this->m_hObject);
	}

	// release object and resume previously set object
	void release(HDC dc, HGDIOBJ hOldObject) const
	{
		// TODO: trigger error if this->m_iResourceCounter == 0
		if (this->m_iResourceCounter == 0)
			throwException(GDIObjectNotAcquiredException);

		// decrement resource counter
		this->m_iResourceCounter--;

		// select previous object
		SelectObject(dc, hOldObject);
	}

	// return true if object is being used
	bool isLocked(void) const
	{
		return this->m_iResourceCounter != 0;
	}

	// get GDI object handle
	const HGDIOBJ get(void) const
	{
		return this->m_hObject;
	}

	// set new GDI object, return false in case of falure
	bool set(HGDIOBJ hObject)
	{
		// cannot change object if in use
		if (isLocked())
			return false;

		// delete previous object if any
		if (this->m_hObject != NULL)
			DeleteObject(this->m_hObject);

		// replace object and return true
		this->m_hObject = hObject;

		return true;
	}

	// clear current object, return false in case of failure
	bool clear(void)
	{
		// cannot delete object if in use
		if (isLocked())
			return false;

		// delete object and return true
		if (this->m_hObject != NULL)
			DeleteObject(this->m_hObject);

		this->m_hObject = NULL;

		return true;
	}

private:
	mutable int m_iResourceCounter;

	HGDIOBJ m_hObject;
};

// automaticaly call acquire and release of a GDI object using RIAA idiom
template<class Type> class AutoGDIObject
{
public:

	// acquire in constructor
	AutoGDIObject(HDC hDC, const Type& rObject) : m_rObject(rObject)
	{
		this->m_hDC = hDC;
		this->m_hOldObject = this->m_rObject.acquire(this->m_hDC);
	}

	// release in destructor
	~AutoGDIObject(void)
	{
		try
		{
			this->m_rObject.release(this->m_hDC, this->m_hOldObject);
		}
		catch (...) {}
	}

private:
	const Type& m_rObject;

	HDC m_hDC;
	HGDIOBJ m_hOldObject;
};