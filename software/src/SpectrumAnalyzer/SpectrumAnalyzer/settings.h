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

#include "shared/storage/registry.h"

#define REGISTRY_KEY        "Software\\OpenRAMAN\\SpectrumAnalyzer"

// load settings from registry
static int loadInt(const std::string& rName, unsigned long ulDefault)
{
    unsigned long ulData;

    if (loadIntFromRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, ulData))
        return ulData;

    return ulDefault;
}

// load settings from registry
static bool loadBool(const std::string& rName, bool bDefault)
{
    unsigned long ulData;

    if (loadIntFromRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, ulData))
        return ulData != 0;

    return bDefault;
}

// load settings from registry
static std::string loadString(const std::string& rName, const std::string &rDefault)
{
    std::string data;

    if (loadStringFromRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, data))
        return data;

    return rDefault;
}

// save settings to registry
static bool saveInt(const std::string& rName, unsigned long ulData)
{
    return saveIntToRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, ulData);
}

// save settings to registry
static bool saveBool(const std::string& rName, bool bData)
{
    return saveIntToRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, bData ? 1 : 0);
}

// save settings to registry
static bool saveString(const std::string& rName, const std::string &rData)
{
    return saveStringToRegistry(RegistryRootKey::CurrentUser, REGISTRY_KEY, rName, rData);
}