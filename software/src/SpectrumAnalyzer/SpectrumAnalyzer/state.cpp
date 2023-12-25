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
#include <string>

#include "shared/math/vector.h"

#include "state.h"
#include "app.h"

bool SpectrumAnalyzerChild::hasBlank(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasBlank();
}

vector_t SpectrumAnalyzerChild::getBlank(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getBlank();
}

bool SpectrumAnalyzerChild::hasBlankOpt(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasBlankOpt();
}

bool SpectrumAnalyzerChild::isParamDialogOpened(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isParamDialogOpened();
}

bool SpectrumAnalyzerChild::isCalibrationDialogOpened(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isCalibrationDialogOpened();
}

bool SpectrumAnalyzerChild::hasPlotData(size_t nIndex) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasPlotData(nIndex);
}

const guiSignal& SpectrumAnalyzerChild::getPlotData(size_t nIndex) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getPlotData(nIndex);
}

void SpectrumAnalyzerChild::clearAnnotations(void)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    this->m_pApp->clearAnnotations();
}

size_t SpectrumAnalyzerChild::addAnnotation(const guiSignal& rSignal)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->addAnnotation(rSignal);
}

guiSignal& SpectrumAnalyzerChild::getAnnotation(size_t nIndex) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getAnnotation(nIndex);
}

guiAxis* SpectrumAnalyzerChild::getPrimaryHorizontalAxis(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getPrimaryHorizontalAxis();
}

guiAxis* SpectrumAnalyzerChild::getPrimaryVerticalAxis(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getPrimaryVerticalAxis();
}

guiAxis* SpectrumAnalyzerChild::getSecondaryHorizontalAxis(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSecondaryHorizontalAxis();
}

guiAxis* SpectrumAnalyzerChild::getSecondaryVerticalAxis(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSecondaryVerticalAxis();
}

bool SpectrumAnalyzerChild::isShowSaturationDataEnabled(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isShowSaturationDataEnabled();
}

bool SpectrumAnalyzerChild::isInMultipleAcquisition(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isInMultipleAcquisition();
}

AxisType SpectrumAnalyzerChild::getAxisType(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getAxisType();
}

int SpectrumAnalyzerChild::getSmoothing(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSmoothing();
}

bool SpectrumAnalyzerChild::isMedFiltEnabled(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isMedFiltEnabled();
}

bool SpectrumAnalyzerChild::isBaselineRemovalEnabled(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isBaselineRemovalEnabled();
}

bool SpectrumAnalyzerChild::isBlankRemovalEnabled(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isBlankRemovalEnabled();
}

double SpectrumAnalyzerChild::getExposure(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getExposure();
}

double SpectrumAnalyzerChild::getGain(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getGain();
}

double SpectrumAnalyzerChild::getGainDB(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getGainDB();
}

bool SpectrumAnalyzerChild::isSGolayEnable(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->isSGolayEnable();
}

int SpectrumAnalyzerChild::getSGolayWindowSize(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSGolayWindowSize();
}

int SpectrumAnalyzerChild::getSGolayOrder(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSGolayOrder();
}

int SpectrumAnalyzerChild::getSGolayDerivative(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSGolayDerivative();
}

LogFormat SpectrumAnalyzerChild::getLogFormat(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getLogFormat();
}

bool SpectrumAnalyzerChild::hasPlot(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasPlot();
}

bool SpectrumAnalyzerChild::hasAxisChangeOpt(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasAxisChangeOpt();
}

bool SpectrumAnalyzerChild::hasCamera(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasCamera();
}

void SpectrumAnalyzerChild::disconnectCamera(void)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->disconnectCamera();
}

void SpectrumAnalyzerChild::setCamera(const std::string& camera)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    this->m_pApp->setCamera(camera);
}

void SpectrumAnalyzerChild::onConfirmImageSave(const std::string& rTitle)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    this->m_pApp->onConfirmImageSave(rTitle);
}

void SpectrumAnalyzerChild::setPlotBuilder(std::shared_ptr<IPlotBuilder> pDataBuilder)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    this->m_pApp->setPlotBuilder(pDataBuilder);
}

double SpectrumAnalyzerChild::getRamanWavelength(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getRamanWavelength();
}

bool SpectrumAnalyzerChild::hasCalibrationData(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->hasCalibrationData();
}

std::array<double, 4> SpectrumAnalyzerChild::getSolution(void) const
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    return this->m_pApp->getSolution();
}

void SpectrumAnalyzerChild::setSolution(const std::array<double, 4>& rSolution)
{
    if (this->m_pApp == nullptr)
        throwException(InvalidFunctionException);

    this->m_pApp->setSolution(rSolution);
}