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

#include <mutex>
#include <atomic>
#include <functional>

#include <Windows.h>
#include <CommCtrl.h>

#include "shared/utils/utils.h"
#include "shared/utils/thread.h"
#include "shared/utils/event.h"
#include "shared/utils/safe.h"
#include "shared/utils/evemon.h"
#include "shared/math/map.h"
#include "shared/camera/camera.h"
#include "shared/gui/dialogs.h"

#include "state.h"
#include "camdata.h"
#include "winmain.h"

#include "resource.h"

// AcquisitionThread class
class AcquisitionThread : public IThread
{
public:
	// constructor
	AcquisitionThread()
	{
		// start waiting
		start();
	}

	// request a new acquisition
	void acquire(std::shared_ptr<ICamera> pCamera)
	{
		// set camera
		atomic_store(&this->m_pCamera, pCamera);

		// clear current image if not retrieved
		AUTOLOCK(this->m_mutex);

		this->m_currImage.clear();
		this->m_done.reset();

		// triger cammera
		try
		{
			if (this->m_pCamera != nullptr)
				this->m_pCamera->trigger();
		}
		catch (...) {}

		// trigger request event
		this->m_request.trigger();
	}

	// get last image
	image_t get(void)
	{
		// wait for image
		this->m_done.wait();

		// no data is available
		this->m_done.reset();

		// lock and move image
		image_t img;

		{
			AUTOLOCK(this->m_mutex);

			img = std::move(this->m_currImage);

			this->m_currImage.clear();
		}

		return img;
	}

	// wait until acquisition is done
	void waitImage(void)
	{
		// wait for image
		this->m_done.wait();
	}

	// return true if image is available
	bool hasData(void) const
	{
		AUTOLOCK(this->m_mutex);

		return !this->m_done.isPending() && this->m_currImage.isValid();
	}

	// return true if system is grabbing an image
	bool isGrabbing(void) const
	{
		return !this->m_request.isPending();
	}

protected:

	// thread loop
	virtual void run(void)
	{
		// poll event to check for image request
		if (!this->m_request.wait(0.01))
			return;

		// get next image
		size_t nNumTrials = 10;

		while (nNumTrials--)
		{
			try
			{
				// get image
				image_t img = this->m_pCamera->acquireImage();

				if (img.isValid())
				{
					// copy image
					{
						AUTOLOCK(this->m_mutex);

						this->m_currImage = img;
					}

					// go to next
					break;
				}
			}
			catch (...) {}
		}

		// trigger done event
		this->m_done.trigger();

		// accept new requests
		this->m_request.reset();
	}

private:
	std::shared_ptr<ICamera> m_pCamera;

	mutable std::mutex m_mutex;

	image_t m_currImage;

	Event m_request, m_done;
};

// acquisition dialog class
class wndIAcquisitionDialog : public IDialog, public SpectrumAnalyzerChild
{
public:

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_ACQUISITION;

	enum
	{
		EVENT_CLOSE = 0,
		EVENT_UPDATE,
		EVENT_ABORT,
	} events;

	// hide on initialization
	virtual void init(void) override
	{
		_debug("wndIAcquisitionDialog::init");

		// listen to actions
		listen(EVENT_CLOSE, SELF(wndIAcquisitionDialog::onClose));
		listen(EVENT_ABORT, SELF(wndIAcquisitionDialog::onAbort));

		show(false);

		this->m_iImagesAcquired = 0;
		this->m_iTotalImages = 0;
	}

	// start acquisition
	void acquire(std::shared_ptr<ICamera> pCamera, int iNumData)
	{
		_debug("starting acquisition of %d images", iNumData);

		// set camera
		this->m_pCamera = pCamera;

		// create data display object
		this->m_pDataBuilder = std::make_shared<CameraDataBuilder>(pCamera != nullptr ? pCamera->uid() : "");

		// set progress bar data
		this->m_iImagesAcquired = 0;
		this->m_iTotalImages = max(1, iNumData);

		SendMessage(getItemHandle(IDC_PROGRESS), PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, this->m_iTotalImages));

		onUpdate(true);

		// start acquisition
		try
		{
			this->m_pCamera->beginAcquisition();
		}
		catch (IException& rException)
		{
			_error("%s", rException.toString().c_str());

			MessageBoxA(getWindowHandle(), rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);

			SendMessage(getWindowHandle(), WM_CLOSE, (WPARAM)0, (LPARAM)0);

			return;
		}
		catch (...)
		{
			_error("Failed to start acquisition!");

			MessageBoxA(getWindowHandle(), "Failed to start acquisition!", "error", MB_ICONHAND | MB_OK);

			SendMessage(getWindowHandle(), WM_CLOSE, (WPARAM)0, (LPARAM)0);

			return;
		}

		// create timer
		SetTimer(getWindowHandle(), 1, 20, NULL);
	}

	// hide on close
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		// abort on close
		case WM_CLOSE:
			// callback
			notify(EVENT_CLOSE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			// close dialog on abort
			case IDC_ABORT:
				notify(EVENT_ABORT);
				break;
			}
			break;

		// update
		case WM_TIMER:
			onUpdate(false);
			break;
		}

		return FALSE;
	}

protected:
	virtual void onImageDone(void) = 0;

	// reset counter
	void reset(void)
	{
		this->m_iImagesAcquired = 0;

		SendMessage(getItemHandle(IDC_PROGRESS), PBM_SETPOS, (WPARAM)this->m_iImagesAcquired, (LPARAM)NULL);
	}

private:
	void onAbort(void)
	{
		_debug("aborting acquisition");

		close();
	}

	void onClose(void)
	{
		_debug("closing acquisition");

		// remove timer
		KillTimer(getWindowHandle(), 1);

		// stop thread and wait for grabbing to be finished
		this->m_acqThread.stop();

		// end acquisition
		this->m_pCamera->endAcquisition();

		// disable window
		show(false);
	}

	void onUpdate(bool bForceUpdate)
	{
		bool bUpdate = bForceUpdate;

		// check for image
		if (this->m_acqThread.hasData())
		{
			// get image
			auto img = this->m_acqThread.get();

			if (img.isValid())
			{
				_debug("acquired new image!");

				// increment number of acquired image
				this->m_iImagesAcquired++;

				bUpdate = true;

				// process image
				process(img);
			}
			else
				_warning("image is invalid");
		}

		// update position
		if (bUpdate)
		{
			SendMessage(getItemHandle(IDC_PROGRESS), PBM_SETPOS, (WPARAM)this->m_iImagesAcquired, (LPARAM)NULL);

			// update text of dialog
			char szTmp[256];

			sprintf_s(szTmp, "Image %d/%d", this->m_iImagesAcquired, this->m_iTotalImages);

			SetDlgItemTextA(getWindowHandle(), IDC_SZ_PROGRESS, szTmp);
		}

		// close if enough images
		if (this->m_iImagesAcquired >= this->m_iTotalImages)
			onImageDone();

		// otherelse request new image
		else if (!this->m_acqThread.isGrabbing())
			this->m_acqThread.acquire(this->m_pCamera);
	}

	void process(const image_t& image)
	{
		// skip if no data display object
		if (this->m_pDataBuilder == nullptr)
			return;

		try
		{
			// retrieve parameters
			bool bMedFilt = isMedFiltEnabled();
			auto exposure = getExposure();
			auto gain = getGain();

			double fScale = 1.0 / (exposure * gain);

			auto img = image;

			// median filtering (if required)
			if (bMedFilt)
				img = medfilt2(img);

			// clear data when if first image of serie
			if (this->m_iImagesAcquired == 1)
				this->m_pDataBuilder->clear();

			// add to accumulator
			this->m_pDataBuilder->addSignalData(fScale * sum_cols(img));
			this->m_pDataBuilder->addSaturationData(max_cols(img));
			this->m_pDataBuilder->addROIData(max_rows(img));

			// set current data display method
			setPlotBuilder(this->m_pDataBuilder);

			// callback
			notify(EVENT_UPDATE);
		}
		catch (...) {}
	}

	AcquisitionThread m_acqThread;

	std::shared_ptr<CameraDataBuilder> m_pDataBuilder;
	std::shared_ptr<ICamera> m_pCamera;

	int m_iTotalImages, m_iImagesAcquired;
};

// single image acquisition
class wndSingleImageAcquisitionDialog : public wndIAcquisitionDialog
{
public:
	using wndIAcquisitionDialog::wndIAcquisitionDialog;

protected:
	virtual void onImageDone(void)
	{
		close();
	}
};

// single image acquisition
class wndMultipleImageAcquisitionDialog : public wndIAcquisitionDialog
{
public:
	using wndIAcquisitionDialog::wndIAcquisitionDialog;

protected:
	virtual void onImageDone(void)
	{
		reset();
	}
};