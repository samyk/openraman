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

#include "state.h"
#include "winmain.h"
#include "resource.h"

// imsave dialog class
class wndImageSaveDialog : public IDialog, public SpectrumAnalyzerChild
{
public:

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_IMSAVE;

	// events that can trigger callbacks
	enum
	{
		EVENT_CLOSE = 0,
		EVENT_SAVE,
	} events;

	// set text on initialization
	virtual void init(void) override
	{
		show(true);

		SetFocus(getItemHandle(IDC_PLOT_TITLE));
	}

	// hide on close
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			show(false);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CONFIRM_SAVE:
				onSave();
				close();
				return TRUE;
			}
			break;
		}

		return FALSE;
	}

private:
	void onSave(void)
	{
		char title[256];

		GetDlgItemTextA(getWindowHandle(), IDC_PLOT_TITLE, title, sizeof(title));

		onConfirmImageSave(title);
	}
};