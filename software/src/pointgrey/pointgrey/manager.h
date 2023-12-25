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

#include <memory>
#include <string>

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#include "shared/utils/exception.h"
#include "shared/utils/evemon.h"
#include "shared/camera/camera.h"

#include "exception.h"
#include "camera.h"

 // Interface to access PtGrey cameras
class PtGreyCameraInterface : public ICameraInterface
{
public:
	// constructor
	PtGreyCameraInterface(void)
	{
		this->m_pSystem = nullptr;
	}

	// destructor
	~PtGreyCameraInterface(void)
	{
		// destructor should never trigger exceptions
		try
		{
			// clear all camera handles
			this->m_pCamerasHandles.clear();

			// release system instance
			if (this->m_pSystem != nullptr && this->m_pSystem.IsValid())
				this->m_pSystem->ReleaseInstance();
		}
		catch (...) {}
	}

	// check if camera exists
	virtual bool hasCamera(const std::string& rLabel) const override
	{
		return this->m_pCamerasHandles.find(rLabel) != this->m_pCamerasHandles.end();
	}

	// get camera by name
	virtual std::shared_ptr<ICamera> getCameraByName(const std::string& rLabel) const override
	{
		// check in map
		auto it = this->m_pCamerasHandles.find(rLabel);

		// throw error if not found
		if (it == this->m_pCamerasHandles.end())
			throwException(CameraNotFoundException, rLabel);

		// otherelse return pointer
		return it->second;
	}

	// refresh camera list
	virtual std::vector<std::string> listCameras(void) override
	{
		std::vector<std::string> ret;

		do
		{
			// get system handle
			if (this->m_pSystem == nullptr)
				this->m_pSystem = Spinnaker::System::GetInstance();

			// clear previous lists
			this->m_pCamerasHandles.clear();

			// check if valid
			if (!this->m_pSystem.IsValid())
				break;

			// recreate list
			Spinnaker::CameraList pCamerasList = this->m_pSystem->GetCameras();

			try
			{
				// browse list
				for (unsigned int i = 0; i < pCamerasList.GetSize(); i++)
				{
					Spinnaker::CameraPtr pCamera = pCamerasList.GetByIndex(i);

					if (pCamera == nullptr)
						continue;

					// create proxy object
					std::shared_ptr<PtGreyCamera> pCameraObject = std::make_shared<PtGreyCamera>(pCamera);

					// get properties
					try
					{
						if (pCameraObject != nullptr)
						{
							// open camera
							pCameraObject->open();

							// get name
							std::string label = pCameraObject->uid();

							// add to list
							ret.emplace_back(label);

							// close camera
							pCameraObject->close();

							// add to list
							this->m_pCamerasHandles.emplace(std::make_pair(label, pCameraObject));
						}
					}
					catch (...)
					{
						// skip in case of failure
						continue;
					}
				}

				// clear list
				pCamerasList.Clear();

				return ret;
			}
			catch (...)
			{
				// clear list before rethrowing
				pCamerasList.Clear();

				throw;
			}
		} while (false);

		// throw exception
		throwException(InitException);
	}

private:
	// members
	Spinnaker::SystemPtr m_pSystem;

	std::map<std::string, std::shared_ptr<PtGreyCamera>> m_pCamerasHandles;
};