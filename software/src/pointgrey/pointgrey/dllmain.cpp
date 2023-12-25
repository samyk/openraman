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
#include <windows.h>

#include <memory>

#include "shared/camera/camera.h"

#include "manager.h"

// program requires Spinnaker SDK to be installed
#pragma comment(lib, "Spinnaker_v140.lib")

// create manager instance
extern "C" _declspec(dllexport) ICameraInterface *createCameraInterface(void)
{
    return new PtGreyCameraInterface();
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}