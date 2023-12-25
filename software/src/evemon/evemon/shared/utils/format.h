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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "safe.h"

// format to std::string
static std::string format(const char* pszFormat, ...)
{
	std::string ret = "";

	va_list args;
	char* pszMessage = nullptr;

	// start reading args
	va_start(args, pszFormat);

	do
	{
		// compute length of string
		auto len = __ADD((size_t)_vscprintf(pszFormat, args), (size_t)1);

		if (len < 0)
			break;

		// allocate memory
		pszMessage = new char[len];

		// read data
		if (vsprintf_s(pszMessage, len, pszFormat, args) < 0)
			break;

		ret = std::string(pszMessage);

	} while (false);

	// stop reading args
	va_end(args);

	// delete memory
	delete[] pszMessage;
	pszMessage = nullptr;

	return ret;
}