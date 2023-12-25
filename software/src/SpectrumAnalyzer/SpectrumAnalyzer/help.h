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
#include <CommCtrl.h>

#include "shared/gui/dialogs.h"

#include "winmain.h"
#include "resource.h"

// about dialog class
class wndHelpDialog : public IDialog
{
public:

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_HELP;

	// set text on initialization
	virtual void init(void) override
	{
		SetDlgItemText(getWindowHandle(), IDC_SZ_STRING, TEXT("SpectrumAnalyzer V1.0.1\r\n\r\n2020 (C) OpenRAMAN\r\n\r\nLicensed under CERN-OHL-W v2\r\nVisit http://www.open-raman.org/"));

		show(true);
	}

	// hide on close
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		if (uiMessage == WM_CLOSE)
			show(false);

		return FALSE;
	}
};