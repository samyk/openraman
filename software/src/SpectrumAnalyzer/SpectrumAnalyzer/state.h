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

#include <functional>
#include <string>
#include <array>

#include "shared/utils/exception.h"

#include "shared/storage/stdext.h"

#include "shared/math/vector.h"

#include "shared/gui/signal.h"
#include "shared/gui/axis.h"

#include "exception.h"

 // types of axis
enum class AxisType
{
    Pixels,
    Wavelengths,
    RamanShifts,
};

// types of logging format
enum class LogFormat
{
    CSV,
    SPC,
};

class SpectrumAnalyzerApp;
class IPlotBuilder;

// global state interface
class ISpectrumAnalyzerGlobals
{
protected:
    virtual bool hasBlank(void) const = 0;
    virtual vector_t getBlank(void) const = 0;
    virtual bool hasBlankOpt(void) const = 0;

    virtual bool isParamDialogOpened(void) const = 0;
    virtual  bool isCalibrationDialogOpened(void) const = 0;

    virtual bool hasPlotData(size_t nIndex) const = 0;
    virtual const guiSignal& getPlotData(size_t nIndex) const = 0;

    virtual void clearAnnotations(void) = 0;
    virtual size_t addAnnotation(const guiSignal& rSignal) = 0;
    virtual guiSignal& getAnnotation(size_t nIndex) const = 0;

    virtual guiAxis* getPrimaryHorizontalAxis(void) const = 0;
    virtual guiAxis* getPrimaryVerticalAxis(void) const = 0;
    virtual guiAxis* getSecondaryHorizontalAxis(void) const = 0;
    virtual guiAxis* getSecondaryVerticalAxis(void) const = 0;

    virtual bool isShowSaturationDataEnabled(void) const = 0;
    virtual bool isInMultipleAcquisition(void) const = 0;
    virtual AxisType getAxisType(void) const = 0;
    virtual int getSmoothing(void) const = 0;
    virtual bool isMedFiltEnabled(void) const = 0;
    virtual bool isBaselineRemovalEnabled(void) const = 0;
    virtual bool isBlankRemovalEnabled(void) const = 0;
    virtual double getExposure(void) const = 0;
    virtual double getGain(void) const = 0;
    virtual double getGainDB(void) const = 0;
    virtual bool hasPlot(void) const = 0;
    virtual bool hasAxisChangeOpt(void) const = 0;
    virtual bool isSGolayEnable(void) const = 0;
    virtual int getSGolayWindowSize(void) const = 0;
    virtual int getSGolayOrder(void) const = 0;
    virtual int getSGolayDerivative(void) const = 0;
    virtual double getRamanWavelength(void) const = 0;
    virtual LogFormat getLogFormat(void) const = 0;

    virtual bool hasCamera(void) const = 0;
    virtual void disconnectCamera(void) = 0;
    virtual void setCamera(const std::string& camera) = 0;

    virtual void onConfirmImageSave(const std::string& rTitle) = 0;

    virtual void setPlotBuilder(std::shared_ptr<IPlotBuilder> pDataBuilder) = 0;

    virtual bool hasCalibrationData(void) const = 0;
    virtual std::array<double, 4> getSolution(void) const = 0;
    virtual void setSolution(const std::array<double, 4>& rSolution) = 0;
};

// global state class for children
class SpectrumAnalyzerChild : public ISpectrumAnalyzerGlobals
{
    friend class SpectrumAnalyzerApp;

public:
    SpectrumAnalyzerChild(void)
    {
        this->m_pApp = nullptr;
    }

protected:
    virtual bool hasBlank(void) const override;
    virtual vector_t getBlank(void) const override;
    virtual bool hasBlankOpt(void) const override;

    virtual bool isParamDialogOpened(void) const override;
    virtual  bool isCalibrationDialogOpened(void) const override;

    virtual bool hasPlotData(size_t nIndex) const override;
    virtual const guiSignal& getPlotData(size_t nIndex) const override;

    virtual void clearAnnotations(void) override;
    virtual size_t addAnnotation(const guiSignal& rSignal) override;
    virtual guiSignal& getAnnotation(size_t nIndex) const override;

    virtual guiAxis* getPrimaryHorizontalAxis(void) const override;
    virtual guiAxis* getPrimaryVerticalAxis(void) const override;
    virtual guiAxis* getSecondaryHorizontalAxis(void) const override;
    virtual guiAxis* getSecondaryVerticalAxis(void) const override;

    virtual bool isShowSaturationDataEnabled(void) const override;
    virtual bool isInMultipleAcquisition(void) const override;
    virtual AxisType getAxisType(void) const override;
    virtual int getSmoothing(void) const override;
    virtual bool isMedFiltEnabled(void) const override;
    virtual bool isBaselineRemovalEnabled(void) const override;
    virtual bool isBlankRemovalEnabled(void) const override;
    virtual double getExposure(void) const override;
    virtual double getGain(void) const override;
    virtual double getGainDB(void) const override;
    virtual bool hasPlot(void) const override;
    virtual bool hasAxisChangeOpt(void) const override;
    virtual bool isSGolayEnable(void) const override;
    virtual int getSGolayWindowSize(void) const override;
    virtual int getSGolayOrder(void) const override;
    virtual int getSGolayDerivative(void) const override;
    virtual LogFormat getLogFormat(void) const override;

    virtual bool hasCamera(void) const override;
    virtual void disconnectCamera(void) override;
    virtual void setCamera(const std::string& camera) override;

    virtual void onConfirmImageSave(const std::string& rTitle) override;

    virtual void setPlotBuilder(std::shared_ptr<IPlotBuilder> pDataBuilder) override;

    virtual double getRamanWavelength(void) const override;

    virtual bool hasCalibrationData(void) const override;
    virtual std::array<double, 4> getSolution(void) const override;
    virtual void setSolution(const std::array<double, 4> &rSolution) override;

private:
    void setApp(SpectrumAnalyzerApp* pApp)
    {
        this->m_pApp = pApp;
    }

    SpectrumAnalyzerApp* m_pApp;
};