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

 // disable stupid warnings
#pragma warning(disable:26451)

#include <string>

#include <Windows.h>

#include "../utils/exception.h"

// types of root key
enum class RegistryRootKey
{
	CurrentUser,
	LocalMachine,
	CurrentConfig,
};

class UnknownRootKeyException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown root key!";
	}
};

// return registry root key
static HKEY getRootKey(RegistryRootKey eRootKey)
{
	switch (eRootKey)
	{
	case RegistryRootKey::CurrentUser:
		return HKEY_CURRENT_USER;

	case RegistryRootKey::LocalMachine:
		return HKEY_LOCAL_MACHINE;

	case RegistryRootKey::CurrentConfig:
		return HKEY_CURRENT_CONFIG;
	}

	throw UnknownRootKeyException();
}

 // save binary data to registry
static bool saveDataToRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue, unsigned char* pData, size_t nSize)
{
	HKEY hKey = 0;

	do
	{
		// open or create subkey
		if (RegCreateKeyExA(getRootKey(eRootKey), rKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
			break;

		// set value
		if (RegSetValueExA(hKey, rValue.c_str(), 0, REG_BINARY, pData, (DWORD)nSize) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// save string to registry
static bool saveStringToRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue, const std::string& rData)
{
	HKEY hKey = 0;

	do
	{
		// open or create subkey
		if (RegCreateKeyExA(getRootKey(eRootKey), rKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
			break;

		// set value
		if (RegSetValueExA(hKey, rValue.c_str(), 0, REG_SZ, (unsigned char*)rData.c_str(), (DWORD)(rData.length() + 1)) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// save int to registry
static bool saveIntToRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue, unsigned long ulData)
{
	HKEY hKey = 0;

	do
	{
		// open or create subkey
		if (RegCreateKeyExA(getRootKey(eRootKey), rKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
			break;

		// set value
		if (RegSetValueExA(hKey, rValue.c_str(), 0, REG_DWORD, (PBYTE)&ulData, sizeof(ulData)) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// load binary data from registry
static bool loadBinaryFromRegistry(RegistryRootKey eRootKey, const std::string &rKey, const std::string &rValue, unsigned char*& rpData, size_t& rSize)
{
	HKEY hKey = 0;

	rpData = nullptr;
	rSize = 0;

	do
	{
		// open subkey in read only
		if (RegOpenKeyExA(getRootKey(eRootKey), rKey.c_str(), REG_OPTION_NON_VOLATILE, KEY_READ, &hKey) != ERROR_SUCCESS)
			break;

		// first query type
		DWORD dwType = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, NULL) != ERROR_SUCCESS)
			break;

		if (dwType != REG_BINARY)
			break;

		// then query size of field
		DWORD dwSize = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, &dwSize) != ERROR_SUCCESS)
			break;

		rSize = dwSize;

		// allocate data and read it
		rpData = (unsigned char*)malloc(rSize);

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, rpData, &dwSize) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// clear memory if not successful
	if (rpData != nullptr)
		free(rpData);

	rpData = nullptr;
	rSize = 0;

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// load string from registry
static bool loadStringFromRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue, std::string& rString)
{
	HKEY hKey = 0;

	rString = "";

	unsigned char* pData = nullptr;

	do
	{
		// open subkey in read only
		if (RegOpenKeyExA(getRootKey(eRootKey), rKey.c_str(), REG_OPTION_NON_VOLATILE, KEY_READ, &hKey) != ERROR_SUCCESS)
			break;

		// first query type
		DWORD dwType = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, NULL) != ERROR_SUCCESS)
			break;

		if (dwType != REG_SZ)
			break;

		// then query size of field
		DWORD dwSize = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, &dwSize) != ERROR_SUCCESS)
			break;

		// allocate data and read it
		pData = (unsigned char*)malloc(__ADD(dwSize, (DWORD)1));
		memset(pData, 0, __ADD(dwSize, (DWORD)1));

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, pData, &dwSize) != ERROR_SUCCESS)
			break;

		// read string
		rString = std::string((char*)pData);

		// clear memory
		if (pData != nullptr)
			free(pData);

		pData = nullptr;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// clear memory
	if (pData != nullptr)
		free(pData);

	pData = nullptr;

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// load int from registry
static bool loadIntFromRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue, unsigned long& rData)
{
	HKEY hKey = 0;

	rData = 0;

	do
	{
		// open subkey in read only
		if (RegOpenKeyExA(getRootKey(eRootKey), rKey.c_str(), REG_OPTION_NON_VOLATILE, KEY_READ, &hKey) != ERROR_SUCCESS)
			break;

		// first query type
		DWORD dwType = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, NULL) != ERROR_SUCCESS)
			break;

		if (dwType != REG_DWORD)
			break;

		// then query size of field
		DWORD dwSize = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, &dwSize) != ERROR_SUCCESS)
			break;

		if (dwSize != sizeof(rData))
			break;

		// read it
		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, (PBYTE)&rData, &dwSize) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// remove value from registry
static bool removeValueFromRegistry(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue)
{
	HKEY hKey = 0;

	do
	{
		// open or create subkey
		if (RegCreateKeyExA(getRootKey(eRootKey), rKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
			break;

		// remove value
		if (RegDeleteValueA(hKey, rValue.c_str()) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// remove key from registry
static bool removeKeyFromRegistry(RegistryRootKey eRootKey, const std::string& rKey)
{
	HKEY hKey = 0;

	do
	{
		// remove key
		if (RegDeleteKeyA(getRootKey(eRootKey), rKey.c_str()) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}

// return true if value exists in registry with READ access
static bool doesRegistryValueExists(RegistryRootKey eRootKey, const std::string& rKey, const std::string& rValue)
{
	HKEY hKey = 0;

	do
	{
		// open subkey in read only
		if (RegOpenKeyExA(getRootKey(eRootKey), rKey.c_str(), REG_OPTION_NON_VOLATILE, KEY_READ, &hKey) != ERROR_SUCCESS)
			break;

		// query type to probe
		DWORD dwType = 0;

		if (RegQueryValueExA(hKey, rValue.c_str(), 0, &dwType, NULL, NULL) != ERROR_SUCCESS)
			break;

		// success
		RegCloseKey(hKey);

		return true;

	} while (false);

	// close key
	if (hKey != 0)
		RegCloseKey(hKey);

	return false;
}