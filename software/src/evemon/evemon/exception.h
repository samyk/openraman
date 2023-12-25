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

#include "shared/utils/exception.h"

 // InitException exception class
class InitException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Cannot initialize application!";
	}
};

// WrongFileTypeException exception class
class WrongFileTypeException : public IException
{
public:
	WrongFileTypeException(const std::string& rFilename)
	{
		this->m_filename = rFilename;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Wrong file type for file \"") + this->m_filename + std::string("\" !");
	}

private:
	std::string m_filename;
};

// UnknownFileTypeException exception class
class UnknownFileTypeException : public IException
{
public:
    UnknownFileTypeException(const std::string& rFilename)
    {
        this->m_filename = rFilename;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Unknown file type for file \"") + this->m_filename + std::string("\" !");
    }

private:
    std::string m_filename;
};