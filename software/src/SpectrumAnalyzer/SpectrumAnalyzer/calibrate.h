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

#include "shared/utils/exception.h"
#include "shared/utils/thread.h"

#include "shared/storage/storage.h"
#include "shared/storage/dynamic_var.h"

#include "shared/math/optfuncs.h"
#include "shared/math/calibration.h"
#include "shared/math/interp.h"

#include "shared/gui/dialogs.h"
#include "shared/gui/marker.h"
#include "shared/gui/signal.h"
#include "shared/gui/pen.h"
#include "shared/gui/brush.h"

#include "state.h"
#include "winmain.h"
#include "resource.h"
#include "settings.h"
#include "exception.h"

// key for settings save
#define KEY_SAMPLING			"CalibrationSampling"
#define KEY_MINRANGE			"CalibrationMinRange"
#define KEY_MAXRANGE			"CalibrationMaxRange"
#define KEY_MINSPAN				"CalibrationMinSpan"
#define KEY_MAXSPAN				"CalibrationMaxSpan"
#define KEY_MINDIST				"CalibrationMinDistort"
#define KEY_MAXDIST				"CalibrationMaxDistort"
#define KEY_MODELTYPE			"CalibrationModel"

 // calibration dialog class
class wndCalibrationDialog : public IDialog, public SpectrumAnalyzerChild
{
public:

	// event types
	enum
	{
		EVENT_CLOSE=0,
		EVENT_INIT_DONE,
		EVENT_UPDATE,
		EVENT_SOURCE_TYPE,
		EVENT_NUMPEAKS,
		EVENT_PEAK_SENSITIVITY,
		EVENT_MODEL_TYPE,
		EVENT_SAMPLING,
		EVENT_MIN_RANGE,
		EVENT_MAX_RANGE,
		EVENT_MIN_SPAN,
		EVENT_MAX_SPAN,
		EVENT_MIN_DISTORTION,
		EVENT_MAX_DISTORTION,
		EVENT_ON_CALIBRATE,
		EVENT_ON_ABORT,
		EVENT_ON_RESET,
		EVENT_ON_IMPORT,
		EVENT_ON_UPLOAD,
		EVENT_SOLUTION_FOUND,
	};

	// model types
	enum class Model
	{
		Linear,
		Cubic,
	};

	// source types
	enum class Source
	{
		Neon,
		MercuryArgon,
	};

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_CALIBRATE;

	// return true if calibration/solution exists
	virtual bool hasCalibrationData(void) const override
	{
		return this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->hasSolution();
	}

	// return solution
	virtual std::array<double, 4> getSolution(void) const override
	{
		// throw error if no solution found
		if (this->m_pOptimizationThread == nullptr || !this->m_pOptimizationThread->hasSolution())
			throw NoSolutionException();

		return vector2array<double, 4>(this->m_pOptimizationThread->getSolution(4));
	}

	// set solution
	virtual void setSolution(const std::array<double, 4>& rSolution) override
	{
		// create static solution
		this->m_pOptimizationThread = std::make_shared<StaticSolution>(array2vector(rSolution));

		// notify solution has been found
		notify(EVENT_SOLUTION_FOUND);
	}

	// set minimum range
	void setRangeMin(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MIN_WAVELENGTH, szTmp);

		// notify event
		notify(EVENT_MIN_RANGE);
	}

	// get minimum range
	double getRangeMin(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MIN_WAVELENGTH, NULL, FALSE);
	}

	// set maximum range
	void setRangeMax(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MAX_WAVELENGTH, szTmp);

		// notify event
		notify(EVENT_MAX_RANGE);
	}

	// get maximum range
	double getRangeMax(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MAX_WAVELENGTH, NULL, FALSE);
	}

	// set minimum span
	void setSpanMin(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MIN_SPAN, szTmp);

		// notify event
		notify(EVENT_MIN_SPAN);
	}

	// get minimum span
	double getSpanMin(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MIN_SPAN, NULL, FALSE);
	}

	// set maximum span
	void setSpanMax(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MAX_SPAN, szTmp);

		// notify event
		notify(EVENT_MAX_SPAN);
	}

	// get maximum span
	double getSpanMax(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MAX_SPAN, NULL, FALSE);
	}

	// set minimum distortion
	void setDistortionMin(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MIN_DISTORTION, szTmp);

		// notify event
		notify(EVENT_MIN_DISTORTION);
	}

	// get minimum distortion
	double getDistortionMin(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MIN_DISTORTION, NULL, FALSE);
	}

	// set maximum distortion
	void setDistortionMax(double fValue)
	{
		// set value
		char szTmp[64];

		sprintf_s(szTmp, "%.0f", fValue);

		SetDlgItemTextA(getWindowHandle(), IDC_MAX_DISTORTION, szTmp);

		// notify event
		notify(EVENT_MAX_DISTORTION);
	}

	// get maximum distortion
	double getDistortionMax(void) const
	{
		return (double)GetDlgItemInt(getWindowHandle(), IDC_MAX_DISTORTION, NULL, FALSE);
	}

	// set model type
	void setModelType(Model eType)
	{
		switch (eType)
		{
		case Model::Linear:
			SendMessage(getItemHandle(IDC_MODEL), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case Model::Cubic:
			SendMessage(getItemHandle(IDC_MODEL), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		default:
			throw UnknownModelException();
		}

		// notify event
		notify(EVENT_MODEL_TYPE);
	}

	// set model type by string
	void setModelType(const std::string& rModel)
	{
		if (rModel == "Linear")
			setModelType(Model::Linear);
		else if (rModel == "Cubic")
			setModelType(Model::Cubic);
		else
			throw UnknownModelException();
	}

	// get model type
	auto getModelType(void) const
	{
		int sel = (int)SendMessage(getItemHandle(IDC_MODEL), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (sel)
		{
		case 0:
			return Model::Linear;

		case 1:
			return Model::Cubic;

		default:
			throw UnknownModelException();
		}
	}

	// set source type
	void setSourceType(Source eType)
	{
		switch (eType)
		{
		case Source::Neon:
			SendMessage(getItemHandle(IDC_SOURCE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case Source::MercuryArgon:
			SendMessage(getItemHandle(IDC_SOURCE), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		default:
			throw UnknownSourceException();
		}

		// notify event
		notify(EVENT_SOURCE_TYPE);
	}

	// get source type
	auto getSourceType(void) const
	{
		int sel = (int)SendMessage(getItemHandle(IDC_SOURCE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (sel)
		{
		case 0:
			return Source::Neon;
		
		case 1:
			return Source::MercuryArgon;

		default:
			throw UnknownSourceException();
		}
	}

	// return list of peaks
	auto getCalibrationData(void) const
	{
		// dispatch cases
		switch (getSourceType())
		{
		case Source::Neon:
			return ::getCalibrationData(CalibrationData::Neon);

		case Source::MercuryArgon:
			return ::getCalibrationData(CalibrationData::MercuryArgon);

			// throw error for unknown cases
		default:
			throw UnknownSourceException();
		}
	}

	// set sensitivity
	void setSensitivity(double fSensitivity)
	{
		// convert to integer
		auto sensitivity = (int)floor(100.0 * fSensitivity);

		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		sensitivity = max(minval, min(sensitivity, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)sensitivity);

		// notify event
		notify(EVENT_PEAK_SENSITIVITY);
	}

	// get sensitivity
	double getSensitivity(void) const
	{
		return 0.01 * (double)SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set number of peaks
	void setNumPeaks(size_t nNumPeaks)
	{
		// get range
		auto minval = (size_t)SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (size_t)SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		nNumPeaks = max(minval, min(nNumPeaks, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nNumPeaks);

		// notify event
		notify(EVENT_NUMPEAKS);
	}

	// get number of peaks
	size_t getNumPeaks(void) const
	{
		return (size_t)SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// set sampling
	void setSampling(int iSampling)
	{
		// get range
		auto minval = (int)SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0);
		auto maxval = (int)SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0);

		// limit to range
		iSampling = max(minval, min(iSampling, maxval));

		// update slider position
		SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iSampling);

		// notify event
		notify(EVENT_SAMPLING);
	}

	// get sampling
	int getSampling(void) const
	{
		return (int)SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// update peak detection
	void updatePeaks(void)
	{
		// skip if not visible
		if (!isVisible())
			return;

		// detect peaks
		onDetectPeaks();
	}

	// clear solution
	void clear(void)
	{
		// clear annotated peaks
		this->m_detectedPeaks.clear();

		// stop thread if running
		if(this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			this->m_pOptimizationThread->stop();

		// clear solution
		this->m_pOptimizationThread.reset();

		// send solution update
		updatePeaks();
	}

	// initialize dialog
	virtual void init(void) override
	{
		// listen to self
		listen(EVENT_MODEL_TYPE, SELF(wndCalibrationDialog::onModelChange));
		listen(EVENT_SOURCE_TYPE, SELF(wndCalibrationDialog::onSourceChange));
		listen(EVENT_NUMPEAKS, SELF(wndCalibrationDialog::onNumPeaksChange));
		listen(EVENT_PEAK_SENSITIVITY, SELF(wndCalibrationDialog::onSensitivityChange));
		listen(EVENT_NUMPEAKS, SELF(wndCalibrationDialog::onDetectPeaks));
		listen(EVENT_PEAK_SENSITIVITY, SELF(wndCalibrationDialog::onDetectPeaks));
		listen(EVENT_SAMPLING, SELF(wndCalibrationDialog::onSamplingChange));
		listen(EVENT_ON_CALIBRATE, SELF(wndCalibrationDialog::onCalibrate));
		listen(EVENT_CLOSE, SELF(wndCalibrationDialog::onClose));
		listen(EVENT_SOLUTION_FOUND, SELF(wndCalibrationDialog::onSolution));
		listen(EVENT_ON_ABORT, SELF(wndCalibrationDialog::onAbort));
		listen(EVENT_ON_RESET, SELF(wndCalibrationDialog::onReset));

		listen(EVENT_MIN_RANGE, SELF(wndCalibrationDialog::onMinRange));
		listen(EVENT_MAX_RANGE, SELF(wndCalibrationDialog::onMaxRange));
		listen(EVENT_MIN_SPAN, SELF(wndCalibrationDialog::onMinSpan));
		listen(EVENT_MAX_SPAN, SELF(wndCalibrationDialog::onMaxSpan));
		listen(EVENT_MIN_DISTORTION, SELF(wndCalibrationDialog::onMinDistort));
		listen(EVENT_MAX_DISTORTION, SELF(wndCalibrationDialog::onMaxDistort));

		// set calibration sources
		SendMessageA(getItemHandle(IDC_SOURCE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Neon");
		SendMessageA(getItemHandle(IDC_SOURCE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Mercury Argon");
		SendMessage(getItemHandle(IDC_SOURCE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		// set model types
		SendMessageA(getItemHandle(IDC_MODEL), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Linear");
		SendMessageA(getItemHandle(IDC_MODEL), CB_ADDSTRING, (WPARAM)0, (LPARAM)"Cubic");
		SendMessage(getItemHandle(IDC_MODEL), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		// sensitivity
		SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1, 100));
		SendMessage(getItemHandle(IDC_SENSITIVITY_SLIDER), TBM_SETTICFREQ, (WPARAM)10, (LPARAM)0);

		// sampling
		SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(5, 20));
		SendMessage(getItemHandle(IDC_SAMPLING_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		// progressbar
		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 1000));
		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETPOS, (WPARAM)0, (LPARAM)NULL);

		// set defaults
		setRangeMin(loadInt(KEY_MINRANGE, 500));
		setRangeMax(loadInt(KEY_MAXRANGE, 800));
		setSpanMin(loadInt(KEY_MINSPAN, 100));
		setSpanMax(loadInt(KEY_MAXSPAN, 150));
		setDistortionMin(loadInt(KEY_MINDIST, 0));
		setDistortionMax(loadInt(KEY_MAXDIST, 10));
		setModelType(loadString(KEY_MODELTYPE, "Cubic"));
		setSourceType(Source::Neon);
		setSampling(loadInt(KEY_SAMPLING, 10));
		setSensitivity(0.80);

		// detect peaks
		updatePeaks();

		// do NOT display window
		show(false);

		// notify init is done
		notify(EVENT_INIT_DONE);
	}

	// enable all
	void enableAll(bool bEnable)
	{
		enableSourceType(bEnable);
		enableNumPeaks(bEnable);
		enableSensitivity(bEnable);
		enableParametersGroup(bEnable);
		enableProgress(bEnable);
		enableStatus(bEnable);
		enableCalibrate(bEnable);
		enableAbort(bEnable);
		enableReset(bEnable);
		enableLoad(bEnable);
		enableSave(bEnable);
	}

private:

	// handle messages
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		case WM_TIMER:
			onUpdate();
			break;

		case WM_CLOSE:
			notify(EVENT_CLOSE);
			
			// do NOT apply default mechanism on close
			return TRUE;

		case WM_HSCROLL:
			if ((HWND)lParam == getItemHandle(IDC_NUMPEAKS_SLIDER))
				notify(EVENT_NUMPEAKS);
			else if ((HWND)lParam == getItemHandle(IDC_SENSITIVITY_SLIDER))
				notify(EVENT_PEAK_SENSITIVITY);
			else if ((HWND)lParam == getItemHandle(IDC_SAMPLING_SLIDER))
				notify(EVENT_SAMPLING);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CALIBRATE:
				if(HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_ON_CALIBRATE);
				break;

			case IDC_ABORT:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_ON_ABORT);
				break;

			case IDC_RESET:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_ON_RESET);
				break;

			case IDC_IMPORT_CALIBRATION:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_ON_IMPORT);
				break;

			case IDC_UPLOAD_CALIBRATION:
				if (HIWORD(wParam) == BN_CLICKED)
					notify(EVENT_ON_UPLOAD);
				break;

			case IDC_SOURCE:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					notify(EVENT_SOURCE_TYPE);
				break;

			case IDC_MODEL:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					notify(EVENT_MODEL_TYPE);
				break;

			case IDC_MIN_WAVELENGTH:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MIN_RANGE);
				break;

			case IDC_MAX_WAVELENGTH:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MAX_RANGE);
				break;

			case IDC_MIN_SPAN:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MIN_SPAN);
				break;

			case IDC_MAX_SPAN:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MAX_SPAN);
				break;

			case IDC_MIN_DISTORTION:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MIN_DISTORTION);
				break;

			case IDC_MAX_DISTORTION:
				if (HIWORD(wParam) == EN_CHANGE)
					notify(EVENT_MAX_DISTORTION);
				break;
			}
			break;
		}

		return FALSE;
	}

	// called after every notifications
	virtual void onNotify(int iEvent) override
	{
		// update enable state
		enableAll(true);
	}

	// action on model change
	void onModelChange(void)
	{
		// clear optimization object if any
		this->m_pOptimizationThread.reset();

		// save to registry
		switch (getModelType())
		{
		case Model::Linear:
			saveString(KEY_MODELTYPE, "Linear");
			break;

		case Model::Cubic:
			saveString(KEY_MODELTYPE, "Cubic");
			break;

		default:
			saveString(KEY_MODELTYPE, "???");
			break;
		}
	}

	// action on source change
	void onSourceChange(void)
	{
		// clear detected peaks
		this->m_detectedPeaks.clear();

		// get minimum number of peaks
		size_t nMinPeaks = 0;

		switch (getModelType())
		{
		case Model::Linear:
			nMinPeaks = 2;
			break;

		case Model::Cubic:
			nMinPeaks = 3;
			break;

		// throw exception for unknown model
		default:
			throw UnknownModelException();
		}

		// get maximum number of peaks
		size_t nMaxPeaks = getCalibrationData().size();

		// resize sliders
		SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(nMinPeaks, nMaxPeaks));
		SendMessage(getItemHandle(IDC_NUMPEAKS_SLIDER), TBM_SETTICFREQ, (WPARAM)1, (LPARAM)0);

		// set number of peaks to maximum which will also trigger peak detection
		setNumPeaks(nMaxPeaks);
	}

	// update number of peaks edit
	void onNumPeaksChange(void)
	{
		// get number of peaks
		auto num_peaks = getNumPeaks();

		char szTmp[64];

		sprintf_s(szTmp, "%zu", num_peaks);

		// update field
		SetDlgItemTextA(getWindowHandle(), IDC_SZ_NUMPEAKS_VAL, szTmp);
	}

	// update sensitivity edit
	void onSensitivityChange(void)
	{
		// get number of peaks
		double fSensitivity = getSensitivity();

		char szTmp[64];

		sprintf_s(szTmp, "%.0f%%", 100.0 * fSensitivity);

		// update field
		SetDlgItemTextA(getWindowHandle(), IDC_SZ_SENSITIVITY_VAL, szTmp);
	}

	// update sampling edit
	void onSamplingChange(void)
	{
		// get sampling
		int iSampling = getSampling();

		char szTmp[64];

		sprintf_s(szTmp, "%d", iSampling);

		// update field
		SetDlgItemTextA(getWindowHandle(), IDC_SZ_SAMPLING_VAL, szTmp);

		// save to registry
		saveInt(KEY_SAMPLING, iSampling);
	}

	// min range action
	void onMinRange(void)
	{
		// save to registry
		saveInt(KEY_MINRANGE, (int)getRangeMin());
	}

	// max range action
	void onMaxRange(void)
	{
		// save to registry
		saveInt(KEY_MAXRANGE, (int)getRangeMax());
	}

	// min span action
	void onMinSpan(void)
	{
		// save to registry
		saveInt(KEY_MINSPAN, (int)getSpanMin());
	}

	// max span action
	void onMaxSpan(void)
	{
		// save to registry
		saveInt(KEY_MAXSPAN, (int)getSpanMax());
	}

	// min distort action
	void onMinDistort(void)
	{
		// save to registry
		saveInt(KEY_MINDIST, (int)getDistortionMin());
	}

	// max distort action
	void onMaxDistort(void)
	{
		// save to registry
		saveInt(KEY_MAXDIST, (int)getDistortionMax());
	}

	// update peak detection
	void onDetectPeaks(void)
	{
		// skip if no plot data
		if (!hasPlotData(0))
			return;

		// retrieve number of peaks
		auto num_peaks = getNumPeaks();
		double fSensitivity = getSensitivity();

		// get plot data
		auto plot_data = getPlotData(0);

		if (plot_data.x.size() == 0 || plot_data.y.size() == 0)
			return;

		auto x = linspace(-1, 1, plot_data.y.size());

		// get calibration data
		auto calibration_data = getCalibrationData();

		// find peaks
		auto peaks = refinePeaksPosition(x, plot_data.y, findPeaks(plot_data.y, num_peaks, fSensitivity));

		// clear previous annotations
		clearAnnotations();

		// create annotation
		guiSignal series;

		series.line.type = guiPen::Type::None;

		series.markers.type = guiMarker::Type::Triangle;
		series.markers.border.color = RGB(0, 0, 0);
		series.markers.fill.color = RGB(255, 64, 64);

		series.pHorizontalAxis = getPrimaryHorizontalAxis();
		series.pVerticalAxis = getPrimaryVerticalAxis();

		series.y = peaks.y;
		series.x = linterp(0.5 * (peaks.x + 1.0) * (double)(plot_data.y.size() - 1), plot_data.x);

		series.render_enable = true;

		// register annotation
		this->m_nAnnotationIndex = addAnnotation(series);

		// save detected peaks
		this->m_detectedPeaks = peaks.x;

		// label peaks
		labelPeaks();

		// update RMS field
		updateRMS();

		// send update message
		notify(EVENT_UPDATE);
	}

	// action on load
	void onLoad(void)
	{

	}

	// action on save
	void onSave(void)
	{

	}

	// abort
	void abort(void)
	{
		// kill thread if existing
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			this->m_pOptimizationThread->stop();
	}

	// abort action
	void onAbort(void)
	{
		// stop thread if it is running (abort mode)
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			abort();

		updatePeaks();
	}

	// reset action
	void onReset(void)
	{
		// clear data
		clear();

		// update progress bar
		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 1000));
		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETPOS, (WPARAM)0, (LPARAM)NULL);
	}

	// action on calibrate
	void onCalibrate(void)
	{
		// skip if no peaks are detected
		if (this->m_detectedPeaks.size() == 0)
			return;

		// get parameters
		double fMinRange = getRangeMin();
		double fMaxRange = getRangeMax();

		double fMinSpan = getSpanMin();
		double fMaxSpan = getSpanMax();

		double fMinDistortion = getDistortionMin();
		double fMaxDistortion = getDistortionMax();

		size_t nNumSamples = getSampling();

		auto calibration_data = getCalibrationData();

		// create global optimization object
		switch (getModelType())
		{
		case Model::Linear:
			this->m_pOptimizationThread = std::make_shared<LinearModelThread>(this->m_detectedPeaks, calibration_data, fMinRange, fMaxRange, fMinSpan, fMaxSpan, nNumSamples);
			break;

		case Model::Cubic:
			this->m_pOptimizationThread = std::make_shared<CubicModelThread>(this->m_detectedPeaks, calibration_data, fMinRange, fMaxRange, fMinSpan, fMaxSpan, fMinDistortion, fMaxDistortion, nNumSamples);
			break;

			// throw exception for unknown model
		default:
			throw UnknownModelException();
		}

		// skip if no thread created
		if (this->m_pOptimizationThread == nullptr)
			return;

		// start thread
		this->m_pOptimizationThread->start();

		// create update timer every 100 ms
		SetTimer(getWindowHandle(), 1, 100, NULL);
	}

	// action on quit
	void onClose(void)
	{
		// abort
		abort();

		// clear current annotation
		clearAnnotations();

		// dismiss window
		show(false);
	}

	// enable algorithm parameters group
	void enableParametersGroup(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_MIN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_MAX), bEnable ? TRUE : FALSE);

		enableModelType(bEnable);
		enableSampling(bEnable);
		enableRange(bEnable);
		enableSpan(bEnable);
		enableDistortion(bEnable);
	}

	// enable source type components
	void enableSourceType(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_SOURCE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SOURCE), bEnable ? TRUE : FALSE);
	}

	// enable num peaks components
	void enableNumPeaks(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_NUMPEAKS), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_NUMPEAKS_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_NUMPEAKS_VAL), bEnable ? TRUE : FALSE);
	}

	// enable sensitivity components
	void enableSensitivity(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_SENSITIVITY), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SENSITIVITY_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_SENSITIVITY_VAL), bEnable ? TRUE : FALSE);
	}

	// enable model type components
	void enableModelType(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_MODEL), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MODEL), bEnable ? TRUE : FALSE);
	}

	// enable sampling components
	void enableSampling(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_SAMPLING), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SAMPLING_SLIDER), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_SAMPLING_VAL), bEnable ? TRUE : FALSE);
	}

	// enable range components
	void enableRange(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_WAVELENGTH), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MIN_WAVELENGTH), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_WAVELENGTH_MIN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MAX_WAVELENGTH), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_WAVELENGTH_MAX), bEnable ? TRUE : FALSE);
	}

	// enable span components
	void enableSpan(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_SPAN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MIN_SPAN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_SPAN_MIN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MAX_SPAN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_SPAN_MAX), bEnable ? TRUE : FALSE);
	}

	// enable distortion components
	void enableDistortion(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		// disable field for linear model
		if (getModelType() == Model::Linear)
			bEnable = false;

		EnableWindow(getItemHandle(IDC_SZ_DISTORTION), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MIN_DISTORTION), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_DISTORTION_MIN), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MAX_DISTORTION), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_SZ_DISTORTION_MAX), bEnable ? TRUE : FALSE);
	}

	// enable progress components
	void enableProgress(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_CALIBRATION_PROGRESS), bEnable ? TRUE : FALSE);
	}

	// enable status components
	void enableStatus(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_SZ_CALIBRATION_STATUS), bEnable ? TRUE : FALSE);
	}

	// enable calibrate components
	void enableCalibrate(bool bEnable)
	{
		// cannot calibrate if no peaks detected
		bEnable &= this->m_detectedPeaks.size() > 0;
		bEnable &= (this->m_pOptimizationThread == nullptr || !this->m_pOptimizationThread->isRunning());

		EnableWindow(getItemHandle(IDC_CALIBRATE), bEnable ? TRUE : FALSE);
	}

	// enable calibrate components
	void enableAbort(bool bEnable)
	{
		// cannot calibrate if not processing
		bEnable &= (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning());

		EnableWindow(getItemHandle(IDC_ABORT), bEnable ? TRUE : FALSE);
	}

	// enable calibrate components
	void enableReset(bool bEnable)
	{
		// cannot reset if no solution
		bEnable &= hasSolution();
		bEnable &= (this->m_pOptimizationThread == nullptr || !this->m_pOptimizationThread->isRunning());

		EnableWindow(getItemHandle(IDC_RESET), bEnable ? TRUE : FALSE);
	}

	// enable load components
	void enableLoad(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		EnableWindow(getItemHandle(IDC_IMPORT_CALIBRATION), bEnable ? TRUE : FALSE);
	}

	// enable save components
	void enableSave(bool bEnable)
	{
		// disable if process is running
		if (this->m_pOptimizationThread != nullptr && this->m_pOptimizationThread->isRunning())
			bEnable = false;

		bEnable &= hasSolution();
		bEnable &= hasCamera();

		EnableWindow(getItemHandle(IDC_UPLOAD_CALIBRATION), bEnable ? TRUE : FALSE);
	}

	// update progressbar
	void onUpdate(void)
	{
		// skip if no optimization thread
		if (this->m_pOptimizationThread == nullptr)
			return;

		// update progressbar
		auto progress = (int)floor(1000.0 * (double)this->m_pOptimizationThread->getProcessedSamples() / (double)(this->m_pOptimizationThread->getMaxSamples() - 1));

		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 1000));
		SendMessage(getItemHandle(IDC_CALIBRATION_PROGRESS), PBM_SETPOS, (WPARAM)progress, (LPARAM)NULL);

		// if no solution yet, display progress in text area
		if (this->m_pOptimizationThread->isRunning())
		{
			char szTmp[256];

			sprintf_s(szTmp, "%zu/%zu", this->m_pOptimizationThread->getProcessedSamples(), this->m_pOptimizationThread->getMaxSamples());

			SetDlgItemTextA(getWindowHandle(), IDC_SZ_CALIBRATION_STATUS, szTmp);
		}
		// otherelse display RMS error and delete timer
		else
		{
			// notify a solution has been found
			notify(EVENT_SOLUTION_FOUND);
		}
	}

	// update RMS field
	void updateRMS(void)
	{
		// skip if no solution
		if (!hasSolution())
		{
			SetDlgItemTextA(getWindowHandle(), IDC_SZ_CALIBRATION_STATUS, "NO CALIBRATION DATA");

			return;
		}

		// get solution
		auto coeffs = getSolution();

		// display model + rms error
		char szTmp[256];

		sprintf_s(szTmp, "RMS: %.3f nm", getCalibrationModelRMS(coeffs, this->m_detectedPeaks, getCalibrationData()));

		SetDlgItemTextA(getWindowHandle(), IDC_SZ_CALIBRATION_STATUS, szTmp);
	}

	// apply this when solution has been found
	void onSolution(void)
	{
		// kill timer
		KillTimer(getWindowHandle(), 1);

		// update RMS
		updateRMS();

		// label peaks
		labelPeaks();

		// update
		notify(EVENT_UPDATE);
	}

	// label peaks
	void labelPeaks(void)
	{
		// skip if no solution
		if (!hasSolution())
			return;

		// get solution
		auto coeffs = getSolution();

		// add label to series
		for (auto& v : this->m_detectedPeaks)
		{
			char szTmp[64];
			sprintf_s(szTmp, "%.3f nm", index2wavelength(coeffs, v));

			guiSignal::DataLabel label;

			label.text = std::string(szTmp);
			label.font.bold = false;
			label.font.size = 15;
			label.offset_x = 0;
			label.offset_y = -15;

			getAnnotation(this->m_nAnnotationIndex).labels.emplace_back(std::move(label));
		}
	}

	// return true if solution exists
	bool hasSolution(void) const
	{
		if (this->m_pOptimizationThread == nullptr)
			return false;

		return this->m_pOptimizationThread->hasSolution();
	}

	// members
	std::shared_ptr<IGlobalOptimizationThread> m_pOptimizationThread;
	vector_t m_detectedPeaks;

	size_t m_nAnnotationIndex;
};