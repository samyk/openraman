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

#include <string>
#include <memory>

#include <Windows.h>

#define HOTKEY_ACQ		1

#define AXIS_PIXELS             "Pixels #"
#define AXIS_WAVELENGTHS        "Wavelength (nm)"
#define AXIS_RAMANSHIFTS        "Wavenumber (cm-1)"

#define VAXIS_TITLE             "Intensity (a.u.)"

#define MAX_RECOMMENDED_ROI     128

#define BUFFER_SIZE				2048

#define SCREENWIDTH				960
#define SCREENHEIGHT			620

#define APP_NAMEA               "OpenRAMAN Spectrum Analyzer"
#define APP_NAME				TEXT(APP_NAMEA)
#define CLASS_NAME				TEXT("SpectrumAnalyzer")

#define IDC_BASE				0x1000
#define IDC_PLOT				(IDC_BASE + 0)
#define IDC_TOOLBAR				(IDC_BASE + 1)
#define IDM_OPEN				(IDC_BASE + 2)
#define IDM_SAVE				(IDC_BASE + 3)
#define IDM_CLIPBOARD			(IDC_BASE + 4)
#define IDM_CONNECT				(IDC_BASE + 5)
#define IDM_MULT_ACQ			(IDC_BASE + 6)
#define IDM_SINGLE_ACQ			(IDC_BASE + 7)
#define IDM_CAM_CONFIG			(IDC_BASE + 8)
#define IDM_SET_BLANK			(IDC_BASE + 9)
#define IDM_CLEAR_BLANK			(IDC_BASE + 10)
#define IDM_CALIBRATE			(IDC_BASE + 11)
#define IDM_SHOW_SATURATION		(IDC_BASE + 12)
#define IDM_HELP				(IDC_BASE + 13)
#define IDM_IMSAVE				(IDC_BASE + 14)

#define ICONID_OPEN             1
#define ICONID_SAVE             2
#define ICONID_CLIPBOARD        3
#define ICONID_CONNECT          4
#define ICONID_SINGLE_ACQ       5
#define ICONID_MULT_ACQ         6
#define ICONID_CONFIG           7
#define ICONID_SETBLANK         8
#define ICONID_CLEARBLANK       9
#define ICONID_CALIBRATE        10
#define ICONID_SHOWSAT          11
#define ICONID_HELP             12
#define ICONID_IMSAVE           13

void redraw(HWND hWnd, UINT uiComponent=0);