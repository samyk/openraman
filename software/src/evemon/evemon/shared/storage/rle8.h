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

#include "../utils/safe.h"

#include "storage_buffer.h"

// RLE8 encoding
static StorageBuffer rle8_encode(const StorageBuffer& rBuffer)
{
	// create temp buffer
	StorageBuffer ret;

	// browse all bytes
	for (size_t i = 0; i < rBuffer.size(); i++)
	{
		// get current byte
		auto curr = *rBuffer.ptr<unsigned char>(i);

		// n will hold number of repetitions
		size_t n = 1;

		// increment n as long as lower than 256 or buffer size
		while (__ADD(i, n) < rBuffer.size() && n < 255)
		{
			// quit if bytes do no match
			if (curr != *rBuffer.ptr<unsigned char>(__ADD(i, n)))
				break;

			// otherelse increment n
			__INC(n, (size_t)1);
		}

		// append two bytes
		ret.append<unsigned char>((unsigned char)n, false);
		ret.append<unsigned char>((unsigned char)curr, false);

		// skip N bytes
		__INC(i, n - 1);
	}

	// return new buffer
	return ret;
}

// RLE8 decode
static StorageBuffer rle8_decode(const StorageBuffer& rBuffer)
{
	// create temp buffer
	StorageBuffer ret;

	// loop through file
	for (size_t i = 0; i < rBuffer.size(); __INC(i, (size_t)2))
	{
		// get current bytes
		auto occurence = *rBuffer.ptr<unsigned char>(i);
		auto symbol = *rBuffer.ptr<unsigned char>(__ADD(i,(size_t)1));

		// add N times
		while (occurence--)
			ret.append(symbol, false);
	}

	// return buffer
	return ret;
}