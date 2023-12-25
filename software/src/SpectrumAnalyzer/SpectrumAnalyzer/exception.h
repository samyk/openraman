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

// InvalidPlotException exception class
class InvalidPlotException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Invalid plot!";
    }
};

// InvalidPlotStateException exception class
class InvalidPlotStateException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Invalid plot builder object!";
    }
};

// NoCameraException exception class
class NoCameraException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "No camera connected!";
    }
};

// NoCalibrationImported exception class
class NoCalibrationImported : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "No calibration could be imported from camera! It is strongly advised to calibrate your camera using a known source. Follow manual for instructions on how to calibration camera.";
    }
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

// FileNotFoundException exception class
class FileNotFoundException : public IException
{
public:
    FileNotFoundException(const std::string& rFilename)
    {
        this->m_filename = rFilename;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Cannot open file \"") + this->m_filename + std::string("\" !");
    }

private:
    std::string m_filename;
};

// EmptyFileException exception class
class EmptyFileException : public IException
{
public:
    EmptyFileException(const std::string& rFilename)
    {
        this->m_filename = rFilename;
    }

    virtual std::string toString(void) const override
    {
        return std::string("File \"") + this->m_filename + std::string("\" is empty or does not contain valid data!");
    }

private:
    std::string m_filename;
};

// NoCalibrationInFileException exception class
class NoCalibrationInFileException : public IException
{
public:
    NoCalibrationInFileException(const std::string& rFilename)
    {
        this->m_filename = rFilename;
    }

    virtual std::string toString(void) const override
    {
        return std::string("No calibration found in file \"") + this->m_filename + std::string("\" !");
    }

private:
    std::string m_filename;
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

// UnknownModelException exception class
class UnknownModelException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unknown model type for calibration!";
    }
};

// UnknownSourceException exception class
class UnknownSourceException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unknown source type for calibration!";
    }
};

// NoSolutionException exception class
class NoSolutionException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "No solution could be found!";
    }
};

// UnknownAxisTypeException exception class
class UnknownAxisTypeException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unknown axis type!";
    }
};

// UnknownLogFormatException exception class
class UnknownLogFormatException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unknown log format!";
    }
};

// NullDataException class
class NullDataException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "No data!";
    }
};

// InvalidFunctionException exception class
class InvalidFunctionException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Invalid function call!";
    }
};