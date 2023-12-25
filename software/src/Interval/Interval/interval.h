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
#include "shared/gui/icons.h"

#include "resource.h"

enum class MultiplierType
{
	Seconds,
	Minutes,
	Hours,
};

enum class LoopMode
{
	Infinite,
	NumTriggers,
};

// UnknownLoopModeException exception class
class UnknownLoopModeException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown loop mode!";
	}
};

// UnknownMultiplierTypeException exception class
class UnknownMultiplierTypeException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown multiplier type!";
	}
};

// interval dialog class
class wndIntervalDialog : public IDialog
{
public:

	// events
	enum
	{
		EVENT_INIT_DONE,
		EVENT_CLOSE,
		EVENT_DESTROY,
		EVENT_RUN,
		EVENT_DELAY_SEL,
		EVENT_DELAY_EDIT,
		EVENT_LOOPMODE,
		EVENT_NUMLOOPS,
		EVENT_UPDATE,
	};

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_DIALOG;

	// set text on initialization
	virtual void init(void) override
	{
		// listen to event
		listen(EVENT_CLOSE, SELF(wndIntervalDialog::onClose));
		listen(EVENT_DESTROY, SELF(wndIntervalDialog::onDestroy));
		listen(EVENT_RUN, SELF(wndIntervalDialog::onRun));
		listen(EVENT_DELAY_SEL, SELF(wndIntervalDialog::onDelaySelection));
		listen(EVENT_DELAY_EDIT, SELF(wndIntervalDialog::onDelayEdit));
		listen(EVENT_LOOPMODE, SELF(wndIntervalDialog::onLoopMode));
		listen(EVENT_NUMLOOPS, SELF(wndIntervalDialog::onNumLoops));
		listen(EVENT_UPDATE, SELF(wndIntervalDialog::onUpdate));

		// initialize components
		SendMessageA(getItemHandle(IDC_DELAY_LIST), CB_ADDSTRING, (WPARAM)0, (LPARAM)"sec");
		SendMessageA(getItemHandle(IDC_DELAY_LIST), CB_ADDSTRING, (WPARAM)0, (LPARAM)"min");
		SendMessageA(getItemHandle(IDC_DELAY_LIST), CB_ADDSTRING, (WPARAM)0, (LPARAM)"hour");
		SendMessageA(getItemHandle(IDC_DELAY_LIST), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		setLoopMode(LoopMode::Infinite);
		setMultiplierType(MultiplierType::Minutes);
		setDelay(1);
		setNumLoops(1);

		// set icon of program
		this->m_iconSmall = ImportedIcon("%SystemRoot%\\system32\\accessibilitycpl.dll", 16, 339);
		this->m_iconLarge = ImportedIcon("%SystemRoot%\\system32\\accessibilitycpl.dll", 48, 339);

		SendMessage(getWindowHandle(), WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)this->m_iconSmall.getIcon());
		SendMessage(getWindowHandle(), WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)this->m_iconLarge.getIcon());

		// notify init done
		notify(EVENT_INIT_DONE);
	}

	// hide on close
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			return true;

		case WM_DESTROY:
			notify(EVENT_DESTROY);
			return true;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_RUN:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_RUN);

					return true;
				}
				break;

			case IDC_DELAY_EDIT:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					notify(EVENT_DELAY_EDIT);

					return true;
				}
				break;

			case IDC_DELAY_LIST:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					notify(EVENT_DELAY_SEL);

					return true;
				}
				break;

			case IDC_INFINITE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					setLoopMode(LoopMode::Infinite);

					return true;
				}
				break;

			case IDC_FINITE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					setLoopMode(LoopMode::NumTriggers);

					return true;
				}
				break;

			case IDC_NUMLOOPS:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					notify(EVENT_NUMLOOPS);

					return true;
				}
				break;
			}
			break;

		case WM_TIMER:
			if (wParam == 1)
			{
				notify(EVENT_UPDATE);

				return true;
			}

			break;
		}

		return FALSE;
	}

	// update enable state
	void enableAll(bool bEnable)
	{
		_enableSettingsGroup(bEnable);
		_enableStatus(bEnable);
		_enableRun(bEnable);
	}

	/* get & set */

	// get multiplier type
	MultiplierType getMultiplierType(void) const
	{
		int iAxis = (int)SendMessage(getItemHandle(IDC_DELAY_LIST), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (iAxis)
		{
		case 0:
			return MultiplierType::Seconds;

		case 1:
			return MultiplierType::Minutes;

		case 2:
			return MultiplierType::Hours;
		}

		throwException(UnknownMultiplierTypeException);
	}

	// set multiplier type
	void setMultiplierType(MultiplierType eMultiplierType)
	{
		switch (eMultiplierType)
		{
		case MultiplierType::Seconds:
			SendMessage(getItemHandle(IDC_DELAY_LIST), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case MultiplierType::Minutes:
			SendMessage(getItemHandle(IDC_DELAY_LIST), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		case MultiplierType::Hours:
			SendMessage(getItemHandle(IDC_DELAY_LIST), CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
			break;

		default:
			throwException(UnknownMultiplierTypeException);
		}

		// notify event
		notify(EVENT_DELAY_SEL);
	}

	// get delay
	int getDelay(void) const
	{
		return GetDlgItemInt(getWindowHandle(), IDC_DELAY_EDIT, NULL, FALSE);
	}

	// set delay
	void setDelay(int iDelay)
	{
		// set value
		SetDlgItemInt(getWindowHandle(), IDC_DELAY_EDIT, (UINT)abs(iDelay), FALSE);

		// notify event
		notify(EVENT_DELAY_EDIT);
	}

	// get loop mode
	LoopMode getLoopMode(void) const
	{
		if (IsDlgButtonChecked(getWindowHandle(), IDC_INFINITE))
			return LoopMode::Infinite;

		if (IsDlgButtonChecked(getWindowHandle(), IDC_FINITE))
			return LoopMode::NumTriggers;

		throwException(UnknownLoopModeException);
	}

	// set loop mode
	void setLoopMode(LoopMode eLoopMode)
	{
		switch (eLoopMode)
		{
		case LoopMode::Infinite:
			CheckRadioButton(getWindowHandle(), IDC_INFINITE, IDC_FINITE, IDC_INFINITE);
			break;

		case LoopMode::NumTriggers:
			CheckRadioButton(getWindowHandle(), IDC_INFINITE, IDC_FINITE, IDC_FINITE);
			break;

		default:
			throwException(UnknownLoopModeException);
		}

		// notify event
		notify(EVENT_LOOPMODE);
	}

	// get num loops
	int getNumLoops(void) const
	{
		return GetDlgItemInt(getWindowHandle(), IDC_NUMLOOPS, NULL, FALSE);
	}

	// set num loop
	void setNumLoops(int iNumLoops)
	{
		// set value
		SetDlgItemInt(getWindowHandle(), IDC_NUMLOOPS, (UINT)abs(iNumLoops), FALSE);

		// notify event
		notify(EVENT_NUMLOOPS);
	}

	// return true if running
	bool isRunning(void) const
	{
		return IsDlgButtonChecked(getWindowHandle(), IDC_RUN);
	}

	// set running status
	void run(bool bEnable)
	{
		// check button
		CheckDlgButton(getWindowHandle(), IDC_RUN, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_RUN);
	}

	// return delay time in seconds
	int getTimeSeconds(void) const
	{
		int iTimeSeconds = getDelay();

		switch (getMultiplierType())
		{
		case MultiplierType::Seconds:
			break;

		case MultiplierType::Minutes:
			iTimeSeconds *= 60;
			break;

		case MultiplierType::Hours:
			iTimeSeconds *= 3600;
			break;
		}

		return iTimeSeconds;
	}

private:

	// update components state on notifications
	virtual void onNotify(int iEvent) override
	{
		enableAll(true);
	}

	/* enable components */

	// enable group
	void _enableSettingsGroup(bool bEnable)
	{
		bEnable &= !isRunning();

		EnableWindow(getItemHandle(IDC_SETTINGS_GROUP), bEnable ? TRUE : FALSE);

		EnableWindow(getItemHandle(IDC_SZ_DELAY), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_DELAY_EDIT), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_DELAY_LIST), bEnable ? TRUE : FALSE);

		EnableWindow(getItemHandle(IDC_INFINITE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_FINITE), bEnable ? TRUE : FALSE);

		bEnable &= (getLoopMode() == LoopMode::NumTriggers);

		EnableWindow(getItemHandle(IDC_NUMLOOPS), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_LOOPS), bEnable ? TRUE : FALSE);
	}

	// enable status
	void _enableStatus(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_STATUS), bEnable ? TRUE : FALSE);
	}

	// enable run
	void _enableRun(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_RUN), bEnable ? TRUE : FALSE);
	}

	/* actions */

	// close action
	void onClose(void)
	{
		show(false);

		DestroyWindow(getWindowHandle());
	}

	// destroy action
	void onDestroy(void)
	{
		PostQuitMessage(0);
	}

	// delay selection action
	void onDelaySelection(void) {}

	// delay edit action
	void onDelayEdit(void) {}

	// loop mode changed
	void onLoopMode(void) {}

	// number of loops changed
	void onNumLoops(void) {}

	// update action
	void onUpdate(void)
	{
		// hit F5 key
		INPUT input;
		memset(&input, 0, sizeof(input));

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_F5;

		SendInput(1, &input, sizeof(INPUT));

		// update status
		this->m_nNumTriggers++;

		char szStatus[128];
		sprintf_s(szStatus, "#%zu triggers", this->m_nNumTriggers);
		SetDlgItemTextA(getWindowHandle(), IDC_STATUS, szStatus);

		// stop eventually
		if (getLoopMode() == LoopMode::NumTriggers && this->m_nNumTriggers >= getNumLoops())
			run(false);
	}

	// run action
	void onRun(void)
	{
		if (isRunning())
		{
			this->m_nNumTriggers = 0;

			SetTimer(getWindowHandle(), 1, getTimeSeconds() * 1000, NULL);
		}
		else
			KillTimer(getWindowHandle(), 1);
	}

	size_t m_nNumTriggers;

	ImportedIcon m_iconSmall, m_iconLarge;
};