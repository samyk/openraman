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

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#include <mutex>
#include <memory>
#include <string>
#include <functional>

#include "shared/utils/utils.h"
#include "shared/utils/exception.h"
#include "shared/math/map.h"

#include "shared/camera/camera.h"

#include "defs.h"
#include "exception.h"

#undef ENABLE_TRIGGER

// Wrapper for important PtGrey camera functions
class PtGreyCamera : public ICamera
{
public:
	// default constructor
	PtGreyCamera(Spinnaker::CameraPtr pCamera)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// init
		this->m_bAcquiring = false;

		// set camera handle
		this->m_pCamera = pCamera;

		// bind all properties
		AnalogControl.gain_auto = EnumProperty(this, RootNodeMap::Camera, { "AnalogControl", "GainAuto" });
		AnalogControl.gain = FloatProperty(this, RootNodeMap::Camera, { "AnalogControl", "Gain" });
		AnalogControl.blacklevel_enable = BooleanProperty(this, RootNodeMap::Camera, { "AnalogControl", "BlackLevelClampingEnable" });
		AnalogControl.gamma_enable = BooleanProperty(this, RootNodeMap::Camera, { "AnalogControl", "GammaEnabled" });
		AnalogControl.sharpness_enable = BooleanProperty(this, RootNodeMap::Camera, { "AnalogControl", "SharpnessEnabled" });
		AnalogControl.hue_enable = BooleanProperty(this, RootNodeMap::Camera, { "AnalogControl", "HueEnabled" });
		AnalogControl.saturation_enable = BooleanProperty(this, RootNodeMap::Camera, { "AnalogControl", "SaturationEnabled" });

		AcquisitionControl.trigger_mode = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "TriggerMode" });
		AcquisitionControl.trigger_source = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "TriggerSource" });
		AcquisitionControl.trigger_selector = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "TriggerSelector" });
		AcquisitionControl.trigger_cmd = CommandProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "TriggerSoftware" });
		AcquisitionControl.exposure_mode = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "ExposureMode" });
		AcquisitionControl.exposure_auto = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "ExposureAuto" });
		AcquisitionControl.exposure_time = FloatProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "ExposureTime" });
		AcquisitionControl.acquisition_mode = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "AcquisitionMode" });
		AcquisitionControl.framerate_mode = EnumProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "AcquisitionFrameRateAuto" });
		AcquisitionControl.hdr_enable = BooleanProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "pgrHDRModeEnabled" });
		AcquisitionControl.start = CommandProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "Start" });
		AcquisitionControl.stop = CommandProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "Stop" });

		// this one is a tricky one...
		AcquisitionControl.framerate_enabled = BooleanProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "AcquisitionFrameRateEnabled" });
		AcquisitionControl.framerate_enable = BooleanProperty(this, RootNodeMap::Camera, { "AcquisitionControl", "AcquisitionFrameRateEnable" });

		ImageFormatControl.pixel_format = EnumProperty(this, RootNodeMap::Camera, { "ImageFormatControl",  "PixelFormat" });
		ImageFormatControl.bigendian = BooleanProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "pgrPixelBigEndian" });
		ImageFormatControl.colorprocessing = BooleanProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "OnBoardColorProcessEnabled" });
		ImageFormatControl.videomode = EnumProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "VideoMode" });
		ImageFormatControl.reversex = BooleanProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "ReverseX" });
		ImageFormatControl.widthmax = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "WidthMax" });
		ImageFormatControl.heightmax = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "HeightMax" });
		ImageFormatControl.ofsx = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "OffsetX" });
		ImageFormatControl.ofsy = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "OffsetY" });
		ImageFormatControl.width = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "Width" });
		ImageFormatControl.height = IntegerProperty(this, RootNodeMap::Camera, { "ImageFormatControl", "Height" });

		UserDefinedValues.selector = EnumProperty(this, RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValueSelector" });
		UserDefinedValues.value = IntegerProperty(this, RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValue" });

		UserSetControl.selector = EnumProperty(this, RootNodeMap::Camera, { "UserSetControl", "UserSetSelector" });
		UserSetControl.load = CommandProperty(this, RootNodeMap::Camera, { "UserSetControl", "UserSetLoad" });
		UserSetControl.save = CommandProperty(this, RootNodeMap::Camera, { "UserSetControl", "UserSetSave" });

		DeviceInformation.vendor = StringProperty(this, RootNodeMap::TransportLayer, { "DeviceInformation", "DeviceVendorName" });
		DeviceInformation.name = StringProperty(this, RootNodeMap::TransportLayer, { "DeviceInformation", "DeviceModelName" });
		DeviceInformation.serialnumber = StringProperty(this, RootNodeMap::TransportLayer, { "DeviceInformation", "DeviceSerialNumber" });
		DeviceInformation.forceip = CommandProperty(this, RootNodeMap::TransportLayer, { "DeviceInformation", "GevDeviceAutoForceIP" });
		DeviceInformation.wrong_subnet = BooleanProperty(this, RootNodeMap::TransportLayer, { "DeviceInformation", "GevDeviceIsWrongSubnet" });

		BufferHandlingControl.stream_mode = EnumProperty(this, RootNodeMap::TLStream, { "BufferHandlingControl", "StreamBufferHandlingMode" });
	}

	// default destructor
	~PtGreyCamera(void)
	{
		// destructor should never trigger exceptions
		try
		{
			// close camera on exit
			close();
		}
		catch (...) {}
	}

	// return UID of camera
	virtual std::string uid(void) const override
	{
		return DeviceInformation.vendor.get() + std::string(" ") + DeviceInformation.name.get() + std::string(" (S/N ") + DeviceInformation.serialnumber.get() + std::string(")");
	}

	// load state
	virtual void load(void) override
	{
		// load from "User Set 2"
		UserSetControl.selector = "UserSet2";
		UserSetControl.load();
	}

	// save state
	virtual void save(void) override
	{
		// save to "User Set 2"
		UserSetControl.selector = "UserSet2";
		UserSetControl.save();
	}

	// init camera
	virtual void init(void) override
	{
		// force ip if wrong subnet
		try
		{
			if (DeviceInformation.wrong_subnet)
			{
				DeviceInformation.forceip();

				if (DeviceInformation.wrong_subnet)
					throw WrongSubnetException();
			}
		}
		catch (...) {}

		// set parameters when possible
		NOTHROW(AnalogControl.gain_auto = "Off");
		NOTHROW(AnalogControl.gamma_enable = false);
		NOTHROW(AnalogControl.sharpness_enable = false);
		NOTHROW(AnalogControl.blacklevel_enable = false);
		NOTHROW(AnalogControl.hue_enable = false);
		NOTHROW(AnalogControl.saturation_enable = false);

		NOTHROW(AcquisitionControl.acquisition_mode = "Continuous");
		NOTHROW(AcquisitionControl.exposure_auto = "Off");
		NOTHROW(AcquisitionControl.exposure_mode = "Timed");
		NOTHROW(AcquisitionControl.framerate_enable = false);
		NOTHROW(AcquisitionControl.framerate_enabled = false);
		NOTHROW(AcquisitionControl.hdr_enable = false);
		NOTHROW(AcquisitionControl.framerate_mode = "Off");

		NOTHROW(ImageFormatControl.pixel_format = "Mono16");
		NOTHROW(ImageFormatControl.reversex = true);
		NOTHROW(ImageFormatControl.bigendian = true);
		NOTHROW(ImageFormatControl.colorprocessing = false);

		NOTHROW(BufferHandlingControl.stream_mode = "NewestOnly");

		// enable trigger mode if possible
#ifdef ENABLE_TRIGGER
		try
		{
			AcquisitionControl.trigger_selector = "FrameStart";
			AcquisitionControl.trigger_source = "Software";
			AcquisitionControl.trigger_mode = "On";
		}
		catch (...)
		{
			NOTHROW(AcquisitionControl.trigger_mode = "Off");
		}
#else
		NOTHROW(AcquisitionControl.trigger_mode = "Off");
#endif

		// assert that mandatory parameters are availables
		assert_node(AnalogControl.gain);
		assert_node(AcquisitionControl.exposure_time);
		assert_node(ImageFormatControl.ofsy);
		assert_node(ImageFormatControl.height);

		// TODO: this one throws unexpected error
		//assert_node(ImageFormatControl.heightmax);

		// check pixel format before acquisition
		if (ImageFormatControl.pixel_format.getString() != "Mono16")
			throw PixelFormatException(ImageFormatControl.pixel_format.getString(), "Mono16");
	}

	// open camera
	virtual void open(void) override
	{
		AUTOLOCK(this->m_mutex);

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		if (this->m_pCamera.IsValid() && !this->m_pCamera->IsInitialized())
			this->m_pCamera->Init();

		// disable heartbeart in debug mode
#ifdef _DEBUG
		try
		{
			disableHeartbeat();
		}
		catch (...) {}
#endif
	}

	// close camera
	virtual void close(void) override
	{
		AUTOLOCK(this->m_mutex);

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		if (this->m_pCamera.IsValid() && this->m_pCamera->IsInitialized())
			this->m_pCamera->DeInit();
	}

	using UserDataType = int32_t;

	// write user data
	virtual void setUserData(unsigned char* pData, size_t nSize) override
	{
		// get selector node
		auto pSelectorNode = getNode<Spinnaker::GenApi::CEnumerationPtr>(RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValueSelector" }, ACCESS_READ | ACCESS_WRITE);

		// get current entry
		auto pCurrEntry = pSelectorNode->GetCurrentEntry();

		try
		{
			// get number of entries
			Spinnaker::GenApi::StringList_t nodelist;

			pSelectorNode->GetSymbolics(nodelist);

			// check size
			if (nSize > sizeof(UserDataType) * nodelist.size())
				throw NotEnoughMemoryException(nSize, sizeof(UserDataType) * nodelist.size());

			// get data node
			auto pDataNode = getNode<Spinnaker::GenApi::CIntegerPtr>(RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValue" }, ACCESS_READ | ACCESS_WRITE);

			// check data node type size
			auto delta = pDataNode->GetMax() - pDataNode->GetMin();

			//if (delta < ((int64_t)1 << (sizeof(UserDataType) * 8)))
			if(delta < (int64_t)4294967295)
				throw WrongUserDataType();

			// browse until no bytes remains
			int iCurrSelector = 0;

			while (nSize > 0)
			{
				// set selector and increment next value
				pSelectorNode->SetIntValue(iCurrSelector++);

				UserDataType curr = 0;

				// whole word
				if (nSize >= sizeof(UserDataType))
				{
					memcpy(&curr, pData, sizeof(UserDataType));

					pData += sizeof(UserDataType);
					nSize -= sizeof(UserDataType);
				}
				// partial word
				else
				{
					memcpy(&curr, pData, nSize);

					nSize = 0;
				}

				// write value
				pDataNode->SetValue((int64_t)curr);
			}
		}
		catch (...)
		{
			// restore previous entry
			if (pCurrEntry != nullptr)
				pSelectorNode->SetIntValue(pCurrEntry->GetValue());

			throw;
		}

		// restore previous entry
		if(pCurrEntry != nullptr)
			pSelectorNode->SetIntValue(pCurrEntry->GetValue());
	}

	// read user data
	virtual void getUserData(unsigned char* pData, size_t nSize) const override
	{
		// get selector node
		auto pSelectorNode = getNode<Spinnaker::GenApi::CEnumerationPtr>(RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValueSelector" }, ACCESS_READ | ACCESS_WRITE);

		// get current entry
		auto pCurrEntry = pSelectorNode->GetCurrentEntry();

		try
		{
			// get number of entries
			Spinnaker::GenApi::StringList_t nodelist;

			pSelectorNode->GetSymbolics(nodelist);

			// check size
			if (nSize > sizeof(UserDataType) * nodelist.size())
			{
				// restore previous entry
				if (pCurrEntry != nullptr)
					pSelectorNode->SetIntValue(pCurrEntry->GetValue());

				throw NotEnoughMemoryException(nSize, sizeof(UserDataType) * nodelist.size());
			}

			// get data node
			auto pDataNode = getNode<Spinnaker::GenApi::CIntegerPtr>(RootNodeMap::Camera, { "UserDefinedValues", "UserDefinedValue" }, ACCESS_READ);

			// check data node type size
			auto delta = pDataNode->GetMax() - pDataNode->GetMin();

			//if (delta < ((int64_t)1 << (sizeof(UserDataType) * 8)))
			if (delta < (int64_t)4294967295)
				throw WrongUserDataType();

			// browse until no bytes remains
			int iCurrSelector = 0;

			while (nSize > 0)
			{
				// set selector and increment next value
				pSelectorNode->SetIntValue(iCurrSelector++);

				// get current value
				UserDataType curr = (UserDataType)pDataNode->GetValue();

				// whole word
				if (nSize >= sizeof(UserDataType))
				{
					memcpy(pData, &curr, sizeof(UserDataType));

					pData += sizeof(UserDataType);
					nSize -= sizeof(UserDataType);
				}
				// partial word
				else
				{
					memcpy(pData, &curr, nSize);

					nSize = 0;
				}
			}
		}
		catch (...)
		{
			// restore previous entry
			if (pCurrEntry != nullptr)
				pSelectorNode->SetIntValue(pCurrEntry->GetValue());

			throw;
		}

		// restore previous entry
		if (pCurrEntry != nullptr)
			pSelectorNode->SetIntValue(pCurrEntry->GetValue());
	}

	// generic set parameter
	virtual void setParam(const std::string& rKey, const std::string& rValue) override
	{
		throw NotImplementedException();
	}

	// generic get parameter
	virtual std::string getParam(const std::string& rKey) const override
	{
		throw NotImplementedException();
	}

	// start acquisition
	virtual void beginAcquisition(void) const override
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		// start acquisition
		try
		{
			this->m_pCamera->BeginAcquisition();

			this->m_bAcquiring = true;
		}
		catch (Spinnaker::Exception& rException)
		{
			MessageBoxA(NULL, rException.what(), "error", MB_ICONHAND | MB_OK);
		}
	}

	// stop acquisition
	virtual void endAcquisition(void) const override
	{
		if (!this->m_bAcquiring)
			return;

		// lock mutex
		AUTOLOCK(this->m_mutex);

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		// end acquisition
		this->m_pCamera->EndAcquisition();

		this->m_bAcquiring = false;
	}

	// trigger camera
	virtual void trigger(void) override
	{
#ifdef ENABLE_TRIGGER

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		// check trigger state
		try
		{
			if (this->AcquisitionControl.trigger_mode.getString() != "On" || this->AcquisitionControl.trigger_source.getString() != "Software")
				throw TriggerModeDisabledException();
		}
		catch (...)
		{
			throw TriggerModeDisabledException();
		}

		// send trigger command
		this->AcquisitionControl.trigger_cmd.run();
#endif
	}

	// acquire an image
	virtual image_t acquireImage(void) override
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// throw exception if no camera exists, should never happen
		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		// get next image
		Spinnaker::ImagePtr pImage = this->m_pCamera->GetNextImage();

		// skip if image is boggous
		if (pImage->IsIncomplete() || pImage->GetBitsPerPixel() != 16)
			throw ImageAcquisitionException();

		// acquire image
		size_t nWidth = pImage->GetWidth();
		size_t nHeight = pImage->GetHeight();
		size_t nStrideBytes = pImage->GetStride();

		// cast to words
		auto pPointer = (unsigned short*)pImage->GetData();

		size_t nStride = nStrideBytes / sizeof(unsigned short);

		// create image
		image_t img(nWidth, nHeight);

		for (size_t y = 0; y < nHeight; y++)
			for (size_t x = 0; x < nWidth; x++)
				img(x, y) = (double)myhtons(pPointer[x + y * nStride]) / 65535.0;

		// release image
		pImage->Release();

		// return object
		return img;
	}

	virtual void setExposure(double fExposureSecond) override
	{
		this->AcquisitionControl.exposure_time = 1e6 * fExposureSecond;
	}

	virtual double getExposure(void) const override
	{
		return this->AcquisitionControl.exposure_time * 1e-6;
	}

	virtual double getExposureMin(void) const override
	{
		return this->AcquisitionControl.exposure_time.getMin() * 1e-6;
	}

	virtual double getExposureMax(void) const override
	{
		return this->AcquisitionControl.exposure_time.getMax() * 1e-6;
	}

	virtual void setGain(double fGain) override
	{
		this->AnalogControl.gain = fGain;
	}

	virtual double getGain(void) const override
	{
		return this->AnalogControl.gain;
	}

	virtual double getGainMin(void) const override
	{
		return this->AnalogControl.gain.getMin();
	}

	virtual double getGainMax(void) const override
	{
		return this->AnalogControl.gain.getMax();
	}

	virtual void setROI(int iHeight) override
	{
		// check height
		if (iHeight > ImageFormatControl.heightmax || iHeight < ImageFormatControl.height.getMin())
			throw InvalidROIException(iHeight, (int)ImageFormatControl.height.getMin(), (int)ImageFormatControl.heightmax);

		// compute new offset
		int iNewOffset = ((int)this->ImageFormatControl.heightmax - iHeight) / 2;

		// reset offset first
		this->ImageFormatControl.ofsy = 0;

		// set height
		this->ImageFormatControl.height = iHeight;

		// compute new offset
		this->ImageFormatControl.ofsy = iNewOffset;
	}

	virtual int getROI(void) const override
	{
		return (int)this->ImageFormatControl.height;
	}

	virtual int getMinROI(void) const override
	{
		return (int)this->ImageFormatControl.height.getMin();
	}

	virtual int getMaxROI(void) const override
	{
		return (int)this->ImageFormatControl.heightmax;
	}

	// IProperty class
	class IProperty
	{
	public:

		// default constructor sets parent to null
		IProperty(void)
		{
			this->m_pParent = nullptr;
		}

		// constructor binds camera to actual node
		IProperty(PtGreyCamera* pParent, RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes)
		{
			this->m_pParent = pParent;
			this->m_eRootNodeMap = eRootNodeMap;
			this->m_nodes = rNodes;
		}

		// copy constructor
		IProperty(const IProperty& rProperty)
		{
			this->m_pParent = rProperty.m_pParent;
			this->m_nodes = rProperty.m_nodes;
			this->m_eRootNodeMap = rProperty.m_eRootNodeMap;
		}

		// move constructor
		IProperty(IProperty&& rProperty)
		{
			this->m_pParent = rProperty.m_pParent;
			this->m_nodes = std::move(rProperty.m_nodes);
			this->m_eRootNodeMap = rProperty.m_eRootNodeMap;

			rProperty.m_pParent = nullptr;
		}

		// unbind on destructor
		virtual ~IProperty(void)
		{
			this->m_pParent = nullptr;
		}

		// copy operator
		const IProperty& operator=(const IProperty& rProperty)
		{
			this->m_pParent = rProperty.m_pParent;
			this->m_nodes = rProperty.m_nodes;
			this->m_eRootNodeMap = rProperty.m_eRootNodeMap;

			return *this;
		}

		// move operator
		const IProperty& operator=(IProperty&& rProperty)
		{
			this->m_pParent = rProperty.m_pParent;
			this->m_nodes = std::move(rProperty.m_nodes);
			this->m_eRootNodeMap = rProperty.m_eRootNodeMap;

			rProperty.m_pParent = nullptr;

			return *this;
		}

		// return root nodemap
		RootNodeMap getRootNodeMap(void) const
		{
			return this->m_eRootNodeMap;
		}

		// return nodes list
		std::vector<std::string> getNodes(void) const
		{
			return this->m_nodes;
		}

		// return true if existing
		bool exists(void) const
		{
			assert_ptr();

			return this->m_pParent->exists(this->m_eRootNodeMap, this->m_nodes);
		}

	protected:
		// assert that the property is bound to a camera
		void assert_ptr(void) const
		{
			if (this->m_pParent == nullptr)
				throw NoCameraException();
		}

		// members
		RootNodeMap m_eRootNodeMap;
		std::vector<std::string> m_nodes;
		PtGreyCamera* m_pParent;
	};

	// TypedProperty class
	template<typename Type> class TypedProperty : public IProperty
	{
	public:

		// use all constructors from the base class
		using IProperty::IProperty;

		// assignment operator
		const TypedProperty<Type>& operator=(Type value)
		{
			set(value);

			return *this;
		}

		// generic set
		void set(Type value)
		{
			assert_ptr();

			this->m_pParent->set<Type>(this->m_eRootNodeMap, this->m_nodes, value);
		}

		// cast operator
		operator Type(void) const
		{
			return get();
		}

		// generic get
		Type get(void) const
		{
			assert_ptr();

			return this->m_pParent->get<Type>(this->m_eRootNodeMap, this->m_nodes);
		}
	};

	// NumberProperty class
	template<typename Type> class NumberProperty : public TypedProperty<Type>
	{
	public:
		// use all constructors from the base class
		using TypedProperty<Type>::TypedProperty;

		// assignment operator
		const NumberProperty<Type>& operator=(Type value)
		{
			TypedProperty<Type>::set(value);

			return *this;
		}

		// cast operator
		operator Type(void) const
		{
			return TypedProperty<Type>::get();
		}

		// return minimum of property
		Type getMin(void) const
		{
			IProperty::assert_ptr();

			return this->m_pParent->minval<Type>(this->m_eRootNodeMap, this->m_nodes);
		}

		// return maximum of property
		Type getMax(void) const
		{
			IProperty::assert_ptr();

			return this->m_pParent->maxval<Type>(this->m_eRootNodeMap, this->m_nodes);
		}
	};

	// all these properties are typed properties from the template above
	using BooleanProperty = TypedProperty<bool>;
	using IntegerProperty = NumberProperty<int64_t>;
	using FloatProperty = NumberProperty<double>;
	using StringProperty = TypedProperty<std::string>;

	// EnumProperty class
	class EnumProperty : public IProperty
	{
	public:

		// import all constructors from the base class
		using IProperty::IProperty;

		// assignment operator
		const EnumProperty& operator=(const std::string& value)
		{
			set(value);

			return *this;
		}

		// set function by string
		void set(const std::string& value)
		{
			assert_ptr();

			this->m_pParent->setEnumByString(this->m_eRootNodeMap, this->m_nodes, value);
		}

		// cast operator to string
		operator std::string(void) const
		{
			return getString();
		}

		// get operator to string
		std::string getString(void) const
		{
			assert_ptr();

			return this->m_pParent->getEnumString(this->m_eRootNodeMap, this->m_nodes);
		}
	};

	// CommandProperty class
	class CommandProperty : public IProperty
	{
	public:

		// import all constructors from the base class
		using IProperty::IProperty;

		// run command
		void run(void)
		{
			assert_ptr();

			this->m_pParent->run(this->m_eRootNodeMap, this->m_nodes);
		}

		// () operator
		void operator()(void)
		{
			run();
		}
	};

	// generic get template
	template<typename Type> Type get(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		static_assert(false, "Type not implemented!");
	}

	// return true if existing
	bool exists(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		try
		{
			auto pNode = getNode(eRootNodeMap, rNodes, 0);

			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	// generic set template
	template<typename Type> void set(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const Type& value)
	{
		static_assert(false, "Type not implemented!");
	}

	// generic minval template
	template<typename Type> Type minval(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		static_assert(false, "Type not implemented!");
	}

	// generic maxval template
	template<typename Type> Type maxval(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		static_assert(false, "Type not implemented!");
	}

	// get integer value
	template<> int64_t get<int64_t>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CIntegerPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetValue();
	}

	// set integer value
	template<> void set<int64_t>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const int64_t& value)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CIntegerPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// cast and set value
		pNode->SetValue(value);
	}

	// get integer minimum
	template<> int64_t minval<int64_t>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CIntegerPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetMin();
	}

	// get integer maximum
	template<> int64_t maxval<int64_t>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CIntegerPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetMax();
	}

	// get boolean value
	template<> bool get<bool>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CBooleanPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetValue();
	}

	// set boolean value
	template<> void set<bool>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const bool& value)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CBooleanPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// cast and set value
		pNode->SetValue(value);
	}

	// get float value
	template<> double get<double>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CFloatPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetValue();
	}

	// set float value
	template<> void set<double>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const double& value)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CFloatPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// cast and set value
		pNode->SetValue(value);
	}

	// get float minimum
	template<> double minval<double>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CFloatPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetMin();
	}

	// get float maximum
	template<> double maxval<double>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CFloatPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return pNode->GetMax();
	}

	// get string value
	template<> std::string get<std::string>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CStringPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return std::string(pNode->GetValue().c_str());
	}

	// set string value
	template<> void set<std::string>(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const std::string& value)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CStringPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// cast and set value
		pNode->SetValue(value.c_str());
	}

	// run command
	void run(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CCommandPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// cast and exec command
		pNode->Execute();
	}

	// get enum string value
	std::string getEnumString(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes) const
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CEnumerationPtr>(eRootNodeMap, rNodes, ACCESS_READ);

		// cast and return value
		return std::string(pNode->ToString().c_str());
	}

	// set string value
	void setEnumByString(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, const std::string& value)
	{
		// lock mutex
		AUTOLOCK(this->m_mutex);

		// get node
		auto pNode = getNode<Spinnaker::GenApi::CEnumerationPtr>(eRootNodeMap, rNodes, ACCESS_WRITE);

		// get entry
		auto pEnumEntryNode = pNode->GetEntryByName(value.c_str());

		// check access and type
		assert_access(pEnumEntryNode, ACCESS_READ);
		assert_type(pEnumEntryNode, Spinnaker::GenApi::intfIEnumEntry);

		// cast and set value
		pNode->SetIntValue(pEnumEntryNode->GetValue());
	}

	// these are accessible to the user
	struct
	{
		EnumProperty gain_auto;
		FloatProperty gain;
		BooleanProperty gamma_enable;
		BooleanProperty sharpness_enable;
		BooleanProperty blacklevel_enable;
		BooleanProperty hue_enable;
		BooleanProperty saturation_enable;
	} AnalogControl;

	struct
	{
		EnumProperty trigger_mode;
		EnumProperty trigger_source;
		EnumProperty trigger_selector;
		CommandProperty trigger_cmd;
		EnumProperty framerate_mode;
		EnumProperty exposure_mode;
		EnumProperty exposure_auto;
		FloatProperty exposure_time;
		EnumProperty acquisition_mode;
		BooleanProperty framerate_enabled, framerate_enable;
		BooleanProperty hdr_enable;
		CommandProperty start;
		CommandProperty stop;
	} AcquisitionControl;

	struct
	{
		EnumProperty pixel_format;
		BooleanProperty bigendian;
		BooleanProperty reversex;
		BooleanProperty colorprocessing;
		EnumProperty videomode;
		IntegerProperty widthmax;
		IntegerProperty heightmax;
		IntegerProperty ofsx;
		IntegerProperty ofsy;
		IntegerProperty width;
		IntegerProperty height;
	} ImageFormatControl;

	struct
	{
		EnumProperty selector;
		IntegerProperty value;
	} UserDefinedValues;

	struct
	{
		EnumProperty selector;
		CommandProperty load;
		CommandProperty save;
	} UserSetControl;

	struct
	{
		StringProperty vendor;
		StringProperty name;
		StringProperty serialnumber;
		CommandProperty forceip;
		BooleanProperty wrong_subnet;
	} DeviceInformation;

	struct
	{
		EnumProperty stream_mode;
	} BufferHandlingControl;

private:
	// heartbeat should be disabled in debug mode according to Spinnaker docummentation
	void disableHeartbeat(void)
	{
		AUTOLOCK(this->m_mutex);

		if (this->m_pCamera == nullptr)
			throw NoCameraException();

		Spinnaker::GenApi::CEnumerationPtr ptrDeviceType = this->m_pCamera->GetTLDeviceNodeMap().GetNode("DeviceType");

		if (!IsAvailable(ptrDeviceType) || !IsReadable(ptrDeviceType))
			return;

		if (ptrDeviceType->GetIntValue() != Spinnaker::DeviceType_GigEVision)
			return;

		Spinnaker::GenApi::CBooleanPtr ptrDeviceHeartbeat = this->m_pCamera->GetNodeMap().GetNode("GevGVCPHeartbeatDisable");

		if (IsAvailable(ptrDeviceHeartbeat) && IsWritable(ptrDeviceHeartbeat))
			ptrDeviceHeartbeat->SetValue(true);
	}

	// get root nodemap
	Spinnaker::GenApi::INodeMap& getRootNodeMap(RootNodeMap eRootNodeMap) const
	{
		// throw error if camera is null or invalid
		if (this->m_pCamera == nullptr || !this->m_pCamera.IsValid())
			throw NoCameraException();

		// get root nodemap
		switch (eRootNodeMap)
		{
		case RootNodeMap::Camera:
			return this->m_pCamera->GetNodeMap();

		case RootNodeMap::TransportLayer:
			return this->m_pCamera->GetTLDeviceNodeMap();

		case RootNodeMap::TLStream:
			return this->m_pCamera->GetTLStreamNodeMap();
		}

		// throw exception otherwise
		throw UnknownRootNodeMap(eRootNodeMap);
	}

	// assert that node exist
	void assert_node(const IProperty& prop)
	{
		// get nodes list as string
		auto nodes = prop.getNodes();

		try
		{
			// locate node
			auto pNode = getNode(prop.getRootNodeMap(), nodes);

			// assert default access
			assert_access(pNode, 0);

			// return successfuly
			return;
		}
		catch (...) {}

		// concat names
		std::string name;

		try
		{
			name = std::string(getRootNodeMap(prop.getRootNodeMap()).GetDeviceName().c_str()) + std::string("> ") + concat_string(nodes, "> ");
		}
		catch (...)
		{
			name = "?";
		}

		// throw exception
		throw MissingNodeException(name);
	}

	// check that the required access is available
	void assert_access(Spinnaker::GenApi::CNodePtr pNode, unsigned long ulAccess = ACCESS_ALL) const
	{
		// check that node is valid
		if (!pNode.IsValid())
			throw InvalidNodeException();

		// check that node is available
		if (!IsAvailable(pNode))
			throw InvalidAccessException(pNode->GetName().c_str());

		// check read access
		if ((ulAccess & ACCESS_READ) != 0 && !IsReadable(pNode))
			throw InvalidAccessException(pNode->GetName().c_str());

		// check read access
		if ((ulAccess & ACCESS_WRITE) != 0 && !IsWritable(pNode))
			throw InvalidAccessException(pNode->GetName().c_str());
	}

	// asser the type of the node
	void assert_type(Spinnaker::GenApi::CNodePtr pNode, Spinnaker::GenApi::EInterfaceType eType) const
	{
		// check that node is valid
		if (!pNode.IsValid())
			throw InvalidNodeException();

		// check type
		if (pNode->GetPrincipalInterfaceType() != eType)
			throw InvalidTypeException(pNode->GetName().c_str());
	}

	// get node from batch of strings
	Spinnaker::GenApi::CNodePtr getNode(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, unsigned long ulAccess = ACCESS_ALL) const
	{
		// get root nodemap
		Spinnaker::GenApi::INodeMap& rootNodeMap = getRootNodeMap(eRootNodeMap);

		// browse nodes
		Spinnaker::GenApi::CNodePtr pCurrNode = nullptr;

		for (auto it = rNodes.begin(); it < rNodes.end(); it++)
		{
			if (pCurrNode == nullptr)
				pCurrNode = rootNodeMap.GetNode(it->c_str());
			else
			{
				// get node map
				Spinnaker::GenApi::CNodeMapPtr pNodeMapPtr = pCurrNode->GetNodeMap();

				if (!pNodeMapPtr.IsValid())
					throw InvalidNodeMapException(it->c_str());

				// get node
				pCurrNode = pNodeMapPtr->GetNode(it->c_str());
			}

			// check that node is valid
			if (!pCurrNode.IsValid())
				throw UnknownNodeException(it->c_str());
		}

		// check that node is valid
		if (!pCurrNode.IsValid())
			throw InvalidNodeException();

		// check access
		assert_access(pCurrNode, ulAccess);

		// return node
		return pCurrNode;
	}

	template<class Type> Spinnaker::GenApi::EInterfaceType getNodeType(void) const
	{
		static_assert(false, "Type not implemented!");
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CIntegerPtr>(void) const
	{
		return Spinnaker::GenApi::intfIInteger;
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CFloatPtr>(void) const
	{
		return Spinnaker::GenApi::intfIFloat;
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CBooleanPtr>(void) const
	{
		return Spinnaker::GenApi::intfIBoolean;
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CStringPtr>(void) const
	{
		return Spinnaker::GenApi::intfIString;
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CCommandPtr>(void) const
	{
		return Spinnaker::GenApi::intfICommand;
	}

	template<> Spinnaker::GenApi::EInterfaceType getNodeType<Spinnaker::GenApi::CEnumerationPtr>(void) const
	{
		return Spinnaker::GenApi::intfIEnumeration;
	}

	template<class Type> Type cast(Spinnaker::GenApi::CNodePtr pNode) const
	{
		// check type
		assert_type(pNode, getNodeType<Type>());

		// cast and return value
		return static_cast<Type>(pNode);
	}

	template<class Type> Type getNode(RootNodeMap eRootNodeMap, const std::vector<std::string>& rNodes, unsigned long ulAccess = ACCESS_ALL) const
	{
		return cast<Type>(getNode(eRootNodeMap, rNodes, ulAccess));
	}

	// members
	Spinnaker::CameraPtr m_pCamera;

	mutable std::mutex m_mutex;

	mutable bool m_bAcquiring;
};