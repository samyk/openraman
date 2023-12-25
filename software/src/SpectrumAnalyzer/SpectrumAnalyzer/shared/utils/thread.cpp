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
#include "thread.h"

void pfnRunThread(IThread* pThread)
{
	// call loop
	if (pThread != nullptr)
		pThread->loop();
}

// run function callback, WinAPI version
DWORD WINAPI pfnRunThreadWinAPI(LPVOID lpParam)
{
	// cast to thread
	IThread* pThread = (IThread*)lpParam;

	// call friend version
	pfnRunThread(pThread);

	// clear thread
	ExitThread(0);
}