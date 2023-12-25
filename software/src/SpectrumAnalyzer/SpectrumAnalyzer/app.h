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
#include <commctrl.h>

#include <memory>
#include <string>

#include "shared/utils/exception.h"
#include "shared/utils/notify.h"
#include "shared/gui/plot.h"
#include "shared/gui/icons.h"
#include "shared/camera/camera.h"

#include "camconfig.h"
#include "camselect.h"
#include "help.h"
#include "imsave.h"
#include "resource.h"
#include "winmain.h"
#include "acquisition.h"
#include "data.h"
#include "udata.h"
#include "filedata.h"
#include "calibrate.h"
#include "state.h"
#include "spc.h"
#include "exception.h"

// main application class
class SpectrumAnalyzerApp : public NotifyImpl, public ISpectrumAnalyzerGlobals
{
	friend class SpectrumAnalyzerChild;

public:

	// event types
	enum
	{
		EVENT_CLOSE,
		EVENT_RENDER,
		EVENT_REDRAW,
		EVENT_UPDATE,
		EVENT_FOCUS,
		EVENT_TIMER,
		EVENT_OPEN,
		EVENT_SAVE,
		EVENT_IMSAVE,
		EVENT_CLIPBOARD,
		EVENT_CONNECT,
		EVENT_SINGLE_ACQUISITION,
		EVENT_MULTIPLE_ACQUISTION,
		EVENT_PARAMS,
		EVENT_SETBLANK,
		EVENT_CLEARBLANK,
		EVENT_CALIBRATE,
		EVENT_SHOWSATURATION,
		EVENT_HELP,
		EVENT_ENABLE_ALL,
		EVENT_DISABLE_ALL,
	};

	// constructor
    SpectrumAnalyzerApp(HINSTANCE hInstance)
    {
		// copy instance
		this->m_hInstance = hInstance;

		// initialize all fields to null
		this->m_hWnd = NULL;
		this->m_hImageList = NULL;
		this->m_hSplashScreen = NULL;
		this->m_hFont = NULL;

		// all components enabled by default
		this->m_bEnable = true;

		// register events
		listen(EVENT_CLOSE, SELF(SpectrumAnalyzerApp::onClose));
		listen(EVENT_RENDER, SELF(SpectrumAnalyzerApp::onRender));
		listen(EVENT_REDRAW, SELF(SpectrumAnalyzerApp::onRedraw));
		listen(EVENT_UPDATE, SELF(SpectrumAnalyzerApp::onUpdate));
		listen(EVENT_FOCUS, SELF(SpectrumAnalyzerApp::onFocus));
		listen(EVENT_TIMER, SELF(SpectrumAnalyzerApp::onTimer));
		listen(EVENT_OPEN, SELF(SpectrumAnalyzerApp::onOpen));
		listen(EVENT_SAVE, SELF(SpectrumAnalyzerApp::onSave));
		listen(EVENT_IMSAVE, SELF(SpectrumAnalyzerApp::onImageSave));
		listen(EVENT_CLIPBOARD, SELF(SpectrumAnalyzerApp::onClipboard));
		listen(EVENT_CONNECT, SELF(SpectrumAnalyzerApp::onConnect));
		listen(EVENT_SINGLE_ACQUISITION, SELF(SpectrumAnalyzerApp::onSingleAcquisition));
		listen(EVENT_MULTIPLE_ACQUISTION, SELF(SpectrumAnalyzerApp::onMultipleAcquisition));
		listen(EVENT_PARAMS, SELF(SpectrumAnalyzerApp::onCameraConfig));
		listen(EVENT_SETBLANK, SELF(SpectrumAnalyzerApp::onSetBlank));
		listen(EVENT_CLEARBLANK, SELF(SpectrumAnalyzerApp::onClearBlank));
		listen(EVENT_CALIBRATE, SELF(SpectrumAnalyzerApp::onCalibrate));
		listen(EVENT_SHOWSATURATION, SELF(SpectrumAnalyzerApp::onShowSaturation));
		listen(EVENT_HELP, SELF(SpectrumAnalyzerApp::onHelp));
		listen(EVENT_ENABLE_ALL, SELF(SpectrumAnalyzerApp::onEnableAll));
		listen(EVENT_DISABLE_ALL, SELF(SpectrumAnalyzerApp::onDisableAll));

		// load splash screen
		this->m_hSplashScreen = (HBITMAP)LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));

		if (this->m_hSplashScreen == NULL)
			throw InitException();

		// create window
		this->m_hWnd = CreateWindow(CLASS_NAME, APP_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, SCREENWIDTH, SCREENHEIGHT, NULL, NULL, hInstance, NULL);

		if (this->m_hWnd == NULL)
			throw InitException();

		// center window
		SetWindowPos(this->m_hWnd, HWND_TOP, GetSystemMetrics(SM_CXSCREEN) / 2 - SCREENWIDTH / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - SCREENHEIGHT / 2, 0, 0, SWP_NOSIZE);

		// create toolbar
		auto hToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, TEXT(""), WS_VISIBLE | WS_CHILD | TBSTYLE_LIST | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 0, 0, 0, 0, this->m_hWnd, (HMENU)IDC_TOOLBAR, hInstance, NULL);

		int iStrNull = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT(""));
		int iStrOpen = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Open"));
		int iStrSave = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Save"));
		int iStrCopy = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Copy"));
		int iStrImage = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Image"));
		int iStrConnect = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Connect"));
		int iStrCapture = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Capture"));
		int iStrLive = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Live"));
		int iStrSetBlank = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Set Blank"));
		int iStrClearBlank = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Clear Blank"));
		int iStrConfig = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Config"));
		int iStrCalibrate = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Calibrate"));
		int iStrSaturation = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Saturation"));
		int iStrHelp = (int)SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)NULL, (LPARAM)TEXT("Help"));

		// toolbar data
		TBBUTTON buttons[] = {
			{ ICONID_OPEN, IDM_OPEN, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrOpen},
			{ ICONID_SAVE, IDM_SAVE, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrSave},
			{ ICONID_CLIPBOARD, IDM_CLIPBOARD, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrCopy},
			{ ICONID_IMSAVE, IDM_IMSAVE, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrImage},
			{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, iStrNull},
			{ ICONID_CONNECT, IDM_CONNECT, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_CHECK, {0}, 0, iStrConnect},
			{ ICONID_SINGLE_ACQ, IDM_SINGLE_ACQ, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrCapture},
			{ ICONID_MULT_ACQ, IDM_MULT_ACQ, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_CHECK, {0}, 0, iStrLive},
			{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, iStrNull},
			{ ICONID_SETBLANK, IDM_SET_BLANK, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrSetBlank},
			{ ICONID_CLEARBLANK, IDM_CLEAR_BLANK, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrClearBlank},
			{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, iStrNull},
			{ ICONID_CONFIG, IDM_CAM_CONFIG, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_CHECK, {0}, 0, iStrConfig},
			{ ICONID_CALIBRATE, IDM_CALIBRATE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_CHECK, {0}, 0, iStrCalibrate},
			{ ICONID_SHOWSAT, IDM_SHOW_SATURATION, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_CHECK, {0}, 0, iStrSaturation},
			{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, iStrNull},
			{ ICONID_HELP, IDM_HELP, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, iStrHelp},
		};

		const WPARAM wImageList = 0;
		const size_t nNumButtons = sizeof(buttons) / sizeof(TBBUTTON);

		SendMessage(hToolbar, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS);

		// import icon if existing
		try
		{
			ImportedIconSet icons;

#if 0
			icons.addIcon("OPEN", "%SystemRoot%\\System32\\SHELL32.dll", 16772);
			icons.addIcon("SAVE", "%SystemRoot%\\System32\\SHELL32.dll", 16761);
			icons.addIcon("CLIPBOARD", "%SystemRoot%\\System32\\SHELL32.dll", 148);
			icons.addIcon("IMSAVE", "%SystemRoot%\\System32\\SHELL32.dll", 63001);
			icons.addIcon("CONNECT", "%SystemRoot%\\System32\\SHELL32.dll", 244);
			icons.addIcon("SINGLE ACQ", "%SystemRoot%\\System32\\SHELL32.dll", 246);
			icons.addIcon("MULTIPLE ACQ", "%SystemRoot%\\System32\\SHELL32.dll", 224);
			icons.addIcon("CONFIG", "%SystemRoot%\\System32\\SHELL32.dll", 16775);
			icons.addIcon("SET BLANK", "%SystemRoot%\\System32\\SHELL32.dll", 322);
			icons.addIcon("CLEAR BLANK", "%SystemRoot%\\System32\\SHELL32.dll", 16777);
			icons.addIcon("SHOW SATURATION", "%SystemRoot%\\System32\\SHELL32.dll", 167);
			icons.addIcon("HELP", "%SystemRoot%\\System32\\SHELL32.dll", 1001);
			icons.addIcon("CALIBRATE", "%SystemRoot%\\System32\\SHELL32.dll", 321);

			icons.saveToFile("icons.bin");
#else

			icons.loadFromResource(IDR_ICONS);

#endif

			// load image list
			const size_t nToolbarSize = icons.getIconSize();

			this->m_hImageList = ImageList_Create((int)nToolbarSize, (int)nToolbarSize, ILC_COLOR32, 14, 0);

			if (this->m_hImageList == NULL)
				throw InitException();

			// set image list to toolbar
			ImageList_Add(this->m_hImageList, (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION), NULL);

			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_OPEN, icons.getIcon("OPEN")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_SAVE, icons.getIcon("SAVE")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_CLIPBOARD, icons.getIcon("CLIPBOARD")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_IMSAVE, icons.getIcon("IMSAVE")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_CONNECT, icons.getIcon("CONNECT")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_SINGLE_ACQ, icons.getIcon("SINGLE ACQ")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_MULT_ACQ, icons.getIcon("MULTIPLE ACQ")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_CONFIG, icons.getIcon("CONFIG")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_SHOWSAT, icons.getIcon("SHOW SATURATION")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_SETBLANK, icons.getIcon("SET BLANK")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_CLEARBLANK, icons.getIcon("CLEAR BLANK")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_HELP, icons.getIcon("HELP")));
			NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, ICONID_CALIBRATE, icons.getIcon("CALIBRATE")));
		}
		catch (...) {}

		// set toolbar data
		SendMessage(hToolbar, TB_SETIMAGELIST, (WPARAM)wImageList, (LPARAM)this->m_hImageList);
		SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
		SendMessage(hToolbar, TB_ADDBUTTONS, (WPARAM)nNumButtons, (LPARAM)&buttons);

		this->m_hFont = CreateFont(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT("Calibri"));

		if (this->m_hFont == NULL)
			throw InitException();

		SendMessage(hToolbar, WM_SETFONT, (WPARAM)this->m_hFont, (LPARAM)TRUE);

		SendMessage(hToolbar, TB_AUTOSIZE, 0, 0);
		ShowWindow(hToolbar, TRUE);

		// create render zone for plot
		RECT client_rect, toolbar_rect, render_rect;

		GetClientRect(this->m_hWnd, &client_rect);
		GetClientRect(hToolbar, &toolbar_rect);

		render_rect.left = client_rect.left + 5;
		render_rect.right = client_rect.right - 5;
		render_rect.top = toolbar_rect.bottom + 5;
		render_rect.bottom = client_rect.bottom - 5;

		auto hRenderZone = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE, render_rect.left, render_rect.top, render_rect.right - render_rect.left, render_rect.bottom - render_rect.top, this->m_hWnd, (HMENU)IDC_PLOT, hInstance, NULL);

		// create standard plot
		this->m_pPlot = createSpectrumPlot();

		if (this->m_pPlot == nullptr)
			throw InitException();

		// create parameter dialog
		this->m_pParamsDialog = createDialog<wndParametersDialog>(this->m_hWnd, hInstance);

		if (this->m_pParamsDialog == nullptr)
			throw InitException();

		this->m_pParamsDialog->setApp(this);

		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_CLOSE, SELF(SpectrumAnalyzerApp::onParamWndClose));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_EXPOSURE, SELF(SpectrumAnalyzerApp::onExposureChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_GAIN, SELF(SpectrumAnalyzerApp::onGainChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_SMOOTHING, SELF(SpectrumAnalyzerApp::onSmoothingChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_AXIS, SELF(SpectrumAnalyzerApp::onAxisChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_RAMAN_WAVELENGTH, SELF(SpectrumAnalyzerApp::onRamanWavelengthChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_BASELINE, SELF(SpectrumAnalyzerApp::onBaselineChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_BLANK, SELF(SpectrumAnalyzerApp::onBlankChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_SGOLAY, SELF(SpectrumAnalyzerApp::onSGolayChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_SGOLAY_WINDOW, SELF(SpectrumAnalyzerApp::onSGolayChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_SGOLAY_ORDER, SELF(SpectrumAnalyzerApp::onSGolayChange));
		this->m_pParamsDialog->listen(wndParametersDialog::EVENT_SGOLAY_DERIVATIVE, SELF(SpectrumAnalyzerApp::onSGolayChange));

		this->m_pParamsDialog->init();

		// create calibration dialog
		this->m_pCalibrationDialog = createDialog<wndCalibrationDialog>(this->m_hWnd, hInstance);

		if (this->m_pCalibrationDialog == nullptr)
			throw InitException();

		this->m_pCalibrationDialog->setApp(this);

		this->m_pCalibrationDialog->listen(wndCalibrationDialog::EVENT_CLOSE, SELF(SpectrumAnalyzerApp::onCalibrateWndClose));
		this->m_pCalibrationDialog->listen(wndCalibrationDialog::EVENT_UPDATE, SELF(SpectrumAnalyzerApp::onCalibrateUpdate));
		this->m_pCalibrationDialog->listen(wndCalibrationDialog::EVENT_SOLUTION_FOUND, SELF(SpectrumAnalyzerApp::onSolutionFound));
		this->m_pCalibrationDialog->listen(wndCalibrationDialog::EVENT_ON_IMPORT, SELF(SpectrumAnalyzerApp::onImportDialog));
		this->m_pCalibrationDialog->listen(wndCalibrationDialog::EVENT_ON_UPLOAD, SELF(SpectrumAnalyzerApp::onUploadCalibration));

		this->m_pCalibrationDialog->init();

		// register hot keys
		RegisterHotKey(this->m_hWnd, HOTKEY_ACQ, 0, VK_F5);

		// accept files
		DragAcceptFiles(this->m_hWnd, TRUE);

		// update window and display
		UpdateWindow(this->m_hWnd);
		ShowWindow(this->m_hWnd, TRUE);
    }

	// destructor
	~SpectrumAnalyzerApp(void)
	{
		// delete image list
		if (this->m_hImageList != NULL)
			ImageList_Destroy(this->m_hImageList);

		this->m_hImageList = NULL;

		// delete splash screen
		if (this->m_hSplashScreen != NULL)
			DeleteObject(this->m_hSplashScreen);

		this->m_hSplashScreen = NULL;

		// delete font
		if (this->m_hFont != NULL)
			DeleteObject(this->m_hFont);

		this->m_hFont = NULL;

		// post quit message
		PostQuitMessage(0);

		this->m_hWnd = NULL;
	}

	// initialize window by issueing redraw operation
	void init(void)
	{
		// redraw window
		notify(EVENT_REDRAW);
		notify(EVENT_RENDER);
	}

	void loadFile(const std::string& rFilename)
	{
		// reset name
		SetWindowTextA(getWindowHandle(), APP_NAMEA);

		// disconnect camera when opening a file to avoid calibration issues
		disconnectCamera();

		// always clear solution first when loading file
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->clear();

		// clear current blank
		onClearBlank();

		// cleanup filename
		char szFilename[MAX_PATH];
		strcpy_s(szFilename, rFilename.c_str());
		char* pszFilename = &szFilename[0];

		while (*pszFilename == '\"')
			pszFilename++;

		while (strlen(pszFilename) > 0 && pszFilename[strlen(pszFilename) - 1] == '\"')
			pszFilename[strlen(pszFilename) - 1] = '\0';

		// create data builder object
		try
		{
			// .csv case
			if (strEndsWith(pszFilename, ".csv"))
			{
				auto pDataBuilder = std::make_shared<CsvDataBuilder>(pszFilename);

				setPlotBuilder(pDataBuilder);
			}
			// .spc case
			else if (strEndsWith(pszFilename, ".spc"))
			{
				// create storage container from file
				StorageContainer container;
				container.unpack(loadBufferFromFile(pszFilename));

				// create spectre file from object
				StorageObject* pObject = container.get("", "data");

				if (pObject == nullptr)
					throw EmptyFileException(pszFilename);

				SpectreFile spc;
				spc.pop(*pObject);

				// create data builder from spectre file
				auto pDataBuilder = std::make_shared<FileDataBuilder>(spc.getData(), spc.getBlank(), spc.getUID());

				setPlotBuilder(pDataBuilder);

				// always clear current calibration
				if (this->m_pCalibrationDialog != nullptr)
					this->m_pCalibrationDialog->clear();

				// load calibration if existing
				auto pCalibration = container.get("", "calibration");

				if (pCalibration != nullptr)
				{
					// load
					storage_vector<double> calibration;
					calibration.pop(*pCalibration);

					// set
					setSolution(vector2array<double, 4>(calibration));
				}

				// load configuration if existing
				auto pConfiguration = container.get("", "config");

				if (pConfiguration != nullptr && this->m_pParamsDialog != nullptr)
					this->m_pParamsDialog->pop(*pConfiguration);
			}
			// throw exception for unknown extensions
			else
				throw UnknownFileTypeException(pszFilename);
		}
		catch (IException& rException)
		{
			MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			MessageBox(this->m_hWnd, TEXT("Unknown error!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// set window title
		auto fileparts = splitFilePath(pszFilename);
		std::string window_title = std::string(APP_NAMEA) + std::string(" - ") + fileparts.sFile;
		SetWindowTextA(getWindowHandle(), window_title.c_str());

		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);

		// update peaks if any
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->updatePeaks();

		// update components
		updateEnable();
	}

	// update enable state of all components
	void updateEnable(void)
	{
		bool bEnable = this->m_bEnable;

		enableOpen(bEnable);
		enableSave(bEnable);
		enableClipboard(bEnable);
		enableConnect(bEnable);
		enableSingleAcquisition(bEnable);
		enableMultipleAcquisition(bEnable);
		enableCameraConfig(bEnable);
		enableSetBlank(bEnable);
		enableClearBlank(bEnable);
		enableCalibrate(bEnable);
		enableShowSaturation(bEnable);
		enableHelp(bEnable);
		enableImageSave(bEnable);

		if (this->m_pParamsDialog != nullptr)
			this->m_pParamsDialog->enableAll(bEnable);

		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->enableAll(bEnable);
	}

	// return raman wavelength
	virtual double getRamanWavelength(void) const override
	{
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		return this->m_pParamsDialog->getRamanWavelength();
	}

	// return true if blank exist
	virtual bool hasBlank(void) const override
	{
		if (this->m_pPlotBuilder == nullptr)
			return false;

		return this->m_blank.size() > 0;
	}

	// return true if blank exist
	virtual vector_t getBlank(void) const override
	{
		return this->m_blank;
	}

	// return true if blank option exist
	virtual bool hasBlankOpt(void) const override
	{
		if (this->m_pPlotBuilder == nullptr)
			return false;

		return this->m_pPlotBuilder->hasBlankOpt();
	}

	// return true if a camera is connected
	virtual bool hasCamera(void) const override
	{
		try
		{
			return getInstance<CameraManager>()->getCurrentCamera() != nullptr;
		}
		catch (...) {}

		return false;
	}

	// set camera
	virtual void setCamera(const std::string& camera) override
	{
		// reset name
		SetWindowTextA(getWindowHandle(), APP_NAMEA);

		try
		{
			// always clear solution first when connecting camera
			if (this->m_pCalibrationDialog != nullptr)
				this->m_pCalibrationDialog->clear();

			// clear current blank
			onClearBlank();

			// connect camera
			getInstance<CameraManager>()->setCurrentCamera(camera);

			// load calibration from camera
			try
			{
				// load calibration
				onLoadCalibration();
			}
			catch (IException& rException)
			{
				MessageBoxA(getWindowHandle(), rException.toString().c_str(), "error", MB_ICONWARNING | MB_OK);
			}
			catch (...)
			{
				MessageBoxA(getWindowHandle(), "Failed to import calibration from camera!", "error", MB_ICONWARNING | MB_OK);
			}

			// get current camera
			auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

			if (pCamera == nullptr)
				throw NoCameraException();

			// check ROI
			if (pCamera->getROI() > MAX_RECOMMENDED_ROI)
			{
				char szTmp[256];

				sprintf_s(szTmp, "ROI is above the recommended settings which may slow down processing time. Do you want to set ROI to the maximum recommended value (%d px) ?", MAX_RECOMMENDED_ROI);

				if (MessageBoxA(this->m_hWnd, szTmp, "camera settings", MB_ICONWARNING | MB_YESNO) == IDYES)
					pCamera->setROI(MAX_RECOMMENDED_ROI);
			}

			// update configuration dialog
			if (this->m_pParamsDialog != nullptr)
			{
				// update exposure
				this->m_pParamsDialog->setExposureMinMax(pCamera->getExposureMin(), pCamera->getExposureMax());
				this->m_pParamsDialog->setExposure(pCamera->getExposure());

				// update gain
				this->m_pParamsDialog->setGainMinMax(pCamera->getGainMin(), pCamera->getGainMax());
				this->m_pParamsDialog->setGainDB(pCamera->getGain());

				// update ROI
				this->m_pParamsDialog->setROIMinMax(pCamera->getMinROI(), pCamera->getMaxROI());
				this->m_pParamsDialog->setROI(pCamera->getROI());
			}

			// update window title
			std::string window_title = std::string(APP_NAMEA) + std::string(" - ") + pCamera->uid();
			SetWindowTextA(getWindowHandle(), window_title.c_str());
		}
		catch (IException& rException)
		{
			disconnectCamera();

			// display error
			MessageBoxA(NULL, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			disconnectCamera();

			// display error
			MessageBox(this->m_hWnd, TEXT("Cannot connect to camera!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// update dialog
		if (this->m_pParamsDialog != nullptr)
			this->m_pParamsDialog->updateDialog();

		setPlotBuilder(nullptr);
	}

	// disconnect camera
	virtual void disconnectCamera(void) override
	{
		// reset name
		SetWindowTextA(getWindowHandle(), APP_NAMEA);

		// uncheck box
		SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_CHECKBUTTON, (WPARAM)IDM_CONNECT, (LPARAM)FALSE);

		// set null camera
		getInstance<CameraManager>()->setCurrentCamera("");

		// update dialog
		if (this->m_pParamsDialog != nullptr)
			this->m_pParamsDialog->updateDialog();

		setPlotBuilder(nullptr);
	}

	// return true if parameters dialog is opened
	virtual bool isParamDialogOpened(void) const override
	{
		if (this->m_pParamsDialog == nullptr)
			return false;

		return this->m_pParamsDialog->isVisible();
	}

	// return true if calibration dialog is opened
	virtual bool isCalibrationDialogOpened(void) const override
	{
		if (this->m_pCalibrationDialog == nullptr)
			return false;

		return this->m_pCalibrationDialog->isVisible();
	}

	// return true if plot exists
	virtual bool hasPlot(void) const override
	{
		if (this->m_pPlot == nullptr)
			return false;

		if (this->m_pPlotBuilder == nullptr)
			return false;

		return true;
	}

	// return true if plot has requested index data
	virtual bool hasPlotData(size_t nIndex) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return this->m_pPlot->series.size() > nIndex;
	}

	// retrieve plot data
	virtual const guiSignal& getPlotData(size_t nIndex) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return this->m_pPlot->series[nIndex];
	}

	// clear current annotations
	virtual void clearAnnotations(void) override
	{
		if (this->m_pPlotBuilder == nullptr)
			throw InvalidPlotStateException();

		// clear annotations
		this->m_pPlotBuilder->clearAnnotations();

		// update
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// add annotation
	virtual size_t addAnnotation(const guiSignal& rSignal) override
	{
		if (this->m_pPlotBuilder == nullptr)
			throw InvalidPlotStateException();

		return this->m_pPlotBuilder->addAnnotation(rSignal);
	}

	// get requested annotation
	virtual guiSignal& getAnnotation(size_t nIndex) const override
	{
		if (this->m_pPlotBuilder == nullptr)
			throw InvalidPlotStateException();

		return this->m_pPlotBuilder->getAnnotation(nIndex);
	}

	// get axis pointer
	virtual guiAxis* getPrimaryHorizontalAxis(void) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return &this->m_pPlot->haxis1;
	}

	// get axis pointer
	virtual guiAxis* getPrimaryVerticalAxis(void) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return &this->m_pPlot->vaxis1;
	}

	// get axis pointer
	virtual guiAxis* getSecondaryHorizontalAxis(void) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return &this->m_pPlot->haxis2;
	}

	// get axis pointer
	virtual guiAxis* getSecondaryVerticalAxis(void) const override
	{
		if (this->m_pPlot == nullptr)
			throw InvalidPlotException();

		return &this->m_pPlot->vaxis2;
	}

	// return true if axis change option shall be enabled
	virtual bool hasAxisChangeOpt(void) const override
	{
		if (this->m_pPlotBuilder == nullptr)
			return false;

		return this->m_pPlotBuilder->hasAxisChangeOpt();
	}

	// set current plot builder
	virtual void setPlotBuilder(std::shared_ptr<IPlotBuilder> pDataBuilder) override
	{
		this->m_pPlotBuilder = pDataBuilder;

		if (this->m_pPlotBuilder != nullptr)
			this->m_pPlotBuilder->setApp(this);

		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// return true if in multiple image acquisition mode
	virtual bool isInMultipleAcquisition(void) const override
	{
		// get button state
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		return SendMessage(hToolbar, TB_ISBUTTONCHECKED, (WPARAM)IDM_MULT_ACQ, (LPARAM)NULL) != 0;
	}

	// return true if show saturation is enabled
	virtual bool isShowSaturationDataEnabled(void) const override
	{
		return SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_ISBUTTONCHECKED, (WPARAM)IDM_SHOW_SATURATION, (LPARAM)NULL) != 0;
	}

	// get current smoothing
	virtual int getSmoothing(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve smoothing parameter
		return this->m_pParamsDialog->getSmoothing();
	}

	// get current axis type
	virtual AxisType getAxisType(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getAxisType();
	}

	// return true if median filtering is enabled
	virtual bool isMedFiltEnabled(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			return false;

		// retrieve parameter
		return this->m_pParamsDialog->isMedFiltEnabled();
	}

	// return true if baseline shall be removed
	virtual bool isBaselineRemovalEnabled(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			return false;

		// retrieve parameter
		return this->m_pParamsDialog->isBaselineRemovalEnabled();
	}

	// return true if blank shall be removed
	virtual bool isBlankRemovalEnabled(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			return false;

		// retrieve parameter
		return this->m_pParamsDialog->isBlankRemovalEnabled();
	}

	// get current exposure
	virtual double getExposure(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getExposure();
	}

	// get current gain
	virtual double getGain(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getGain();
	}

	// get current gain in DB
	virtual double getGainDB(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getGainDB();
	}

	// return sgolay enable state
	virtual bool isSGolayEnable(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			return false;

		// retrieve parameter
		return this->m_pParamsDialog->isSGolayEnable();
	}

	// return sgolay window size
	virtual int getSGolayWindowSize(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getSGolayWindow();
	}

	// return sgolay polynom order
	virtual int getSGolayOrder(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getSGolayOrder();
	}

	// return sgolay derivative order
	virtual int getSGolayDerivative(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getSGolayDerivative();
	}

	// return log format
	virtual LogFormat getLogFormat(void) const override
	{
		// skip if no param dialog
		if (this->m_pParamsDialog == nullptr)
			throw InvalidDialogException();

		// retrieve parameter
		return this->m_pParamsDialog->getLogFormat();
	}

	// return true if has calibration data
	virtual bool hasCalibrationData(void) const override
	{
		// skip if no param dialog
		if (this->m_pCalibrationDialog == nullptr)
			return false;

		// retrieve data
		return this->m_pCalibrationDialog->hasCalibrationData();
	}

	// return solution
	virtual std::array<double, 4> getSolution(void) const override
	{
		// skip if no param dialog
		if (this->m_pCalibrationDialog == nullptr)
			throw InvalidDialogException();

		// retrieve data
		return this->m_pCalibrationDialog->getSolution();
	}

	// set solution
	virtual void setSolution(const std::array<double, 4>& rSolution) override
	{
		// skip if no param dialog
		if (this->m_pCalibrationDialog == nullptr)
			throw InvalidDialogException();

		// set data
		this->m_pCalibrationDialog->setSolution(rSolution);
	}

	// return window handle
	HWND getWindowHandle(void) const
	{
		return this->m_hWnd;
	}

	// drag file
	void dragFile(HDROP hDrop)
	{
		if (DragQueryFileA(hDrop, ~0, NULL, 0) > 0)
		{
			char szBuffer[MAX_PATH];
			DragQueryFileA(hDrop, 0, szBuffer, sizeof(szBuffer));

			loadFile(szBuffer);
		}

		DragFinish(hDrop);
	}

	// event handling function
	bool windowProc(UINT uiMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			return true;

		case WM_DESTROY:
			PostQuitMessage(0);
			return true;

		case WM_COPYDATA:
			return processCopyData((HWND)wParam, (COPYDATASTRUCT*)lParam);

		case WM_DROPFILES:
			dragFile((HDROP)wParam);
			break;

		case WM_HOTKEY:
			switch (wParam)
			{
			case HOTKEY_ACQ:
				if (SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_ISBUTTONENABLED, (WPARAM)IDM_SINGLE_ACQ, (LPARAM)NULL) != 0)
					notify(EVENT_SINGLE_ACQUISITION);
				return true;
			}
			break;

		case WM_CREATE:
			
			break;

		case WM_PAINT:
			notify(EVENT_RENDER);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDM_OPEN:
				notify(EVENT_OPEN);
				return true;

			case IDM_SAVE:
				notify(EVENT_SAVE);
				return true;

			case IDM_CLIPBOARD:
				notify(EVENT_CLIPBOARD);
				return true;

			case IDM_CONNECT:
				notify(EVENT_CONNECT);
				return true;

			case IDM_SINGLE_ACQ:
				notify(EVENT_SINGLE_ACQUISITION);
				return true;

			case IDM_MULT_ACQ:
				notify(EVENT_MULTIPLE_ACQUISTION);
				return true;

			case IDM_CAM_CONFIG:
				notify(EVENT_PARAMS);
				return true;

			case IDM_SET_BLANK:
				notify(EVENT_SETBLANK);
				return true;

			case IDM_CLEAR_BLANK:
				notify(EVENT_CLEARBLANK);
				return true;

			case IDM_CALIBRATE:
				notify(EVENT_CALIBRATE);
				return true;

			case IDM_SHOW_SATURATION:
				notify(EVENT_SHOWSATURATION);
				return true;

			case IDM_HELP:
				notify(EVENT_HELP);
				return true;

			case IDM_IMSAVE:
				notify(EVENT_IMSAVE);
				return true;

			default:
				break;
			}
			break;

		case WM_SETFOCUS:
			notify(EVENT_FOCUS);
			return true;

		case WM_TIMER:
			if (wParam == 1)
				notify(EVENT_TIMER);
			return true;
		}

		return false;
	}

private:

	// process copy data message
	bool processCopyData(HWND hWnd, COPYDATASTRUCT* pData)
	{
		// skip if null
		if (pData == nullptr)
			return false;

		// dispatch messages types
		switch (pData->dwData)
		{
			// load file
		case 1:
			loadFile((char*)pData->lpData);
			return true;
		}

		// message was not processed
		return false;
	}

	// save data to file
	void saveToFile(const std::string& rFile, bool bSilentMode = false)
	{
		// skip if no data
		if (!hasPlot())
			return;

		FILE* pFile = nullptr;

		// write file stream
		fopen_s(&pFile, rFile.c_str(), "wb+");

		if (pFile == nullptr)
		{
			// do not show dialog box in silent mode
			if (!bSilentMode)
				MessageBox(this->m_hWnd, TEXT("Unable to write file!"), TEXT("error"), MB_ICONHAND | MB_OK);

			return;
		}

		// write all data to file
		try
		{
			// .csv case
			if (strEndsWith(rFile.c_str(), ".csv"))
			{
				// convert to text
				auto text = this->m_pPlotBuilder->toString(this->m_pPlot);

				// put text to file
				fputs(text.c_str(), pFile);
			}
			// .spc case
			else if (strEndsWith(rFile.c_str(), ".spc"))
			{
				// create storage container
				StorageContainer container;

				// create spectre file and push to object
				{
					// create object
					StorageObject obj("", "data");

					// create spectre file
					auto spc = this->m_pPlotBuilder->createSpectreFile();

					spc.push(obj);

					// add to container
					container.emplace_back(std::move(obj));
				}

				// get calibration if any
				if (hasCalibrationData())
				{
					// create object
					StorageObject obj("", "calibration");

					// convert to storage vector
					auto solution = getSolution();

					storage_vector<double> calibration(solution.begin(), solution.end());

					calibration.push(obj);

					// add to container
					container.emplace_back(std::move(obj));
				}

				// get parameters if any
				if (this->m_pParamsDialog != nullptr)
				{
					// create object
					StorageObject obj("", "config");

					// create sub-object and push to it
					this->m_pParamsDialog->push(obj);

					// add to container
					container.emplace_back(std::move(obj));
				}

				// pack container to storage buffer
				auto buffer = container.pack();

				// write buffer to file
				fwrite(buffer.data(), sizeof(unsigned char), buffer.size(), pFile);
			}
			// otherelse throw error
			else
				throw WrongFileTypeException(rFile);
		}
		catch (IException& rException)
		{
			// do not show dialog box in silent mode
			if (!bSilentMode)
				MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			// do not show dialog box in silent mode
			if (!bSilentMode)
				MessageBox(this->m_hWnd, TEXT("Error when saving results!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// close stream
		fclose(pFile);
	}

	// create plot
	std::shared_ptr<guiPlot> createSpectrumPlot(void)
	{
		auto plot = std::make_shared<guiPlot>();

		// skip if null
		if (plot == nullptr)
			return nullptr;

		// configure plot
		plot->haxis1.render_enable = true;
		plot->haxis1.title.font.family = "Calibri";
		plot->haxis1.title.font.angle = 0;
		plot->haxis1.title.font.size = 32;
		plot->haxis1.labels.valign = guiText::VerticalAlign::Top;
		plot->haxis1.labels.format = [](double val)
		{
			char szTmp[64];
			sprintf_s(szTmp, "%f", val);

			return std::string(szTmp);
		};

		plot->haxis2.render_enable = false;
		plot->haxis2.title.font.family = "Calibri";
		plot->haxis2.title.font.angle = 0;
		plot->haxis2.title.font.size = 32;
		plot->haxis2.labels.valign = guiText::VerticalAlign::Top;
		plot->haxis2.labels.format = [](double val)
		{
			char szTmp[64];
			sprintf_s(szTmp, "%f", val);

			return std::string(szTmp);
		};

		plot->vaxis1.render_enable = true;
		plot->vaxis1.title.font.family = "Calibri";
		plot->vaxis1.title.font.size = 32;
		plot->vaxis1.title.font.angle = 90.0;
		plot->vaxis1.labels.halign = guiText::HorizontalAlign::Right;
		plot->vaxis1.labels.format = [](double val)
		{
			char szTmp[64];
			sprintf_s(szTmp, "%f", val);

			return std::string(szTmp);
		};

		plot->vaxis2.render_enable = false;
		plot->vaxis2.title.font.family = "Calibri";
		plot->vaxis2.title.font.size = 32;
		plot->vaxis2.title.font.angle = 270.0;
		plot->vaxis2.labels.halign = guiText::HorizontalAlign::Left;
		plot->vaxis2.labels.format = [](double val)
		{
			char szTmp[64];
			sprintf_s(szTmp, "%f", val);

			return std::string(szTmp);
		};

		plot->title.font.family = "Calibri";
		plot->title.font.size = 48;
		plot->title.font.angle = 0;

		plot->margin.top = 25;

		return plot;
	}

	// double-buffered rendering
	void innerRender(HDC hDC, RECT client_rect, bool bUpdatePlot = true)
	{
		// render plot if existing
		if (hasPlot())
		{
			// render plot
			this->m_pPlot->render(hDC, client_rect);

			// skip rest
			return;
		}

		// fill gray rect
		HBRUSH hBrush = CreateSolidBrush(RGB(192, 192, 192));

		if (hBrush != NULL)
		{
			FillRect(hDC, &client_rect, hBrush);

			DeleteObject(hBrush);
		}

		// skip if no splash screen
		if (this->m_hSplashScreen == NULL)
			return;

		// get bitmap width and height
		BITMAP bmp;

		if (GetObject(this->m_hSplashScreen, sizeof(bmp), &bmp))
		{
			// center splashscreen
			int left = (client_rect.left + client_rect.right - bmp.bmWidth) / 2;
			int top = (client_rect.top + client_rect.bottom - bmp.bmHeight) / 2;

			// create DC
			HDC hBitmapDC = CreateCompatibleDC(hDC);

			// select splashscreen bitmap object;
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, this->m_hSplashScreen);

			// render
			BitBlt(hDC, left, top, bmp.bmWidth, bmp.bmHeight, hBitmapDC, 0, 0, SRCCOPY);

			// select old bitmap
			SelectObject(hBitmapDC, hOldBitmap);

			// delete object
			DeleteDC(hBitmapDC);
		}
	}

	// enable open toolbar button
	void enableOpen(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_OPEN, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable save toolbar button
	void enableSave(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasPlotData(0);

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_SAVE, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable image-save toolbar button
	void enableImageSave(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasPlotData(0);

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_IMSAVE, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable clipboard copy toolbar button
	void enableClipboard(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasPlotData(0);

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_CLIPBOARD, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable connect toolbar button
	void enableConnect(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_CONNECT, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable single acquisition toolbar button
	void enableSingleAcquisition(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasCamera();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_SINGLE_ACQ, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable multiple acquisition toolbar button
	void enableMultipleAcquisition(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= hasCamera();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_MULT_ACQ, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable parameters toolbar button
	void enableCameraConfig(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= hasPlot() || hasCamera();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_CAM_CONFIG, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable set blank toolbar button
	void enableSetBlank(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();

		if (!hasBlankOpt())
			bEnable = false;

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_SET_BLANK, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable clear blank toolbar button
	void enableClearBlank(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		if (!hasBlank())
			bEnable = false;

		if (!hasBlankOpt())
			bEnable = false;

		bEnable &= !isInMultipleAcquisition();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_CLEAR_BLANK, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable calibration toolbar button
	void enableCalibrate(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasPlot();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_CALIBRATE, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable show saturation toolbar button
	void enableShowSaturation(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		bEnable &= (this->m_pPlotBuilder != nullptr) && this->m_pPlotBuilder->hasSaturationOpt();

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_SHOW_SATURATION, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// enable about toolbar button
	void enableHelp(bool bEnable)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		SendMessage(hToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_HELP, (LPARAM)(bEnable ? TRUE : FALSE));
	}

	// redraw action
	void onRedraw(void)
	{
		redraw(this->m_hWnd, IDC_PLOT);
	}

	// on focus action
	void onFocus(void)
	{
		notify(EVENT_REDRAW);
	}

	// on close action
	void onClose(void)
	{
		if (isInMultipleAcquisition())
			this->m_pMultipleAcquisitionDialog->close();

		// disconnect camera
		disconnectCamera();

		// destroy
		DestroyWindow(this->m_hWnd);
	}

	// update plot data action
	void onUpdate(void)
	{
		// skip if no plot builder or plot
		if (this->m_pPlotBuilder == nullptr || this->m_pPlot == nullptr)
			return;

		// update plot data
		this->m_pPlotBuilder->build(this->m_pPlot);

		// adjust right margin
		if (this->m_pPlot->vaxis2.render_enable)
			this->m_pPlot->margin.right = DEFAULT_MARGIN;
		else
			this->m_pPlot->margin.right = 25;

		// adjust left margin
		if (this->m_pPlot->vaxis1.render_enable)
			this->m_pPlot->margin.left = DEFAULT_MARGIN;
		else
			this->m_pPlot->margin.left = 25;

		// adjust bottom margin
		if (this->m_pPlot->haxis1.render_enable)
			this->m_pPlot->margin.bottom = DEFAULT_MARGIN;
		else
			this->m_pPlot->margin.bottom = 25;
	}

	// render plot action
	void onRender(void)
	{
		HDC hBufferDC;
		HBITMAP hBufferBitmap;
		PAINTSTRUCT hPS;
		RECT hClientRect;

		HWND hPlotWnd = GetDlgItem(this->m_hWnd, IDC_PLOT);

		GetClientRect(hPlotWnd, &hClientRect);

		BeginPaint(hPlotWnd, &hPS);

		hBufferDC = CreateCompatibleDC(hPS.hdc);
		hBufferBitmap = CreateCompatibleBitmap(hPS.hdc, hClientRect.right - hClientRect.left, hClientRect.bottom - hClientRect.top);

		SelectObject(hBufferDC, hBufferBitmap);

		// render to hBufferDC and hClientRect
		innerRender(hBufferDC, hClientRect);

		BitBlt(hPS.hdc, hClientRect.left, hClientRect.top, hClientRect.right - hClientRect.left, hClientRect.bottom - hClientRect.top, hBufferDC, 0, 0, SRCCOPY);

		DeleteObject(hBufferBitmap);
		DeleteDC(hBufferDC);

		EndPaint(hPlotWnd, &hPS);
	}

	// clipboard action
	void onClipboard(void)
	{
		if (this->m_pPlot == nullptr)
			return;

		if (OpenClipboard(NULL) == 0)
			return;

		EmptyClipboard();

		if (this->m_pPlot->series.size() < 1)
		{
			CloseClipboard();

			return;
		}

		char szTmp[256];

		// write header
		std::string text = this->m_pPlot->haxis1.title.text + std::string("\t") + this->m_pPlot->vaxis1.title.text + std::string("\r\n");

		// scan data
		size_t n = max(this->m_pPlot->series[0].x.size(), this->m_pPlot->series[0].y.size());

		for (size_t i = 0; i < n; i++)
		{
			// write x
			if (i < this->m_pPlot->series[0].x.size())
			{
				sprintf_s(szTmp, "%.5e", this->m_pPlot->series[0].x[i]);

				text += std::string(szTmp);
			}

			// separator
			text += std::string("\t");

			// write y
			if (i < this->m_pPlot->series[0].y.size())
			{
				sprintf_s(szTmp, "%.5e", this->m_pPlot->series[0].y[i]);

				text += std::string(szTmp);
			}

			// line feed
			if (i + 1 < n)
				text += "\r\n";
		}

		HANDLE hMemory = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1);

		if (hMemory == 0)
		{
			CloseClipboard();

			return;
		}

		memcpy(GlobalLock(hMemory), text.c_str(), text.length() + 1);

		GlobalUnlock(hMemory);

		SetClipboardData(CF_TEXT, hMemory);

		CloseClipboard();
	}

	// connect to camera action
	void onConnect(void)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		auto bEnable = SendMessage(hToolbar, TB_ISBUTTONCHECKED, (WPARAM)IDM_CONNECT, (LPARAM)NULL) == TRUE;

		// disconnect camera if requested
		if (!bEnable)
		{
			disconnectCamera();

			return;
		}

		// create camera selection dialog
		try
		{
			auto pDialog = createDialog<wndCameraSelectDialog>(this->m_hWnd, this->m_hInstance);

			if (pDialog == nullptr)
				throw 0;

			notify(EVENT_DISABLE_ALL);

			pDialog->listen(wndCameraSelectDialog::EVENT_CLOSE, SELF(SpectrumAnalyzerApp::onEnableAll));
			pDialog->listen(wndCameraSelectDialog::EVENT_CONFIRM, SELF(SpectrumAnalyzerApp::onEnableAll));

			pDialog->setApp(this);

			pDialog->init();

			try
			{
				// disable all controls
				notify(EVENT_DISABLE_ALL);

				// list camera
				auto list = getInstance<CameraManager>()->listCameras();

				if (list.size() == 0)
					throw 0;

				for (auto& v : list)
					pDialog->add(v);
			}
			catch (IException& rException)
			{
				// re-enable all-controls
				notify(EVENT_ENABLE_ALL);

				// uncheck button
				SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_CHECKBUTTON, (WPARAM)IDM_CONNECT, (LPARAM)FALSE);

				// destroy window
				if (pDialog != nullptr)
					pDialog->destroy();

				// display error message
				MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);

				// skip rest
				return;
			}
			catch (...)
			{
				// re-enable all-controls
				notify(EVENT_ENABLE_ALL);

				// uncheck button
				SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_CHECKBUTTON, (WPARAM)IDM_CONNECT, (LPARAM)FALSE);

				// destroy window
				if(pDialog != nullptr)
					pDialog->destroy();

				// display error message
				MessageBox(this->m_hWnd, TEXT("No camera found!"), TEXT("error"), MB_ICONHAND | MB_OK);

				// skip rest
				return;
			}

			// show camera selection window
			pDialog->show(true);
		}
		catch (...)
		{
			// uncheck button
			SendMessage(GetDlgItem(this->m_hWnd, IDC_TOOLBAR), TB_CHECKBUTTON, (WPARAM)IDM_CONNECT, (LPARAM)FALSE);
		}
	}

	// acquisition stopped action
	void onSingleAcquisitionStop(void)
	{
		// log data if required
		if (this->m_pParamsDialog != nullptr && this->m_pParamsDialog->isLoggingEnabled())
		{
			// get formated time
			time_t raw_time;
			tm time_info;

			time(&raw_time);
			localtime_s(&time_info, &raw_time);

			char szFileName[256];

			strftime(szFileName, sizeof(szFileName), "%Y-%m-%d-%H-%M-%S", &time_info);

			switch (getLogFormat())
			{
			case LogFormat::CSV:
				strcat_s(szFileName, ".csv");
				break;

			case LogFormat::SPC:
				strcat_s(szFileName, ".spc");
				break;

			default:
				throw UnknownLogFormatException();
			}

			// build file path
			std::string fullfile = this->m_pParamsDialog->getLogPath() + std::string("\\") + std::string(szFileName);

			// save file, do not throw any message in case of errors to avoid stalling
			try
			{
				saveToFile(fullfile, true);
			}
			catch (...) {}
		}

		// re-enable everything
		notify(EVENT_ENABLE_ALL);
	}

	// acquisition update action
	void onSingleAcquisitionUpdate(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// acquisition stopped action
	void onMultipleAcquisitionStop(void)
	{
		// re-enable everything
		notify(EVENT_ENABLE_ALL);
	}

	// acquisition update action
	void onMultipleAcquisitionUpdate(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// start acquisition
	template<class Type> std::shared_ptr<Type> startAcquisition(NotifyImpl::observant_t onClose, NotifyImpl::observant_t onUpdate)
	{
		// retrieve parameters
		if (this->m_pParamsDialog == nullptr)
			return nullptr;

		auto average = this->m_pParamsDialog->getAverage();

		std::shared_ptr<Type> pDialog;

		// create acquisition dialog
		try
		{
			// get camera
			auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

			if (pCamera == nullptr)
				return nullptr;

			// create dialog
			pDialog = createDialog<Type>(this->m_hWnd, this->m_hInstance);

			if (pDialog == nullptr)
				return nullptr;

			// set state
			pDialog->setApp(this);

			// register close event
			pDialog->listen(wndIAcquisitionDialog::EVENT_CLOSE, onClose);

			// register update event
			pDialog->listen(wndIAcquisitionDialog::EVENT_UPDATE, onUpdate);

			// init dialog
			pDialog->init();

			// start acquisition
			pDialog->acquire(pCamera, average);

			// return dialog
			return pDialog;
		}
		catch (IException& rException)
		{
			MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			MessageBox(this->m_hWnd, TEXT("Cannot start acquisition!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// close dialog in case of error
		if (pDialog)
			pDialog->close();

		return nullptr;
	}

	// single image acquisition action
	void onSingleAcquisition(void)
	{
		// create dialog
		auto pDialog = startAcquisition<wndSingleImageAcquisitionDialog>(SELF(SpectrumAnalyzerApp::onSingleAcquisitionStop), SELF(SpectrumAnalyzerApp::onSingleAcquisitionUpdate));

		// skip if failed
		if (pDialog == nullptr)
			return;

		// set wait state
		notify(EVENT_DISABLE_ALL);

		// show window
		pDialog->show(true);
	}

	// multiple acquisition action
	void onMultipleAcquisition(void)
	{
		// get button state
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		auto bEnable = SendMessage(hToolbar, TB_ISBUTTONCHECKED, (WPARAM)IDM_MULT_ACQ, (LPARAM)NULL) != 0;

		// start acquisition on click
		if (bEnable)
		{
			// create dialog
			auto pDialog = startAcquisition<wndMultipleImageAcquisitionDialog>(SELF(SpectrumAnalyzerApp::onMultipleAcquisitionStop), SELF(SpectrumAnalyzerApp::onMultipleAcquisitionUpdate));

			// skip if failed
			if (pDialog == nullptr)
			{
				SendMessage(hToolbar, TB_CHECKBUTTON, (WPARAM)IDM_MULT_ACQ, (LPARAM)FALSE);

				return;
			}

			// update buttons
			notify(EVENT_ENABLE_ALL);

			// save dialog pointer
			this->m_pMultipleAcquisitionDialog = pDialog;
		}
		// stop acquisition
		else
		{
			// stop acquisition
			if (this->m_pMultipleAcquisitionDialog != nullptr)
				this->m_pMultipleAcquisitionDialog->close();

			this->m_pMultipleAcquisitionDialog = nullptr;
		}
	}

	// parameters action
	void onCameraConfig(void)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		auto bEnable = SendMessage(hToolbar, TB_ISBUTTONCHECKED, (WPARAM)IDM_CAM_CONFIG, (LPARAM)NULL) == TRUE;

		if (this->m_pParamsDialog != nullptr)
		{
			this->m_pParamsDialog->enableAll(bEnable);
			this->m_pParamsDialog->show(bEnable);
		}
		else
			SendMessage(hToolbar, TB_CHECKBUTTON, (WPARAM)IDM_CAM_CONFIG, FALSE);
	}

	// set blank action
	void onSetBlank(void)
	{
		// set blank
		if (this->m_pPlotBuilder != nullptr)
			this->m_blank = this->m_pPlotBuilder->getBlankData();

		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// clear blank action
	void onClearBlank(void)
	{
		// clear blank
		if (this->m_pPlotBuilder != nullptr)
			this->m_blank.clear();

		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// calibration action
	void onCalibrate(void)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		auto bEnable = SendMessage(hToolbar, TB_ISBUTTONCHECKED, (WPARAM)IDM_CALIBRATE, (LPARAM)NULL) == TRUE;

		// skip if no dialog
		if (this->m_pCalibrationDialog == nullptr)
		{
			SendMessage(hToolbar, TB_CHECKBUTTON, (WPARAM)IDM_CALIBRATE, (LPARAM)FALSE);

			return;
		}

		if (bEnable)
		{
			this->m_pCalibrationDialog->show(true);
			this->m_pCalibrationDialog->updatePeaks();
		}
		else
			this->m_pCalibrationDialog->close();
	}

	// show saturation action
	void onShowSaturation(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// help action
	void onHelp(void)
	{
		auto pDialog = createDialog<wndHelpDialog>(this->m_hWnd, this->m_hInstance);

		if (pDialog != nullptr)
			pDialog->init();
	}

	// parameters dialog close
	void onParamWndClose(void)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		SendMessage(hToolbar, TB_CHECKBUTTON, (WPARAM)IDM_CAM_CONFIG, (LPARAM)FALSE);
	}

	// calibration dialog close
	void onCalibrateWndClose(void)
	{
		auto hToolbar = GetDlgItem(this->m_hWnd, IDC_TOOLBAR);

		SendMessage(hToolbar, TB_CHECKBUTTON, (WPARAM)IDM_CALIBRATE, (LPARAM)FALSE);
	}

#pragma pack(push)
#pragma pack(1)
	struct calibration_data_s
	{
		unsigned short usIdent;
		unsigned char ucChecksum;
		unsigned char ucReserved;
		float coeffs[4];
	};
#pragma pack(pop)

	// load calibration from camera
	void onLoadCalibration(void)
	{
		// get camera
		auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

		if (pCamera == nullptr)
			throw NoCameraException();

		// get calibration data
		struct calibration_data_s data;

		pCamera->getUserData((unsigned char*)&data, sizeof(data));

		// verify ident
		if (data.usIdent != 0xCADA)
			throw NoCalibrationImported();

		// verify checksum
		unsigned char ucOldChecksum = data.ucChecksum;
		data.ucChecksum = 0;

		if(ucOldChecksum != checksum8((unsigned char*)&data, sizeof(data)))
			throw NoCalibrationImported();

		// convert to doubles
		std::array<double, 4> coeffs;

		for (size_t i = 0; i < 4; i++)
			coeffs[i] = (double)data.coeffs[i];

		// set solution
		setSolution(coeffs);
	}

	// upload calibration to camera
	void onUploadCalibration(void)
	{
		// skip if no solution
		if (!hasCalibrationData())
			return;

		// get camera
		auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

		if (pCamera == nullptr)
			throw NoCameraException();

		// retrieve solution
		auto coeffs = getSolution();

		// set calibration data
		struct calibration_data_s data;

		data.usIdent = 0xCADA;
		data.ucChecksum = 0;
		data.ucReserved = 0;

		for (size_t i = 0; i < 4; i++)
			data.coeffs[i] = (float)coeffs[i];

		// compute checksum
		data.ucChecksum = checksum8((unsigned char*)&data, sizeof(data));

		// upload
		pCamera->setUserData((unsigned char*)&data, sizeof(data));

		// popup message
		MessageBox(this->m_hWnd, TEXT("Calibration successfuly uploaded to camera!"), TEXT("calibration"), MB_ICONASTERISK | MB_OK);
	}

	// calibration dialog update
	void onCalibrateUpdate(void)
	{
		// notify update & redraw
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// calibration solution found
	void onSolutionFound(void)
	{
		// update parameters dialog
		if (this->m_pParamsDialog != nullptr)
			this->m_pParamsDialog->updateAxisData();
	}

	// import action
	void onImportDialog(void)
	{
		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[] = "OpenRAMAN Spectre (.spc)\0*.spc\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER;

		if (!GetOpenFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		// create storage container from file
		StorageContainer container;
		container.unpack(loadBufferFromFile(szFilename));

		// load calibration if existing
		auto pCalibration = container.get("", "calibration");

		if (pCalibration == nullptr)
			throw NoCalibrationInFileException(szFilename);

		// load calibration
		storage_vector<double> calibration;
		calibration.pop(*pCalibration);

		// retrieve UID from SpectreFile object
		std::string uid = "";

		StorageObject* pObject = container.get("", "data");

		if (pObject != nullptr)
		{
			SpectreFile spc;
			spc.pop(*pObject);

			uid = spc.getUID();
		}

		// check for UID if camera is connected
		if (hasCamera())
		{
			// get current camera
			auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

			// check UID
			if (pCamera != nullptr && pCamera->uid() != uid)
			{
				// ask user if he wants to continue
				if (MessageBox(getWindowHandle(), TEXT("Imported file was not calibrated using the same camera and may therefore yield incorrect results. Import calibration anyway ?"), TEXT("calibration"), MB_ICONWARNING | MB_YESNO) == IDNO)
					return;
			}
		}

		// set
		setSolution(vector2array<double, 4>(calibration));
	}

	// change exposure
	void onExposureChange(void)
	{
		// skip if no parameters dialog
		if (this->m_pParamsDialog == nullptr)
			return;

		// get current camera
		auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

		// skip if no camera
		if (pCamera == nullptr)
			return;

		// set exposure
		pCamera->setExposure(this->m_pParamsDialog->getExposure());
	}

	// change gain
	void onGainChange(void)
	{
		// skip if no parameters dialog
		if (this->m_pParamsDialog == nullptr)
			return;

		// get current camera
		auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

		// skip if no camera
		if (pCamera == nullptr)
			return;

		// set gain
		pCamera->setGain(this->m_pParamsDialog->getGainDB());
	}

	// change ROI
	void onROIChange(void)
	{
		// skip if no parameters dialog
		if (this->m_pParamsDialog == nullptr)
			return;

		// get current camera
		auto pCamera = getInstance<CameraManager>()->getCurrentCamera();

		// skip if no camera
		if (pCamera == nullptr)
			return;

		// set ROI
		pCamera->setROI(this->m_pParamsDialog->getROI());
	}

	// change smoothing
	void onSmoothingChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);

		// update calibration
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->updatePeaks();
	}

	// change axis type
	void onAxisChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);

		// update calibration
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->updatePeaks();
	}

	// change raman wavelength
	void onRamanWavelengthChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// change blank
	void onBlankChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);

		// update calibration
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->updatePeaks();
	}

	// change baseline
	void onBaselineChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);

		// update calibration
		if (this->m_pCalibrationDialog != nullptr)
			this->m_pCalibrationDialog->updatePeaks();
	}

	// sgolay was changed
	void onSGolayChange(void)
	{
		// update plot data
		notify(EVENT_UPDATE);
		notify(EVENT_REDRAW);
	}

	// open action
	void onOpen(void)
	{
		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[] = "OpenRAMAN Spectre (.spc)\0*.spc\0Comma Separated (.csv)\0*.csv\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER;

		if (!GetOpenFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		loadFile(szFilename);
	}

	// enable all components
	void onEnableAll(void)
	{
		this->m_bEnable = true;

		updateEnable();
	}

	// disable all components
	void onDisableAll(void)
	{
		this->m_bEnable = false;

		updateEnable();
	}

	// save action
	void onSave(void)
	{
		if (!hasPlot())
			return;

		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[256] = "OpenRAMAN Spectre (.spc)\0*.spc\0Comma Separated (.csv)\0*.csv\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if (!GetSaveFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		// add extension if not set
		switch (sFile.nFilterIndex)
		{
		case 1:
			if (!strEndsWith(szFilename, ".spc"))
				strcat_s(szFilename, ".spc");
			break;

		case 2:
			if (!strEndsWith(szFilename, ".csv"))
				strcat_s(szFilename, ".csv");
			break;
		}	

		// save file
		saveToFile(szFilename);
	}

	// save image
	void onConfirmImageSave(const std::string& rTitle)
	{
		// ImageSaveException class
		class ImageSaveException : public IException
		{
		public:
			virtual std::string toString(void) const override
			{
				return "Invalid Image Data!";
			}
		};

		if (this->m_pPlot == nullptr)
			return;

		if (!hasPlotData(0))
			return;

		if (this->m_pPlot->series[0].x.size() == 0 || this->m_pPlot->series[0].y.size() == 0)
			return;

		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[] = "*.bmp\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if (!GetSaveFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		if (!strEndsWith(szFilename, ".bmp"))
			strcat_s(szFilename, ".bmp");

		FILE* pFile = nullptr;

		// write file stream
		fopen_s(&pFile, szFilename, "wb+");

		if (pFile == nullptr)
		{
			MessageBox(this->m_hWnd, TEXT("Unable to write file!"), TEXT("error"), MB_ICONHAND | MB_OK);

			return;
		}

		const int iWidth = 860;
		const int iHeight = 560;

		// write image to file
		unsigned char* pbData = nullptr;

		// set bkcolor to white
		auto old_bkcolor = this->m_pPlot->window.background.color.get();
		this->m_pPlot->window.background.color = RGB(255, 255, 255);

		// set title
		this->m_pPlot->title.text = rTitle;

		// get window
		RECT client_rect;

		HWND hPlotWnd = GetDlgItem(this->m_hWnd, IDC_PLOT);

		client_rect.left = 0;
		client_rect.right = iWidth;
		client_rect.top = 0;
		client_rect.bottom = iHeight;

		// begin paint
		PAINTSTRUCT hPS;

		BeginPaint(hPlotWnd, &hPS);

		// create temporary bitmap & dc
		HDC hBufferDC = CreateCompatibleDC(hPS.hdc);
		HBITMAP hBufferBitmap = CreateCompatibleBitmap(hPS.hdc, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);

		// select object
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBufferDC, hBufferBitmap);

		try
		{
			// render
			innerRender(hBufferDC, client_rect, false);

			// create bitmap
			BITMAPINFO bmp_info;
			BITMAPFILEHEADER fileheader;
			BITMAPINFOHEADER infoheader;

			memset(&fileheader, 0, sizeof(BITMAPFILEHEADER));
			memset(&infoheader, 0, sizeof(BITMAPINFOHEADER));

			infoheader.biSize = sizeof(BITMAPINFOHEADER);
			infoheader.biWidth = client_rect.right - client_rect.left;
			infoheader.biHeight = client_rect.bottom - client_rect.top;
			infoheader.biPlanes = 1;
			infoheader.biBitCount = 24;
			infoheader.biClrUsed = 0;
			infoheader.biCompression = BI_RGB;
			infoheader.biSizeImage = infoheader.biWidth * infoheader.biHeight * 3 + (infoheader.biWidth % 4) * infoheader.biHeight;
			infoheader.biClrImportant = 0;
			infoheader.biXPelsPerMeter = 2834;
			infoheader.biYPelsPerMeter = 2834;

			fileheader.bfType = 'MB';
			fileheader.bfSize = sizeof(BITMAPFILEHEADER) + infoheader.biSize + infoheader.biSizeImage;
			fileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + infoheader.biSize;

			// allocate data
			pbData = new unsigned char[infoheader.biSizeImage];

			memcpy(&bmp_info, &infoheader, sizeof(BITMAPINFOHEADER));

			// get data
			if (!GetDIBits(hBufferDC, hBufferBitmap, 0, (WORD)infoheader.biHeight, pbData, &bmp_info, DIB_RGB_COLORS))
				throw ImageSaveException();

			// write file
			fwrite(&fileheader, sizeof(BITMAPFILEHEADER), 1, pFile);
			fwrite(&infoheader, sizeof(BITMAPINFOHEADER), 1, pFile);
			fwrite(pbData, sizeof(BYTE), infoheader.biSizeImage, pFile);
		}
		catch (...)
		{
			MessageBox(this->m_hWnd, TEXT("Error when saving results!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// select old object
		SelectObject(hBufferDC, hOldBitmap);
		ReleaseDC(hPlotWnd, hBufferDC);

		// delete objects
		if (hBufferBitmap != NULL)
			DeleteObject(hBufferBitmap);

		hBufferBitmap = NULL;

		if (hBufferDC != NULL)
			DeleteDC(hBufferDC);

		hBufferDC = NULL;

		// end paint
		EndPaint(hPlotWnd, &hPS);

		// delete bitmap data
		if (pbData != nullptr)
			delete[] pbData;

		pbData = nullptr;

		// reset bkcolor
		this->m_pPlot->window.background.color = old_bkcolor;

		// reset title
		this->m_pPlot->title.text = "";

		// close stream
		fclose(pFile);
	}

	// image save action
	void onImageSave(void)
	{
		notify(EVENT_DISABLE_ALL);

		auto pDialog = createDialog<wndImageSaveDialog>(this->m_hWnd, this->m_hInstance);

		if (pDialog == nullptr)
			return;

		pDialog->setApp(this);

		pDialog->listen(wndImageSaveDialog::EVENT_CLOSE, SELF(SpectrumAnalyzerApp::onEnableAll));

		pDialog->init();
	}

	// timer action
	void onTimer(void)
	{
		// kill timer
		KillTimer(this->m_hWnd, 1);

		// only if in multiple acquisition mode
		if (isInMultipleAcquisition())
		{
			// acquire image
			onSingleAcquisition();

			// set new timer
			SetTimer(this->m_hWnd, 1, 0, NULL);
		}
	}

	// called after each notification
	virtual void onNotify(int iEvent) override
	{
		// update enable state of components
		updateEnable();
	}

	// members variable
	vector_t m_blank;

	std::shared_ptr<guiPlot> m_pPlot;
	std::shared_ptr<IPlotBuilder> m_pPlotBuilder;

    std::shared_ptr<wndParametersDialog> m_pParamsDialog;
    std::shared_ptr<wndCalibrationDialog> m_pCalibrationDialog;
	std::shared_ptr<wndIAcquisitionDialog> m_pMultipleAcquisitionDialog;

	bool m_bEnable;

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HFONT m_hFont;
	HIMAGELIST m_hImageList;
	HBITMAP m_hSplashScreen;
};