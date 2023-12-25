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
#include <Windows.h>
#include <commctrl.h>
#include <uxtheme.h>

#include "shared/utils/utils.h"
#include "shared/utils/exception.h"
#include "shared/utils/notify.h"
#include "shared/gui/dialogs.h"

#include "interval.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

BOOL WINAPI WinMain(HINSTANCE hInstance, HINSTANCE HprevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// init application
	try
	{
		InitCommonControls();

		// create application
		auto pDialog = createDialog<wndIntervalDialog>(NULL, hInstance);

		if (pDialog == nullptr)
			throw InvalidDialogException();

		pDialog->init();
		pDialog->show(true);

		// message loop
		MSG hMsg;

		while (GetMessage(&hMsg, NULL, 0, 0))
		{
			TranslateMessage(&hMsg);
			DispatchMessage(&hMsg);
		}
	}
	catch (IException& rException)
	{
		MessageBoxA(NULL, rException.toString().c_str(), "critical error", MB_ICONHAND | MB_OK);
	}
	catch (...)
	{
		MessageBox(NULL, TEXT("Cannot start application!"), TEXT("critical error"), MB_ICONHAND | MB_OK);
	}

	return FALSE;
}
