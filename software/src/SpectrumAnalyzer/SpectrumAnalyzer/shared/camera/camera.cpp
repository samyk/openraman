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
#include "../utils/singleton.h"

#include "camera.h"

// version
#if _USRDLL
extern "C" _declspec(dllexport) unsigned long version(void)
{
    return CAMINTERFACEVERSION;
}
#else
unsigned long version(void)
{
    return CAMINTERFACEVERSION;
}

// initialize singleton only if non dll
INITIALIZE_SINGLETON(CameraManager);
#endif