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

#include "storage_buffer.h"
#include "storage_object.h"
#include "encode.h"

#include <vector>
#include <string>
#include <ios>
#include <fstream>

#define DEFAULT_ENCODING_FLAGS		(FLAG_ENABLE_RLE8 | FLAG_ENABLE_RLE0)

// MultipleContainerException exception class
class MultipleContainerException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Container has multiple items!";
	}
};

// StorageContainer class
class StorageContainer : public std::list<StorageObject>
{
public:
	StorageContainer(unsigned __int32 ulFileType='SPC0', unsigned __int32 ulEncryptionKey = 0xfeedbeef, size_t nEncryptionBlockSize = 4)
	{
		this->m_ulFileType = ulFileType;

		this->m_ulEncryptionKey = ulEncryptionKey;
		this->m_nEncryptionBlockSize = nEncryptionBlockSize;
	}

	// retrieve one specific item
	StorageObject *get(const std::string& rOwnerName, const std::string& rVarName)
	{
		// browse all items
		for (auto it = begin(); it != end(); it++)
		{
			if (it->getOwnerName() == rOwnerName && it->getVarName() == rVarName)
				return it.operator->();
		}

		// return null if not found
		return nullptr;
	}

	// save to file
	void saveToFile(const std::string& rFilename, unsigned long ulEncodingFlags = DEFAULT_ENCODING_FLAGS)
	{
		// open file stream
		std::ofstream file(rFilename, std::ios::out | std::ios::binary | std::ios::trunc);

		// exit if cannot open file
		if (!file.is_open())
			throwException(FileOpenException, rFilename);

		// pack everything
		auto buffer = pack(ulEncodingFlags);

		// write
		file.write((const char*)buffer.data(), buffer.size());
	}

	// pack to storage buffer
	StorageBuffer pack(unsigned long ulEncodingFlags = DEFAULT_ENCODING_FLAGS)
	{
		// storage buffer
		StorageBuffer ret, ofs_table, all_buffers;

		// fill-in header file
		size_t nHeaderOfs = ret.allocate<struct header_s>();

		ret.ptr<struct header_s>(nHeaderOfs)->ulIdent = this->m_ulFileType;
		ret.ptr<struct header_s>(nHeaderOfs)->ulChecksum = 0;

		ret.ptr<struct header_s>(nHeaderOfs)->nNumBuffers = __UINT64(size());

		// add all buffers to "all_buffers" buffer and save offsets
		for (auto it = begin(); it != end(); it++)
		{
			struct buffer_s s;

			auto temp = it->pack();

			// encode buffer
			auto encoded = encode(temp, ulEncodingFlags);

			s.ucEncoding = encoded.encoding;
			s.nOffset = __UINT64(all_buffers.append(encoded.buffer, false));
			s.nSize = __UINT64(encoded.buffer.size());

			ofs_table.append(s, false);
		}		

		// write offset table
		ret.ptr<struct header_s>(nHeaderOfs)->nBuffersTableOfs = __UINT64(ret.append(ofs_table, false));
		ret.ptr<struct header_s>(nHeaderOfs)->nBuffersDataOfs = __UINT64(ret.append(all_buffers, false));
		ret.ptr<struct header_s>(nHeaderOfs)->ulSeed = __UINT32(rand() % 0xFFFF) | (__UINT32(rand() % 0xFFFF) << 16);

		// eventually padd with zeros
		size_t nPadding = __SUB(ret.size(), sizeof(struct header_s)) % sizeof(unsigned __int32);

		if (nPadding > 0)
			ret.allocate(__SUB(sizeof(unsigned __int32), nPadding));

		// checksum
		ret.ptr<struct header_s>(nHeaderOfs)->ulChecksum = checksum(ret.data(), ret.size());

		// encrypt
		encrypt((unsigned __int32*)(__ADDRESS(ret.data(), sizeof(struct header_s))), __SUB(ret.size(), sizeof(struct header_s)) / sizeof(unsigned __int32), this->m_ulEncryptionKey ^ ret.ptr<struct header_s>(nHeaderOfs)->ulSeed, true);

		// return buffer;
		return ret;
	}

	// unpack from storage buffer
	void unpack(StorageBuffer buffer)
	{
		// WrongFileTypeException exception class
		class WrongFileTypeException : public IException
		{
		public:
			virtual std::string toString(void) const override
			{
				return "Invalid File type. Aborting!";
			}
		};

		// WrongChecksumException exception class
		class WrongChecksumException : public IException
		{
		public:
			virtual std::string toString(void) const override
			{
				return "Data may have been corrupted. Aborting!";
			}
		};

		// InvalidBufferException exception class
		class InvalidBufferException : public IException
		{
		public:
			virtual std::string toString(void) const override
			{
				return "Buffer is invalid. Aborting!";
			}
		};

		// first clear everything
		clear();

		// read header
		auto pHeader = buffer.ptr<struct header_s>(0, sizeof(struct header_s));

		// check ident
		if (pHeader->ulIdent != this->m_ulFileType)
			throwException(WrongFileTypeException);

		// restore original header
		auto ulOldChecksum = pHeader->ulChecksum;
		pHeader->ulChecksum = 0;

		// unencrypt data
		encrypt((unsigned __int32*)__ADDRESS(buffer.data(), sizeof(struct header_s)), __SUB(buffer.size(), sizeof(struct header_s)) / sizeof(unsigned __int32), this->m_ulEncryptionKey ^ pHeader->ulSeed, false);

		// verify checksum
		auto ulChecksum = checksum(buffer.data(), buffer.size());

		if (ulChecksum != ulOldChecksum)
			throwException(WrongChecksumException);

		// get data
		auto pOffsetsTable = buffer.ptr<struct buffer_s>(__SIZE_T(pHeader->nBuffersTableOfs), __MULT(sizeof(struct buffer_s), __SIZE_T(pHeader->nNumBuffers)));
		auto pData = buffer.ptr<unsigned char>(__SIZE_T(pHeader->nBuffersDataOfs));

		for (size_t i = 0; i < __SIZE_T(pHeader->nNumBuffers); i++)
		{
			if (__SIZE_T(__ADD(pOffsetsTable[i].nOffset, pOffsetsTable[i].nSize)) > buffer.size())
				throwException(InvalidBufferException);

			StorageBuffer temp_buffer(__ADDRESS(pData, __SIZE_T(pOffsetsTable[i].nOffset)), __SIZE_T(pOffsetsTable[i].nSize));

			StorageObject obj;
			obj.unpack(decode(temp_buffer, pOffsetsTable[i].ucEncoding));

			emplace_back(std::move(obj));
		}
	}

private:
#pragma pack(push, 8)
	struct header_s
	{
		unsigned __int32 ulIdent;
		unsigned __int32 ulChecksum;

		unsigned __int64 nNumBuffers;
		unsigned __int64 nBuffersTableOfs;

		unsigned __int32 ulSeed;

		unsigned __int64 nBuffersDataOfs;

		unsigned __int32 ulReserved[10];
	};

	struct buffer_s
	{
		unsigned char ucEncoding;
		unsigned __int64 nOffset;
		unsigned __int64 nSize;
	};
#pragma pack(pop)

	// simple encryption function for avalanche effect
	void encrypt(unsigned __int32 *pulBuffer, size_t nNumElements, unsigned __int32 ulEncryptionKey, bool bForward)
	{
		if(this->m_nEncryptionBlockSize == 0)
			return;

		auto repmat = [](unsigned __int32 ulValue)
		{
			ulValue &= 0xFF;

			ulValue |= ulValue << 8;
			ulValue |= ulValue << 16;

			return ~ulValue;
		};

		ulEncryptionKey += repmat((unsigned __int32)nNumElements);

		for (size_t j = 0; j < this->m_nEncryptionBlockSize; j++)
			for (size_t i = j; i < nNumElements; i = __ADD(i, this->m_nEncryptionBlockSize))
			{
				auto temp = ~pulBuffer[i];

				pulBuffer[i] ^= ulEncryptionKey;

				ulEncryptionKey += bForward ? ~pulBuffer[i] : temp;
				ulEncryptionKey += repmat((unsigned __int32)i);
			}
	}

	// compute checksum
	unsigned __int32 checksum(unsigned char *p, size_t nSize)
	{
		unsigned __int32 ulSum = 0;

		// add longs as long as possible
		while (nSize >= sizeof(unsigned __int32))
		{
			ulSum += ~*((unsigned __int32*)p);
			p += sizeof(unsigned __int32);
			nSize -= sizeof(unsigned __int32);
		}

		// then add shorts
		while (nSize >= sizeof(unsigned __int16))
		{
			ulSum += ~*((unsigned __int16*)p);
			p += sizeof(unsigned __int16);
			nSize -= sizeof(unsigned __int16);
		}

		// then add chars
		while (nSize >= sizeof(unsigned __int8))
		{
			ulSum += ~*((unsigned __int8*)p);
			p += sizeof(unsigned __int8);
			nSize -= sizeof(unsigned __int8);
		}

		return ~ulSum;
	}

	unsigned __int32 m_ulFileType;
	unsigned __int32 m_ulEncryptionKey;

	size_t m_nEncryptionBlockSize;
};