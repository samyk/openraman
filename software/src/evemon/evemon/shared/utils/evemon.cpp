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

#include "evemon.h"

INITIALIZE_SINGLETON(EventMonitor);

BEGIN_STORAGE(EventData)
{
    DECLARE_STORAGE(EventData, type),
    DECLARE_STORAGE(EventData, line),
    DECLARE_STORAGE(EventData, filename),
    DECLARE_STORAGE(EventData, procname),
    DECLARE_STORAGE(EventData, message),
    DECLARE_STORAGE(EventData, ltime),
}
END_STORAGE(EventData, EventData)