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

 // this action will no throw exception in case of failure
#define NOTHROW(Action) { try{ Action; } catch(...){} }

// generic exception class
class IException
{
public:
    IException(void) {}

	virtual std::string toString(void) const = 0;
};

// send message to event monitor
void hookException(const IException& rException, const char* pszFilename, unsigned int uiLineNumber);

#define throwException(ExceptionType, ...)       { ExceptionType e{__VA_ARGS__}; hookException(e, __FILE__, __LINE__); throw e; }