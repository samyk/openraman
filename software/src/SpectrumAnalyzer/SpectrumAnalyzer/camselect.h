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

#pragma warning(disable:26451)

#include <Windows.h>
#include <CommCtrl.h>

#include "shared/gui/dialogs.h"

#include "state.h"
#include "winmain.h"
#include "resource.h"

// camera selection dialog class
class wndCameraSelectDialog : public IDialog, public SpectrumAnalyzerChild
{
public:
	// use default IDialog constructors
	using IDialog::IDialog;

	// events
	enum
	{
		EVENT_CLOSE=0,
		EVENT_CONFIRM,
	};

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_CAMERA;

	// add string to the selection box
	void add(const std::string& rLabel)
	{
		SendMessageA(getItemHandle(IDC_CAMERA), CB_ADDSTRING, (WPARAM)NULL, (LPARAM)rLabel.c_str());
		SendMessageA(getItemHandle(IDC_CAMERA), CB_SETCURSEL, (WPARAM)0, (LPARAM)NULL);
	}

	// dialog initialization
	virtual void init(void) override
	{
		// listen to events
		listen(EVENT_CLOSE, SELF(wndCameraSelectDialog::onClose));
		listen(EVENT_CONFIRM, SELF(wndCameraSelectDialog::onConfirm));

		// do not show by default
		show(false);
	}

	// dialog procedure
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_OK:
				notify(EVENT_CONFIRM);
				break;

			default:
				break;
			}

			break;
		}

		// unhandled cases use the default dialog procedure
		return FALSE;
	}

private:
	void onClose(void)
	{
		// disconnect camera
		disconnectCamera();

		show(false);
	}

	// handle ok button
	void onConfirm(void)
	{
		char camera[256];

		// check length of string
		int id = (int)SendMessage(getItemHandle(IDC_CAMERA), CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL);
		int len = (int)SendMessageA(getItemHandle(IDC_CAMERA), CB_GETLBTEXTLEN, (WPARAM)id, (LPARAM)NULL);

		// should never happen but better test it
		if (len + 1 >= sizeof(camera))
		{
			// disconnect camera
			disconnectCamera();

			// destroy window
			show(false);

			DestroyWindow(getWindowHandle());

			// skip rest
			return;
		}

		// get label text
		SendMessageA(getItemHandle(IDC_CAMERA), CB_GETLBTEXT, (WPARAM)id, (LPARAM)camera);

		// set camera
		setCamera(camera);

		// destroy window
		show(false);

		DestroyWindow(getWindowHandle());
	}
};