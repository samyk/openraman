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

#include "../utils/exception.h"

#include "storage_buffer.h"
#include "rle0.h"
#include "rle8.h"

#define FLAG_ENABLE_RLE8			(1 << 0)
#define FLAG_ENABLE_RLE0			(1 << 1)

// type of encoding
enum
{
	ENCODING_NONE = 0,
	ENCODING_RLE8 = 1,
	ENCODING_RLE0 = 2,
};

// UnknownEncodingException exception class
class UnknownEncodingException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown encoding method!";
	}
};

static void OutputHex(const StorageBuffer& rBuffer)
{
	char szTmp[16];

	// scan
	for (size_t i = 0; i < rBuffer.size(); __INC(i,(size_t)16))
	{
		// scan line
		for (size_t j = 0; j < 16; j++)
		{
			if (i + j < rBuffer.size())
				sprintf_s(szTmp, "%.2X ", *rBuffer.ptr<unsigned char>(__ADD(i,j)));
			else
				sprintf_s(szTmp, "   ");

			OutputDebugStringA(szTmp);

			if (j == 7)
				OutputDebugStringA(" ");
		}

		OutputDebugStringA("\t");

		for (size_t j = 0; j < 16; j++)
		{
			if (i + j < rBuffer.size())
			{
				char c = (char)*rBuffer.ptr<unsigned char>(__ADD(i,j));

				sprintf_s(szTmp, "%.c", isalpha(c) ? c : '.');
			}
			else
				sprintf_s(szTmp, " ");

			OutputDebugStringA(szTmp);

			if (j == 7)
				OutputDebugStringA(" ");
		}

		OutputDebugStringA("\r\n");
	}
}

// encode storage buffer
static auto encode(const StorageBuffer& rBuffer, unsigned long ulFlags)
{
	struct
	{
		int encoding;
		StorageBuffer buffer;
	} ret;

	// try all methods
	const size_t nNumBuffers = 3;

	StorageBuffer buffers[nNumBuffers];

	buffers[ENCODING_NONE] = rBuffer;

	for (size_t i = 1; i < nNumBuffers; i++)
		buffers[i] = buffers[0];

	if((ulFlags & FLAG_ENABLE_RLE8) != 0)
		buffers[ENCODING_RLE8] = rle8_encode(rBuffer);

	if((ulFlags & FLAG_ENABLE_RLE0) != 0)
		buffers[ENCODING_RLE0] = rle0_encode(rBuffer);

	/*
	if (rBuffer.size() < 10*1024)
	{
		if (!rle0_decode(buffers[ENCODING_RLE0]).equals(rBuffer))
		{
			OutputDebugStringA("Original:\r\n");
			OutputHex(rBuffer);
			OutputDebugStringA("\r\n");

			OutputDebugStringA("Encoded:\r\n");
			OutputHex(buffers[ENCODING_RLE0]);
			OutputDebugStringA("\r\n");

			OutputDebugStringA("Decoded:\r\n");
			OutputHex(rle0_decode(buffers[ENCODING_RLE0]));
			OutputDebugStringA("\r\n");
		}
	}
	*/

	// find best
	size_t nMinSize = buffers[0].size();
	ret.encoding = 0;

	for (size_t i = 1; i < nNumBuffers; i++)
	{
		if (buffers[i].size() < nMinSize)
		{
			nMinSize = buffers[i].size();
			ret.encoding = (int)i;
		}
	}

	// return
	ret.buffer = buffers[ret.encoding];

	return ret;
}

// decode storage buffer
static StorageBuffer decode(const StorageBuffer& rBuffer, int encoding)
{
	switch (encoding)
	{
	case ENCODING_NONE:
		return rBuffer;
		break;

	case ENCODING_RLE8:
		return rle8_decode(rBuffer);

	case ENCODING_RLE0:
		return rle0_decode(rBuffer);

	default:
		throwException(UnknownEncodingException);
	}
}