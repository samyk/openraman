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

#include <windows.h>
#include <commctrl.h>

#include "shared/utils/exception.h"
#include "shared/utils/evemon.h"

#include "app.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

SpectrumAnalyzerApp* g_pMainApplication = nullptr;

// redraw part of the window
void redraw(HWND hWnd, UINT uiComponent)
{
	RECT rect;

	// get rect to redraw
	if (uiComponent > 0)
		GetWindowRect(GetDlgItem(hWnd, uiComponent), &rect);
	else
		GetWindowRect(hWnd, &rect);

	// map to own window
	MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&rect, 2);

	// send redraw command
	RedrawWindow(hWnd, &rect, NULL, RDW_UPDATENOW | RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_ALLCHILDREN);
}

double autoscale(double val)
{
	double basisfloor = floor(log(val) / log(5.0));

	return ceil(val / basisfloor) * basisfloor;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (g_pMainApplication != nullptr && hWnd == g_pMainApplication->getWindowHandle() && g_pMainApplication->windowProc(uiMessage, wParam, lParam))
			return 0;
	}
	catch (IException& rException)
	{
		MessageBoxA(hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
	}
	catch (...)
	{
		MessageBoxA(hWnd, "Unknown exception!", "error", MB_ICONHAND | MB_OK);
	}

	return DefWindowProc(hWnd, uiMessage, wParam, lParam);
}

WNDCLASS makeWndClass(HINSTANCE hInstance)
{
	WNDCLASS hWndClass;

	hWndClass.style = CS_HREDRAW | CS_VREDRAW;
	hWndClass.lpfnWndProc = wndProc;
	hWndClass.cbClsExtra = 0;
	hWndClass.cbWndExtra = 0;
	hWndClass.hInstance = hInstance;
	hWndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	hWndClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
	hWndClass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	hWndClass.lpszMenuName = NULL;
	hWndClass.lpszClassName = CLASS_NAME;

	return hWndClass;
}

BOOL WINAPI WinMain(HINSTANCE hInstance, HINSTANCE HprevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	_debug("Starting SpectrumAnalyzer");

	// check if program is already opened
	HWND hPrevWnd = FindWindow(CLASS_NAME, NULL);

	if (hPrevWnd != NULL)
	{
		_debug("A previous instance of the program was found");

		// send program the file to open if any
		if (*lpCmdLine != '\0')
		{
			// use WM_COPYDATA to send data
			COPYDATASTRUCT data;

			data.dwData = 1;
			data.lpData = (void*)lpCmdLine;
			data.cbData = (DWORD)strlen(lpCmdLine) + 1;

			SendMessage(hPrevWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&data);
		}

		// set window foreground
		SetForegroundWindow(hPrevWnd);

		// close this instance
		return FALSE;
	}

	// init application
	try
	{
		// init
		InitCommonControls();

		auto hWndClass = makeWndClass(hInstance);
		RegisterClass(&hWndClass);

		// init camera manager
		getInstance<CameraManager>()->loadInterfaces(hInstance);

		// create application
		g_pMainApplication = new SpectrumAnalyzerApp(hInstance);

		if (g_pMainApplication != nullptr)
			g_pMainApplication->init();

		// open file if specified
		if (*lpCmdLine != '\0')
			g_pMainApplication->loadFile(lpCmdLine);

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
		_critical("%s", rException.toString());

		MessageBoxA(NULL, rException.toString().c_str(), "critical error", MB_ICONHAND | MB_OK);
	}
	catch (...)
	{
		_critical("Unknown critical error!");

		MessageBox(NULL, TEXT("Cannot start application!"), TEXT("critical error"), MB_ICONHAND | MB_OK);
	}

	// delete application object
	delete g_pMainApplication;

	g_pMainApplication = nullptr;

	// clear interfaces
	try
	{
		getInstance<CameraManager>()->clearInterfaces();
	}
	catch (IException& rException)
	{
		_critical("%s", rException.toString());

		MessageBoxA(NULL, rException.toString().c_str(), "critical error", MB_ICONHAND | MB_OK);
	}
	catch (...)
	{
		_critical("Unknown critical error!");

		MessageBox(NULL, TEXT("Cannot start application!"), TEXT("critical error"), MB_ICONHAND | MB_OK);
	}

	return FALSE;
}
