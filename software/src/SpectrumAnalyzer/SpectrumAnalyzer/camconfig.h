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

// disable stupid warnings
#pragma warning(disable:26451)

#include <math.h>
#include <stdio.h>

#include <Windows.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <ShlObj_core.h>

#pragma comment(lib, "shlwapi.lib")

#include "shared/utils/exception.h"
#include "shared/utils/flags.h"

#include "shared/storage/storage.h"
#include "shared/storage/dynamic_var.h"

#include "shared/gui/dialogs.h"

#include "state.h"
#include "resource.h"
#include "winmain.h"
#include "settings.h"
#include "exception.h"

// key for params saving
#define KEY_SMOOTHING			"Smoothing"
#define KEY_AVERAGE				"Average"
#define KEY_MEDFILT				"MedFiltEnable"
#define KEY_LOGGING				"LoggingEnable"
#define KEY_BLANK				"BlankEnable"
#define KEY_LOGPATH				"LogPath"
#define KEY_RAMANWAVELENGTH		"RamanWavelength"
#define KEY_BASELINE			"RemoveBaseline"
#define KEY_SGOLAY_ENABLE		"SGolayEnable"
#define KEY_SGOLAY_WINDOW		"SGolayWindow"
#define KEY_SGOLAY_ORDER		"SGolayOrder"
#define KEY_SGOLAY_DERIV		"SGolayDerivative"
#define KEY_AXIS				"PreferredAxisType"
#define KEY_LOGFORMAT			"LogFormat"

// software parameters dialog class
class wndParametersDialog : public IDialog, public SpectrumAnalyzerChild, public IStoreableObject
{
public:

	// implement storage mechanism
	IMPLEMENT_STORAGE;

	// use IDialog class constructors
	using IDialog::IDialog;

	// resource id for this dialog class
	static const UINT RESOURCE_ID = IDD_CAM_CONFIG;

	// events that can trigger callbacks
	enum
	{
		EVENT_CLOSE=0,
		EVENT_INIT_DONE,
		EVENT_EXPOSURE,
		EVENT_GAIN,
		EVENT_ROI,
		EVENT_AVERAGE,
		EVENT_MEDIANFILT,
		EVENT_LOG,
		EVENT_SMOOTHING,
		EVENT_AXIS,
		EVENT_BASELINE,
		EVENT_BROWSE,
		EVENT_RAMAN_WAVELENGTH,
		EVENT_SGOLAY,
		EVENT_SGOLAY_WINDOW,
		EVENT_SGOLAY_ORDER,
		EVENT_SGOLAY_DERIVATIVE,
		EVENT_LOGFORMAT,
		EVENT_BLANK,
	} events;

	// return log format type
	virtual LogFormat getLogFormat(void) const override
	{
		int iAxis = (int)SendMessage(getItemHandle(IDC_LOGFORMAT), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (iAxis)
		{
		case 0:
			return LogFormat::CSV;

		case 1:
			return LogFormat::SPC;
		}

		throwException(UnknownLogFormatException);
	}

	// set log format
	void setLogFormat(LogFormat eLogFormat)
	{
		switch (eLogFormat)
		{
		case LogFormat::CSV:
			SendMessage(getItemHandle(IDC_LOGFORMAT), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case LogFormat::SPC:
			SendMessage(getItemHandle(IDC_LOGFORMAT), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		default:
			throwException(UnknownLogFormatException);
		}

		// notify event
		notify(EVENT_LOGFORMAT);
	}

	// return axis type
	virtual AxisType getAxisType(void) const override
	{
		int iAxis = (int)SendMessage(getItemHandle(IDC_AXIS_TYPE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (iAxis)
		{
		case 0:
			return AxisType::Pixels;

		case 1:
			return AxisType::Wavelengths;

		case 2:
			return AxisType::RamanShifts;
		}
		
		throwException(UnknownAxisTypeException);
	}

	// set axis type
	void setAxisType(AxisType eAxisType)
	{
		switch (eAxisType)
		{
		case AxisType::Pixels:
			SendMessage(getItemHandle(IDC_AXIS_TYPE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case AxisType::Wavelengths:
			SendMessage(getItemHandle(IDC_AXIS_TYPE), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		case AxisType::RamanShifts:
			SendMessage(getItemHandle(IDC_AXIS_TYPE), CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
			break;

		default:
			throwException(UnknownAxisTypeException);
		}

		// notify event
		notify(EVENT_AXIS);
	}

	// return true if median filtering is enabled
	virtual bool isMedFiltEnabled(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return IsDlgButtonChecked(getWindowHandle(), IDC_MEDFILT) == TRUE;
	}

	// set median filtering
	void enableMedFiltParam(bool bEnable)
	{
		// set checkbox
		CheckDlgButton(getWindowHandle(), IDC_MEDFILT, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_MEDIANFILT);
	}

	// return true if baseline removal is enabled
	virtual bool isBaselineRemovalEnabled(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return IsDlgButtonChecked(getWindowHandle(), IDC_BASELINE) == TRUE;
	}

	// set baseline removal
	void enableBaselineRemovalParam(bool bEnable)
	{
		// set checkbox
		CheckDlgButton(getWindowHandle(), IDC_BASELINE, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_BASELINE);
	}

	// return true if blank removal is enabled
	virtual bool isBlankRemovalEnabled(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return IsDlgButtonChecked(getWindowHandle(), IDC_BLANK) == TRUE;
	}

	// set blank removal
	void enableBlankRemovalParam(bool bEnable)
	{
		// set checkbox
		CheckDlgButton(getWindowHandle(), IDC_BLANK, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_BLANK);
	}

	// return log path
	std::string getLogPath(void) const
	{
		return this->m_logpath;
	}

	// return true if logging data
	bool isLoggingEnabled(void) const
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return IsDlgButtonChecked(getWindowHandle(), IDC_LOG) == TRUE;
	}

	// enable logging
	void enableLoggingParam(bool bEnable)
	{
		// set checkbox
		CheckDlgButton(getWindowHandle(), IDC_LOG, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_LOG);
	}

	// return smoothing kernel size
	virtual int getSmoothing(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return 1 + 2 * (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_SMOOTHING_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set smoothing
	void setSmoothing(int iSmoothing)
	{
		// convert
		iSmoothing = (iSmoothing - 1) >> 1;

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SMOOTHING_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SMOOTHING_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iSmoothing = max(minval, min(iSmoothing, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SMOOTHING_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iSmoothing);

		// notify event
		notify(EVENT_SMOOTHING);
	}

	// return number of images to average
	int getAverage(void) const
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_AVERAGE_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set average
	void setAverage(int iAverage)
	{
		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_AVERAGE_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_AVERAGE_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iAverage = max(minval, min(iAverage, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_AVERAGE_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iAverage);

		// notify event
		notify(EVENT_AVERAGE);
	}

	// return exposure in seconds
	virtual double getExposure(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		int iExposureTicks = (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_EXPOSURE_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);

		return 1e-3 * pow(10, 0.25e-1 * (double)iExposureTicks);
	}

	// set exposure
	void setExposure(double fExposureSeconds)
	{
		if (fExposureSeconds <= 0)
			return;

		// convert to int
		int iExposureTicks = (int)floor(log10(fExposureSeconds * 1000.0) / 0.25e-1);

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_EXPOSURE_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_EXPOSURE_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iExposureTicks = max(minval, min(iExposureTicks, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_EXPOSURE_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iExposureTicks);

		// notify event
		notify(EVENT_EXPOSURE);
	}

	// update min/max settings for exposure
	void setExposureMinMax(double fMin, double fMax)
	{
		auto transform = [](double fValue)
		{
			return log10(fValue * 1000) / 0.25e-1;
		};

		// transform to int values
		int iMin = (int)ceil(transform(fMin));
		int iMax = (int)floor(transform(fMax));

		int iTickSpacing = (int)round(10.0 / (double)(iMax - iMin));

		// send messages
		SendMessage(getItemHandle(IDC_EXPOSURE_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
		SendMessage(getItemHandle(IDC_EXPOSURE_SLIDER), TBM_SETTICFREQ, (WPARAM)iTickSpacing, (LPARAM)0);
	}

	// return gain in dB
	virtual double getGainDB(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		int iGainTicks = (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_GAIN_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);

		return 0.1 * (double)iGainTicks;
	}

	// return gain in linscale
	virtual double getGain(void) const
	{
		return pow(10, getGainDB() / 20.0);
	}

	// set exposure
	void setGainDB(double fGainDB)
	{
		// convert to int
		int iGain = (int)floor(fGainDB * 10.0);

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_GAIN_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_GAIN_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iGain = max(minval, min(iGain, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_GAIN_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iGain);

		// notify event
		notify(EVENT_GAIN);
	}

	// update min/max settings for gain
	void setGainMinMax(double fMin, double fMax)
	{
		// transform to int values
		int iMin = (int)ceil(fMin * 10.0);
		int iMax = (int)floor(fMax * 10.0);

		int iTickSpacing = (int)round(10.0 / (double)(iMax - iMin));

		// send messages
		SendMessage(getItemHandle(IDC_GAIN_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
		SendMessage(getItemHandle(IDC_GAIN_SLIDER), TBM_SETTICFREQ, (WPARAM)iTickSpacing, (LPARAM)0);
	}

	// return ROI in pixels
	int getROI(void) const
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_ROI_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set ROI
	void setROI(int iROI)
	{
		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_ROI_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_ROI_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iROI = max(minval, min(iROI, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_ROI_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iROI);

		// notify event
		notify(EVENT_ROI);
	}

	// update min/max settings for ROI
	void setROIMinMax(int iMin, int iMax)
	{
		int iTickSpacing = (int)round(10.0 / (double)(iMax - iMin));

		// send messages
		SendMessage(getItemHandle(IDC_ROI_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
		SendMessage(getItemHandle(IDC_ROI_SLIDER), TBM_SETTICFREQ, (WPARAM)iTickSpacing, (LPARAM)0);
	}

	// enable all components
	void enableAll(bool bEnable)
	{
		enableCameraAcquisitionGroup(bEnable);
		enablePlotComponentsGroup(bEnable);
		enableLogGroup(bEnable);
	}

	// set folder for logging
	void setLogPath(const std::string& rPath)
	{
		// save path
		this->m_logpath = rPath;

		// create directory if not existing
		if (PathFileExistsA(rPath.c_str()) == FALSE)
			CreateDirectoryA(rPath.c_str(), NULL);

		// update text
		SetDlgItemTextA(getWindowHandle(), IDC_LOG_PATH, this->m_logpath.c_str());

		// save to registry
		saveString(KEY_LOGPATH, this->m_logpath);
	}

	// set raman wavelength
	void setRamanWavelength(double fWavelength)
	{
		// convert to int
		int iWavelength = (int)floor(fWavelength * 10.0);

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_RAMAN_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_RAMAN_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iWavelength = max(minval, min(iWavelength, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_RAMAN_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iWavelength);

		// notify event
		notify(EVENT_RAMAN_WAVELENGTH);
	}

	// get raman wavelength
	virtual double getRamanWavelength(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		int iTicks = (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_RAMAN_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);

		return 0.1 * (double)iTicks;
	}

	// return true if sgolay is enabled
	virtual bool isSGolayEnable(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return IsDlgButtonChecked(getWindowHandle(), IDC_SGOLAY) == TRUE;
	}

	// enable sgolay
	void enableSGolayParam(bool bEnable)
	{
		// set checkbox
		CheckDlgButton(getWindowHandle(), IDC_SGOLAY, bEnable ? TRUE : FALSE);

		// notify event
		notify(EVENT_SGOLAY);
	}

	// return sgolay window
	virtual int getSGolayWindow(void) const
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return 1 + 2 * (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_SGOLAY_WINDOW_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set sgolay window
	void setSGolayWindow(int iWindow)
	{
		// adapt to range
		iWindow = (iWindow - 1) / 2;

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iWindow = max(minval, min(iWindow, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iWindow);

		// notify event
		notify(EVENT_SGOLAY_WINDOW);
	}

	// return sgolay order
	virtual int getSGolayOrder(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_SGOLAY_ORDER_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set sgolay order
	void setSGolayOrder(int iOrder)
	{
		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iOrder = max(minval, min(iOrder, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iOrder);

		// notify event
		notify(EVENT_SGOLAY_ORDER);
	}

	// return sgolay derivative
	virtual int getSGolayDerivative(void) const override
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// return data
		return (int)SendMessage(GetDlgItem(getWindowHandle(), IDC_SGOLAY_DERIV_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set sgolay derivative
	void setSGolayDerivative(int iDerivative)
	{
		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iDerivative = max(minval, min(iDerivative, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iDerivative);

		// notify event
		notify(EVENT_SGOLAY_DERIVATIVE);
	}

	// update dialog
	void updateDialog(void)
	{
		// set rest to "N/A" if no camera is connected
		if (!hasCamera())
		{
			char szTmp[64];
			sprintf_s(szTmp, "N/A");

			SetDlgItemTextA(getWindowHandle(), IDC_EXPOSURE_EDIT, szTmp);
			SetDlgItemTextA(getWindowHandle(), IDC_GAIN_EDIT, szTmp);
			SetDlgItemTextA(getWindowHandle(), IDC_AVERAGE_EDIT, szTmp);
			SetDlgItemTextA(getWindowHandle(), IDC_ROI_EDIT, szTmp);
		}
		else
			onAverage();
	}

	// update axis element
	void updateAxisData(void)
	{
		// disable axis if no calibration data
		if (!hasCalibrationData())
		{
			// reset choice list
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Pixels");
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

			// force axis to pixels
			if(getAxisType() != AxisType::Pixels)
				setAxisType(AxisType::Pixels);
		}
		else
		{
			// reset choice list
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Pixels");
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Wavelengths");
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Raman Shifts");
			SendMessageA(getItemHandle(IDC_AXIS_TYPE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

			// set axis to wavelengths
			setAxisType(AxisType::Wavelengths);
		}
	}

	// set preferred log format
	void setPreferredLogFormat(void)
	{
		// get format
		auto format = loadString(KEY_LOGFORMAT, ".csv");

		// set format
		if (format == ".csv")
			setLogFormat(LogFormat::CSV);
		else if (format == ".spc")
			setLogFormat(LogFormat::SPC);
	}

	// set default axis
	void setPreferredAxis(void)
	{
		// get axis
		auto axis = loadString(KEY_AXIS, "Wavelengths");

		// set axis
		if (axis == "Pixels")
			setAxisType(AxisType::Pixels);
		else if (axis == "Wavelengths")
			setAxisType(AxisType::Wavelengths);
		else if (axis == "RamanShifts")
			setAxisType(AxisType::RamanShifts);
	}

	// initialize dialog
	virtual void init(void) override
	{
		// listen to events
		listen(EVENT_CLOSE, SELF(wndParametersDialog::onClose));
		listen(EVENT_EXPOSURE, SELF(wndParametersDialog::onExposure));
		listen(EVENT_GAIN, SELF(wndParametersDialog::onGain));
		listen(EVENT_ROI, SELF(wndParametersDialog::onROI));
		listen(EVENT_BROWSE, SELF(wndParametersDialog::onBrowse));
		listen(EVENT_SMOOTHING, SELF(wndParametersDialog::onSmoothing));
		listen(EVENT_AVERAGE, SELF(wndParametersDialog::onAverage));
		listen(EVENT_LOG, SELF(wndParametersDialog::onLogging));
		listen(EVENT_MEDIANFILT, SELF(wndParametersDialog::onMedFilt));
		listen(EVENT_RAMAN_WAVELENGTH, SELF(wndParametersDialog::onRamanWavelength));
		listen(EVENT_BASELINE, SELF(wndParametersDialog::onBaseline));
		listen(EVENT_SGOLAY, SELF(wndParametersDialog::onSGolay));
		listen(EVENT_SGOLAY_WINDOW, SELF(wndParametersDialog::onSGolayWindow));
		listen(EVENT_SGOLAY_ORDER, SELF(wndParametersDialog::onSGolayOrder));
		listen(EVENT_SGOLAY_DERIVATIVE, SELF(wndParametersDialog::onSGolayDerivative));
		listen(EVENT_AXIS, SELF(wndParametersDialog::onAxisChange));
		listen(EVENT_LOGFORMAT, SELF(wndParametersDialog::onLogFormatChange));
		listen(EVENT_BLANK, SELF(wndParametersDialog::onBlank));

		// bind dynamic vars
		BIND_DYNAMIC_VAR(wndParametersDialog, this->smoothing, getSmoothing, setSmoothing);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->average, getAverage, setAverage);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->sgolay_window, getSGolayWindow, setSGolayWindow);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->sgolay_order, getSGolayOrder, setSGolayOrder);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->sgolay_deriv, getSGolayDerivative, setSGolayDerivative);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->raman_wavelength, getRamanWavelength, setRamanWavelength);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->medfilt, isMedFiltEnabled, enableMedFiltParam);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->baseline, isBaselineRemovalEnabled, enableBaselineRemovalParam);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->sgolay, isSGolayEnable, enableSGolayParam);
		BIND_DYNAMIC_VAR(wndParametersDialog, this->axis, getAxisType, setAxisType);

		// initialize axis type combobox
		updateAxisData();

		// initialize log format
		SendMessageA(getItemHandle(IDC_LOGFORMAT), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Comma Separated (.csv)");
		SendMessageA(getItemHandle(IDC_LOGFORMAT), CB_ADDSTRING, (WPARAM)0, (LPARAM)"OpenRAMAN Spectre (.spc)");
		SendMessageA(getItemHandle(IDC_LOGFORMAT), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		setPreferredLogFormat();

		// initialize exposure slider
		setExposureMinMax(1e-3, 10);
		setExposure(1);

		// initialize gain slider
		setGainMinMax(0, 10);
		setGainDB(0);

		// initialize raman slider
		SendMessage(getItemHandle(IDC_RAMAN_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(3500, 8500));
		SendMessage(getItemHandle(IDC_RAMAN_SLIDER), TBM_SETTICFREQ, (WPARAM)10, (LPARAM)0);

		setRamanWavelength(0.1 * (double)loadInt(KEY_RAMANWAVELENGTH, 5320));

		// initialize smoothing slider
		SendMessage(getItemHandle(IDC_SMOOTHING_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 10));
		SendMessage(getItemHandle(IDC_SMOOTHING_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		setSmoothing(loadInt(KEY_SMOOTHING, 3));

		// initialize average slider
		SendMessage(getItemHandle(IDC_AVERAGE_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1, 100));
		SendMessage(getItemHandle(IDC_AVERAGE_SLIDER), TBM_SETTICFREQ, (WPARAM)10, (LPARAM)0);

		setAverage(loadInt(KEY_AVERAGE, 1));

		// initialize ROI slider
		setROIMinMax(1, 1);
		setROI(1);

		// enable median filtering by default
		enableMedFiltParam(loadBool(KEY_MEDFILT, true));

		// disable log by default
		enableLoggingParam(loadBool(KEY_LOGGING, false));

		// enable blank removal by default
		enableBlankRemovalParam(loadBool(KEY_BLANK, true));

		// set user folder by default to log
		char szPath[MAX_PATH];

		SHGetSpecialFolderPathA(getWindowHandle(), szPath, CSIDL_MYDOCUMENTS, FALSE);

		setLogPath(loadString(KEY_LOGPATH, std::string(szPath) + std::string("\\OpenRAMAN")));

		// initialize baseline removal
		enableBaselineRemovalParam(loadBool(KEY_BASELINE, false));

		// initialize sgolay sliders
		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 15));
		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)5);
		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 5));
		SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)2);
		SendMessage(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 5));
		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);
		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		enableSGolayParam(loadBool(KEY_SGOLAY_ENABLE, false));
		setSGolayWindow(loadInt(KEY_SGOLAY_WINDOW, 5));
		setSGolayOrder(loadInt(KEY_SGOLAY_ORDER, 2));
		setSGolayDerivative(loadInt(KEY_SGOLAY_DERIV, 0));

		// do not show after initialization
		show(false);
	}

	dynamic_var<int> smoothing, average, sgolay_window, sgolay_order, sgolay_deriv;
	dynamic_var<double> raman_wavelength;
	dynamic_var<bool> medfilt, baseline, sgolay;
	dynamic_var<AxisType> axis;

private:

	// handle events
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		// handle dialog close, do NOT return false to prevent the dialog from being destroyed by the default dialog procedure
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			return TRUE;

		// update edit components if sliders have been changed
		case WM_HSCROLL:
			if ((HWND)lParam == getItemHandle(IDC_EXPOSURE_SLIDER))
				notify(EVENT_EXPOSURE);
			else if ((HWND)lParam == getItemHandle(IDC_GAIN_SLIDER))
				notify(EVENT_GAIN);
			else if ((HWND)lParam == getItemHandle(IDC_AVERAGE_SLIDER))
				notify(EVENT_AVERAGE);
			else if ((HWND)lParam == getItemHandle(IDC_ROI_SLIDER))
				notify(EVENT_ROI);
			else if ((HWND)lParam == getItemHandle(IDC_SMOOTHING_SLIDER))
				notify(EVENT_SMOOTHING);
			else if ((HWND)lParam == getItemHandle(IDC_SGOLAY_WINDOW_SLIDER))
				notify(EVENT_SGOLAY_WINDOW);
			else if ((HWND)lParam == getItemHandle(IDC_SGOLAY_ORDER_SLIDER))
				notify(EVENT_SGOLAY_ORDER);
			else if ((HWND)lParam == getItemHandle(IDC_SGOLAY_DERIV_SLIDER))
				notify(EVENT_SGOLAY_DERIVATIVE);
			else if ((HWND)lParam == getItemHandle(IDC_RAMAN_SLIDER))
				notify(EVENT_RAMAN_WAVELENGTH);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_LOG:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_LOG);
				break;

			case IDC_BROWSE:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_BROWSE);
				break;

			case IDC_AXIS_TYPE:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					notify(EVENT_AXIS);
				break;

			case IDC_LOGFORMAT:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					notify(EVENT_LOGFORMAT);
				break;

			case IDC_BASELINE:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_BASELINE);
				break;

			case IDC_MEDFILT:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_MEDIANFILT);
				break;

			case IDC_SGOLAY:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_SGOLAY);
				break;

			case IDC_BLANK:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_BLANK);
				break;
			}
			break;
		}

		// all other cases rely on the default dialog procedure
		return FALSE;
	}

	// update components state after all event notify
	virtual void onNotify(int iEvent) override
	{
		enableAll(true);

		updateDialog();
	}

	// enable exposure
	void enableExposure(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasCamera();

		// exposure components
		EnableWindow(getItemHandle(IDC_SZ_EXPOSURE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_EXPOSURE_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_EXPOSURE_EDIT), bEnable ? TRUE : FALSE);
	}

	// enable gain
	void enableGain(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasCamera();

		// gain components
		EnableWindow(getItemHandle(IDC_SZ_GAIN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_GAIN_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_GAIN_EDIT), bEnable ? TRUE : FALSE);
	}

	// enable average
	void enableAverage(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasCamera();

		// disable in multiple acquisition mode
		bEnable &= !isInMultipleAcquisition();

		// average components
		EnableWindow(getItemHandle(IDC_SZ_AVERAGE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_AVERAGE_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_AVERAGE_EDIT), bEnable ? TRUE : FALSE);
	}

	// enable ROI
	void enableROI(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasCamera();

		// disable in multiple acquisition mode
		bEnable &= !isInMultipleAcquisition();

		// ROI components
		EnableWindow(getItemHandle(IDC_SZ_ROI), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_ROI_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_ROI_EDIT), bEnable ? TRUE : FALSE);
	}

	// enable median filtering
	void enableMedianFiltering(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasCamera();

		// median filtering component
		EnableWindow(getItemHandle(IDC_MEDFILT), bEnable ? TRUE : FALSE);
	}

	// enable camera acquisition components
	void enableCameraAcquisitionGroup(bool bEnable)
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// disable all if no camera is connected
		bEnable &= hasCamera();

		// group
		EnableWindow(getItemHandle(IDC_CAMERA_GROUP), bEnable ? TRUE : FALSE);

		enableExposure(bEnable);
		enableGain(bEnable);
		enableAverage(bEnable);
		enableROI(bEnable);
		enableMedianFiltering(bEnable);
	}

	// enable axis
	void enableAxis(bool bEnable)
	{
		bEnable &= hasPlot();
		bEnable &= hasAxisChangeOpt();

		// axis type
		EnableWindow(getItemHandle(IDC_SZ_AXIS), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_AXIS_TYPE), bEnable ? TRUE : FALSE);
	}

	// enable raman wavelength
	void enableRamanWavelength(bool bEnable)
	{
		bEnable &= hasPlot();
		bEnable &= hasAxisChangeOpt();
		bEnable &= getAxisType() == AxisType::RamanShifts;

		// axis type
		EnableWindow(getItemHandle(IDC_SZ_RAMAN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_RAMAN_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ2_RAMAN), bEnable ? TRUE : FALSE);
	}

	// enable smoothing
	void enableSmoothing(bool bEnable)
	{
		bEnable &= hasPlot();

		// smoothing components
		EnableWindow(getItemHandle(IDC_SZ_SMOOTHING), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SMOOTHING_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SMOOTHING_EDIT), bEnable ? TRUE : FALSE);
	}

	// enable baseline removal
	void enableBaselineRemoval(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasPlot();

		// median filtering component
		EnableWindow(getItemHandle(IDC_BASELINE), bEnable ? TRUE : FALSE);
	}

	// enable blank removal
	void enableBlankRemoval(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasBlank();

		// median filtering component
		EnableWindow(getItemHandle(IDC_BLANK), bEnable ? TRUE : FALSE);
	}

	// enable SGolay part
	void enableSGolay(bool bEnable)
	{
		// disable all if no camera is connected
		bEnable &= hasPlot();

		// median filtering component
		EnableWindow(getItemHandle(IDC_SGOLAY), bEnable ? TRUE : FALSE);

		// skip rest if checkbox is unchecked
		bEnable &= isSGolayEnable();

		EnableWindow(getItemHandle(IDC_SZ_SGOLAY_WINDOW), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_WINDOW_EDIT), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), bEnable ? TRUE : FALSE);

		EnableWindow(getItemHandle(IDC_SZ_SGOLAY_ORDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_ORDER_EDIT), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_ORDER_SLIDER), bEnable ? TRUE : FALSE);

		EnableWindow(getItemHandle(IDC_SZ_SGOLAY_DERIV), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_DERIV_EDIT), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), bEnable ? TRUE : FALSE);
	}

	// enable plot components
	void enablePlotComponentsGroup(bool bEnable)
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		bEnable &= hasPlot();

		// group
		EnableWindow(getItemHandle(IDC_PLOT_GROUP), bEnable ? TRUE : FALSE);

		enableAxis(bEnable);
		enableSmoothing(bEnable);
		enableBaselineRemoval(bEnable);
		enableBlankRemoval(bEnable);
		enableRamanWavelength(bEnable);
		enableSGolay(bEnable);
	}

	// enable log
	void enableLogGroup(bool bEnable)
	{
		// throw exception if no window (should never happen)
		if (getWindowHandle() == NULL)
			throwException(NoWindowException);

		// disable all if no camera is connected
		bEnable &= !isInMultipleAcquisition();
		bEnable &= hasCamera();

		// group
		EnableWindow(getItemHandle(IDC_LOG_GROUP), bEnable ? TRUE : FALSE);

		// log checkbox
		EnableWindow(getItemHandle(IDC_LOG), bEnable ? TRUE : FALSE);

		// disable rest of the components if the button is unchecked
		bEnable &= IsDlgButtonChecked(getWindowHandle(), IDC_LOG) == TRUE;

		EnableWindow(getItemHandle(IDC_BROWSE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_LOG_PATH), bEnable ? TRUE : FALSE);

		EnableWindow(getItemHandle(IDC_SZ_LOGFORMAT), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_LOGFORMAT), bEnable ? TRUE : FALSE);
	}

	// handle browse button
	void onBrowse(void)
	{
		char szPath[MAX_PATH];

		// initialize Com
		CoInitialize(NULL);

		// browse for file
		BROWSEINFOA info;

		memset(&info, 0, sizeof(info));

		info.hwndOwner = getWindowHandle();
		info.pidlRoot = NULL;
		info.pszDisplayName = szPath;
		info.lpszTitle = "Select Folder";
		info.ulFlags = BIF_NEWDIALOGSTYLE;

		PIDLIST_ABSOLUTE pList = SHBrowseForFolderA(&info);

		// retrieve path and set it
		if (pList != NULL)
		{
			SHGetPathFromIDListA(pList, szPath);

			setLogPath(szPath);
		}

		// clear result
		if(pList != NULL)
			CoTaskMemFree(pList);

		pList = NULL;

		// de-initialize Com
		CoUninitialize();
	}

	// handle close button
	void onClose(void)
	{
		// make window invisible
		show(false);
	}

	// ROI action
	void onROI(void)
	{
		auto value = getROI();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%d px", value);
		SetDlgItemTextA(getWindowHandle(), IDC_ROI_EDIT, szTmp);
	}

	// smoothing action
	void onSmoothing(void)
	{
		auto iValue = getSmoothing();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%d", iValue);
		SetDlgItemTextA(getWindowHandle(), IDC_SMOOTHING_EDIT, szTmp);

		// save to registry
		saveInt(KEY_SMOOTHING, iValue);
	}

	// exposure action
	void onExposure(void)
	{
		auto value = getExposure();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%.3lf ms", 1e3 * value);
		SetDlgItemTextA(getWindowHandle(), IDC_EXPOSURE_EDIT, szTmp);
	}

	// gain action
	void onGain(void)
	{
		auto gain = getGainDB();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%.1lf dB", gain);
		SetDlgItemTextA(getWindowHandle(), IDC_GAIN_EDIT, szTmp);
	}

	// average action
	void onAverage(void)
	{
		auto iValue = getAverage();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%d", iValue);
		SetDlgItemTextA(getWindowHandle(), IDC_AVERAGE_EDIT, szTmp);

		// save to registry
		saveInt(KEY_AVERAGE, iValue);
	}

	// med filt action
	void onMedFilt(void)
	{
		// save to registry
		saveBool(KEY_MEDFILT, isMedFiltEnabled());
	}

	// blank action
	void onBlank(void)
	{
		// save to registry
		saveBool(KEY_BLANK, isBlankRemovalEnabled());
	}

	// logging action
	void onLogging(void)
	{
		// save to registry
		saveBool(KEY_LOGGING, isLoggingEnabled());
	}

	// raman wavelength action
	void onRamanWavelength(void)
	{
		auto wavelength = getRamanWavelength();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%.1lf nm", wavelength);
		SetDlgItemTextA(getWindowHandle(), IDC_SZ2_RAMAN, szTmp);

		// save to registry
		saveInt(KEY_RAMANWAVELENGTH, (int)(10.0 * getRamanWavelength()));
	}

	// baseline removal action
	void onBaseline(void)
	{
		// save to registry
		saveBool(KEY_BASELINE, isBaselineRemovalEnabled());
	}

	// sgolay check action
	void onSGolay(void)
	{
		// save to registry
		saveBool(KEY_SGOLAY_ENABLE, isSGolayEnable());
	}

	// sgolay window action
	void onSGolayWindow(void)
	{
		// get value
		int iValue = getSGolayWindow();

		// update edit components
		char szTmp[64];
		sprintf_s(szTmp, "%d pts", iValue);
		SetDlgItemTextA(getWindowHandle(), IDC_SGOLAY_WINDOW_EDIT, szTmp);

		// save to registry
		saveInt(KEY_SGOLAY_WINDOW, iValue);
	}

	// sgolay order action
	void onSGolayOrder(void)
	{
		// get value
		int iValue = getSGolayOrder();

		// update edit components
		char szTmp[64];

		if(iValue == 0)
			sprintf_s(szTmp, "boxcar");
		else if(iValue == 1)
			sprintf_s(szTmp, "linear");
		else if(iValue == 2)
			sprintf_s(szTmp, "quadratic");
		else if(iValue == 3)
			sprintf_s(szTmp, "cubic");
		else if(iValue == 4)
			sprintf_s(szTmp, "quartic");
		else if(iValue == 5)
			sprintf_s(szTmp, "quintic");
		else
			sprintf_s(szTmp, "%dth", iValue);

		SetDlgItemTextA(getWindowHandle(), IDC_SGOLAY_ORDER_EDIT, szTmp);

		// save to registry
		saveInt(KEY_SGOLAY_ORDER, iValue);

		// update range of other fields
		int minOrder = 0;

		while ((1 + 2 * minOrder) <= iValue)
			minOrder++;

		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(minOrder, 49));
		SendMessage(getItemHandle(IDC_SGOLAY_WINDOW_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, iValue));
		SendMessage(getItemHandle(IDC_SGOLAY_DERIV_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		// update other fields in case they are out of range
		setSGolayWindow(getSGolayWindow());
		setSGolayDerivative(getSGolayDerivative());
	}

	// sgolay derivative action
	void onSGolayDerivative(void)
	{
		// get value
		int iValue = getSGolayDerivative();

		// update edit components
		char szTmp[64];

		if (iValue == 0)
			sprintf_s(szTmp, "smooth");
		else if (iValue == 1)
			sprintf_s(szTmp, "1st");
		else if (iValue == 2)
			sprintf_s(szTmp, "2nd");
		else if (iValue == 3)
			sprintf_s(szTmp, "3rd");
		else
			sprintf_s(szTmp, "%dth", iValue);

		SetDlgItemTextA(getWindowHandle(), IDC_SGOLAY_DERIV_EDIT, szTmp);

		// save to registry
		saveInt(KEY_SGOLAY_DERIV, iValue);
	}

	// axis change action
	void onAxisChange(void)
	{
		switch (getAxisType())
		{
		case AxisType::Pixels:
			saveString(KEY_AXIS, "Pixels");
			break;

		case AxisType::Wavelengths:
			saveString(KEY_AXIS, "Wavelengths");
			break;

		case AxisType::RamanShifts:
			saveString(KEY_AXIS, "RamanShifts");
			break;
		}
	}

	// log format change action
	void onLogFormatChange(void)
	{
		switch (getLogFormat())
		{
		case LogFormat::CSV:
			saveString(KEY_LOGFORMAT, ".csv");
			break;

		case LogFormat::SPC:
			saveString(KEY_LOGFORMAT, ".spc");
			break;
		}
	}

	// push variable to storage object
	template<typename Type> void push(StorageObject& rContainer, const char *pszName, Type value) const
	{
		rContainer.addVariable("config", pszName, typeid(value).name(), sizeof(value), &value);
	}

	std::string m_logpath;
};