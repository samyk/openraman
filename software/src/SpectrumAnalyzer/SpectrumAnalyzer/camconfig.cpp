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
#include "camconfig.h"

// declare storage data
BEGIN_STORAGE(wndParametersDialog)
{
    DECLARE_STORAGE(wndParametersDialog, smoothing),
    DECLARE_STORAGE(wndParametersDialog, average),
    DECLARE_STORAGE(wndParametersDialog, sgolay_window),
    DECLARE_STORAGE(wndParametersDialog, sgolay_order),
    DECLARE_STORAGE(wndParametersDialog, sgolay_deriv),
    DECLARE_STORAGE(wndParametersDialog, raman_wavelength),
    DECLARE_STORAGE(wndParametersDialog, medfilt),
    DECLARE_STORAGE(wndParametersDialog, baseline),
    DECLARE_STORAGE(wndParametersDialog, sgolay),
    DECLARE_STORAGE(wndParametersDialog, axis),
}
END_STORAGE(wndParametersDialog, wndParametersDialog)