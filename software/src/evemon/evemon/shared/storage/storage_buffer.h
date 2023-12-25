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

#include "../utils/utils.h"
#include "../utils/exception.h"
#include "../utils/safe.h"

#include "registry.h"

#include <vector>
#include <string>
#include <ios>
#include <fstream>

#include <Windows.h>

// FileOpenException exception class
class FileOpenException : public IException
{
public:
	FileOpenException(const std::string& rFilename)
	{
		this->m_filename = rFilename;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Cannot open file \"") + this->m_filename + std::string("\"!");
	}

private:
	std::string m_filename;
};

// FileReadException
class FileReadException : public IException
{
public:
	FileReadException(const std::string& rFilename, size_t nSize)
	{
		this->m_filename = rFilename;
		this->m_nSize = nSize;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[32];

		sprintf_s(szTmp, "%zu", this->m_nSize);

		return std::string("Cannot read ") + std::string(szTmp) + std::string(" bytes from file \"") + this->m_filename + std::string("\"!");
	}

private:
	std::string m_filename;
	size_t m_nSize;
};

// MemoryAllocationException exception class
class MemoryAllocationException : public IException
{
public:
	MemoryAllocationException(size_t nSize)
	{
		this->m_nSize = nSize;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[128];

		sprintf_s(szTmp, "Cannot allocated %zu bytes!", this->m_nSize);

		return std::string(szTmp);
	}

private:
	size_t m_nSize;
};

// RegistryAccessException exception class
class RegistryAccessException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Cannot access registry data!";
	}
};

// manage buffer memory for storage
class StorageBuffer
{
public:
	// initialize to null values
	StorageBuffer(void)
	{
		this->m_pData = nullptr;
		this->m_nSize = 0;
	}

	StorageBuffer(const unsigned char *pData, size_t nSize)
	{
		this->m_pData = nullptr;
		this->m_nSize = 0;

		append(pData, nSize, false);
	}

	StorageBuffer(const StorageBuffer& rBuffer)
	{
		this->m_pData = nullptr;
		this->m_nSize = 0;

		this->operator=(rBuffer);
	}

	StorageBuffer(StorageBuffer&& rBuffer)
	{
		this->m_pData = rBuffer.m_pData;
		this->m_nSize = rBuffer.m_nSize;

		rBuffer.m_pData = nullptr;
		rBuffer.m_nSize = 0;
	}

	// release memory on dtor
	~StorageBuffer(void)
	{
		clear();
	}

	// load buffer from file
	void loadFromFile(const std::string& rFilename)
	{
		// clear previous memory
		clear();

		// open file
		FILE *pFile = nullptr;

		fopen_s(&pFile, rFilename.c_str(), "rb");

		if (pFile == nullptr)
			throwException(FileOpenException, rFilename);

		// get file size
		fseek(pFile, 0, SEEK_END);
		size_t nSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		// allocate memory
		this->m_pData = (unsigned char*)malloc(nSize);

		if (this->m_pData == nullptr)
		{
			fclose(pFile);

			throwException(MemoryAllocationException, nSize);
		}

		// assign size
		this->m_nSize = nSize;

		// read data
		if (fread(this->m_pData, sizeof(unsigned char), nSize, pFile) != nSize)
		{
			fclose(pFile);
			clear();

			throwException(FileReadException, rFilename, nSize);
		}

		// close file
		fclose(pFile);
	}

	// load from registry
	void loadFromRegistry(RegistryRootKey eRootKey, const char *pszRegistryKey, const char *pszValueName)
	{
		// clear previous
		clear();

		// load from registry
		if (!loadBinaryFromRegistry(eRootKey, pszRegistryKey, pszValueName, this->m_pData, this->m_nSize))
			throwException(RegistryAccessException);
	}

	// copy operator
	StorageBuffer& operator=(const StorageBuffer& rBuffer)
	{
		clear();

		if (rBuffer.m_nSize != 0 && rBuffer.m_pData != nullptr)
			append(rBuffer.m_pData, rBuffer.m_nSize, false);

		return *this;
	}

	// allocate empty space
	size_t allocate(size_t nSize)
	{
		size_t nNewSize = __ADD(this->m_nSize, nSize);

		this->m_pData = (unsigned char*)realloc(this->m_pData, nNewSize);

		if (this->m_pData == nullptr)
			throwException(MemoryAllocationException, nNewSize);

		memset(__ADDRESS(this->m_pData, this->m_nSize), 0, nSize);

		size_t nOffset = this->m_nSize;
		this->m_nSize = nNewSize;

		return nOffset;
	}

	// allocate space for known type and return pointer to type
	template<typename Type> size_t allocate(void)
	{
		return allocate(sizeof(Type));
	}

	// cast to data at given offset
	template<typename Type> Type* ptr(size_t nOffset, size_t nCheckSize=0)
	{
		if (nOffset >= this->m_nSize)
			throwException(InvalidOffsetException);

		if (__ADD(nOffset, nCheckSize) > this->m_nSize)
			throwException(TypeFitException);

		return (Type*)__ADDRESS(data(), nOffset);
	}

	// cast to data at given offset (const)
	template<typename Type> const Type* ptr(size_t nOffset, size_t nCheckSize = 0) const
	{
		if (nOffset >= this->m_nSize && nCheckSize != 0)
			throwException(InvalidOffsetException);

		if (__ADD(nOffset, nCheckSize) > this->m_nSize)
			throwException(TypeFitException);

		return (const Type*)__ADDRESS(data(), nOffset);
	}

	// copy data
	size_t append(const unsigned char *pBytes, size_t nSize, bool bRecycle)
	{
		// check if data can be recycled
		if (bRecycle && this->m_nSize >= nSize)
		{
			// scan
			for (size_t i = 0; i <= __SUB(this->m_nSize, nSize); i++)
			{
				// return current position if data matches
				if (match(__ADDRESS(this->m_pData, i), pBytes, nSize))
					return i;
			}
		}

		// allocate new data otherelse
		size_t nNewSize = __ADD(this->m_nSize, nSize);

		this->m_pData = (unsigned char*)realloc(this->m_pData, nNewSize);

		if (this->m_pData == nullptr)
			throwException(MemoryAllocationException, nNewSize);

		memcpy(__ADDRESS(this->m_pData, this->m_nSize), pBytes, nSize);

		size_t nOffset = this->m_nSize;
		this->m_nSize = nNewSize;

		return nOffset;
	}

	// copy data of known type
	template<typename Type> size_t append(const Type& rType, bool bRecycle)
	{
		return append((unsigned char*)&rType, sizeof(rType), bRecycle);
	}

	// copy string is special
	template<> size_t append(const std::string& rString, bool bRecycle)
	{
		return append((unsigned char*)rString.c_str(), __ADD(rString.length(), (size_t)1), bRecycle);
	}

	// copy other storage buffer is special
	template<> size_t append(const StorageBuffer& rBuffer, bool bRecycle)
	{
		return append(rBuffer.data(), rBuffer.size(), bRecycle);
	}

	// pointer to data
	unsigned char *data(void)
	{
		return this->m_pData;
	}

	// const pointer to data
	const unsigned char *data(void) const
	{
		return this->m_pData;
	}

	// size of data
	size_t size() const
	{
		return this->m_nSize;
	}

	// return true if both blocks contains the same data
	bool equals(const StorageBuffer& rBlock)
	{
		if (rBlock.size() != this->m_nSize)
			return false;

		if (rBlock.m_pData == nullptr)
			return false;

		if (this->m_pData == nullptr)
			return false;

		auto pOtherBlockData = rBlock.data();

		for (size_t i = 0; i < this->m_nSize; i++)
			if (this->m_pData[i] != pOtherBlockData[i])
				return false;

		return true;
	}

private:
	// InvalidOffsetException exception class
	class InvalidOffsetException : public IException
	{
	public:
		virtual std::string toString(void) const override
		{
			return "Invalid offset!";
		}
	};

	// TypeFitException exception class
	class TypeFitException : public IException
	{
	public:
		virtual std::string toString(void) const override
		{
			return "Type does not fit in available buffer space!";
		}
	};

	// check if both data matches
	bool match(const unsigned char* pData1, const unsigned char* pData2, size_t nSize)
	{
		// return false if any bytes do not match
		for (size_t i = 0; i < nSize; i++)
			if (pData1[i] != pData2[i])
				return false;

		// return true otherwise
		return true;
	}

	void clear(void)
	{
		if(this->m_pData != nullptr)
			free(this->m_pData);

		this->m_pData = nullptr;
		this->m_nSize = 0;
	}

	unsigned char *m_pData;
	size_t m_nSize;
};

// load storage buffer from a file
static StorageBuffer loadBufferFromFile(const std::string& rFilename)
{
	StorageBuffer buffer;
	buffer.loadFromFile(rFilename);

	return buffer;
}

// load storage buffer from registry
static StorageBuffer loadBufferFromRegistry(RegistryRootKey eRootKey, const char *pszRegistryKey, const char *pszValueName)
{
	StorageBuffer buffer;
	buffer.loadFromRegistry(eRootKey, pszRegistryKey, pszValueName);

	return buffer;
}

// UnknownResourceException exception class
class UnknownResourceException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown Resource!";
	}
};

// load storage buffer from resource
static StorageBuffer loadBufferFromResource(int iResourceID, HMODULE hModule=NULL)
{
	// find resource inside current module
	auto hResource = FindResource(hModule, MAKEINTRESOURCE(iResourceID), RT_RCDATA);

	if (hResource == NULL)
		throwException(UnknownResourceException);

	// get resource size
	size_t nSize = SizeofResource(hModule, hResource);

	// load resource
	auto hGlobal = LoadResource(hModule, hResource);

	if (hGlobal == NULL)
		throwException(UnknownResourceException);

	// lock resource
	auto pData = LockResource(hGlobal);

	if (pData == NULL)
		throwException(UnknownResourceException);

	try
	{
		// create storage buffer
		StorageBuffer ret((unsigned char*)pData, nSize);

		// unlock resource
		FreeResource(hGlobal);

		// return
		return ret;
	}
	catch (...)
	{
		// unlock resource and rethrow error
		FreeResource(hGlobal);

		throw;
	}
}
