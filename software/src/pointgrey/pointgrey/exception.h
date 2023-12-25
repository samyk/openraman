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

#include "shared/utils/utils.h"
#include "shared/utils/exception.h"

#include "defs.h"

// TriggerModeDisabledException class
class TriggerModeDisabledException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Trigger function called but trigger is not enabled in the camera!";
	}
};

// NotImplementedException class
class NotImplementedException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Function is not implemente yet!";
	}
};

// PixelFormatException exception
class PixelFormatException : public IException
{
public:
	PixelFormatException(const std::string& found, const std::string& expected)
	{
		this->m_found = found;
		this->m_expected = expected;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Pixel format \"") + this->m_expected + std::string("\" was expected but \"") + this->m_found + ("\" was found!");
	}

private:
	std::string m_found, m_expected;
};

// class InvalidROIException
class InvalidROIException : public IException
{
public:
	InvalidROIException(int iHeight, int iMin, int iMax)
	{
		this->m_iHeight = iHeight;
		this->m_iMin = iMin;
		this->m_iMax = iMax;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[256];

		sprintf_s(szTmp, "Cannot set ROI to %d! Input must be bound between %d and %d!", this->m_iHeight, this->m_iMin, this->m_iMax);

		return std::string(szTmp);
	}

private:
	int m_iHeight, m_iMin, m_iMax;
};

// MissingNodeException exception
class MissingNodeException : public IException
{
public:
	MissingNodeException(const std::string& rNode)
	{
		this->m_nodes = rNode;
	}

	virtual std::string toString(void) const override
	{
		return std::string("\"") + this->m_nodes + std::string("\" was expected but was not found or has no valid access!");
	}

private:
	std::string m_nodes;
};

// InitException class
class InitException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Initialization error!";
	}
};

// WrongUserDataType class
class WrongUserDataType : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "UserData node does not have the expected bytes size!";
	}
};

// NotEnoughMemoryException class
class NotEnoughMemoryException : public IException
{
public:
	NotEnoughMemoryException(size_t nRequestedSize, size_t nAvailableSize)
	{
		this->m_nRequestedSize = nRequestedSize;
		this->m_nAvailableSize = nAvailableSize;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[256];

		sprintf_s(szTmp, "Not enough memory! %zu bytes were requested but only %zu are available!", this->m_nRequestedSize, this->m_nAvailableSize);

		return std::string(szTmp);
	}

private:
	size_t m_nRequestedSize, m_nAvailableSize;
};

// CannotAccessPropertyExcepion class
class CannotAccessPropertyExcepion : public IException
{
public:
	CannotAccessPropertyExcepion(const std::string& rMessage)
	{
		this->m_sMessage = rMessage;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Cannot access property \"") + this->m_sMessage + std::string("!");
	}

private:
	std::string m_sMessage;
};

// NoCameraException class
class NoCameraException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "No Camera!";
	}
};

// ImageAcquisitionException class
class ImageAcquisitionException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Cannot acquire image!";
	}
};

// WrongSubnetException class
class WrongSubnetException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Device is on wrong subnet! Please check GiGE adapter IP address!";
	}
};

class UnknownRootNodeMap : public IException
{
public:
	UnknownRootNodeMap(RootNodeMap eRootNodeMap)
	{
		this->m_eRootNodeMap = eRootNodeMap;
	}

	virtual std::string toString(void) const override
	{
		return "Unknown root nodemap!";
	}

private:
	RootNodeMap m_eRootNodeMap;
};

// UnknownNodeException class
class UnknownNodeException : public IException
{
public:
	UnknownNodeException(const std::string& rNodeName)
	{
		this->m_nodeName = rNodeName;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Unknown node \"") + this->m_nodeName + std::string("\"!");
	}

private:
	std::string m_nodeName;
};

// InvalidNodeMapException class
class InvalidNodeMapException : public IException
{
public:
	InvalidNodeMapException(const std::string& rNodeName)
	{
		this->m_nodeName = rNodeName;
	}

	virtual std::string toString(void) const override
	{
		return std::string("\"") + this->m_nodeName + std::string("\" has invalid nodemap!");
	}

private:
	std::string m_nodeName;
};

// InvalidNodeException class
class InvalidNodeException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Invalid node!";
	}
};

// InvalidAccessException class
class InvalidAccessException : public IException
{
public:
	InvalidAccessException(const std::string& name)
	{
		this->m_name = name;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Node \"") + this->m_name + std::string("\" does not have the requested access mode!");
	}

private:
	std::string m_name;
};

// class InvalidTypeException
class InvalidTypeException : public IException
{
public:
	InvalidTypeException(const std::string& name)
	{
		this->m_name = name;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Node \"") + this->m_name + std::string("\" does not have the requested type!");
	}

private:
	std::string m_name;
};