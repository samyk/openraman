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
#include <memory>

#include <Windows.h>

#include "../utils/singleton.h"

#include "dialogs.h"

// generic procedure to be dispatched to dialog manager
INT_PTR CALLBACK genericDialogProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	// function will return FALSE (default dialog procedure) in case of errors
	try
	{
		return getInstance<DialogsManager>()->dialogProc(hWnd, uiMessage, wParam, lParam);
	}
	catch (UnknownDialogException&)
	{
		// do nothing
	}
	catch (IException& rException)
	{
		MessageBoxA(hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
	}
	catch (...)
	{
		MessageBoxA(hWnd, "Unknown exception!", "error", MB_ICONHAND | MB_OK);
	}

	return FALSE;
}

// register singleton
INITIALIZE_SINGLETON(DialogsManager);