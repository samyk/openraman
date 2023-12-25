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
#include "exception.h"
#include "evemon.h"

// send exception to event monitor
void hookException(const IException& rException, const char* pszFilename, unsigned int uiLineNumber)
{
    auto p = EventMonitor::getInstance();

    if (p != nullptr)
        p->notify(EventType::DBG_EXCEPTION, pszFilename, uiLineNumber, rException.toString());
}