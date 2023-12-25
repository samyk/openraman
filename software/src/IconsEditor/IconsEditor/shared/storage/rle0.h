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

#include "rle8.h"

// RLE0 encoding
static StorageBuffer rle0_encode(const StorageBuffer& rBuffer)
{
	// create temp buffer
	StorageBuffer ret;

	// apply RLE8 first
	auto rle8 = rle8_encode(rBuffer);

	// current block size
	size_t nCurrBlockSize = 1;

	// scan RLE8 buffer
	size_t i = 0;

	while (i < rle8.size())
	{
		// unecoded region
		if (*rle8.ptr<unsigned char>(i) == 1)
		{
			// temp buffer for unencoded data
			StorageBuffer unencoded;

			// scan next tokens
			size_t j = 0;

			while (__ADD(i,(j << 1)) < rle8.size() && j < 255)
			{
				auto tmp = __ADD(i, (j << 1));

				// skip if not unencoded anymore
				if (*rle8.ptr<unsigned char>(tmp) != 1)
					break;

				// add to unencoded buffer
				unencoded.append(rle8.ptr<unsigned char>(__ADD(tmp, (size_t)1)), 1, false);

				// increment j
				__INC(j, (size_t)1);
			}

			// number of bytes to write
			size_t nWriteSize = unencoded.size();

			// evaluate if it worth replacing
			size_t nOriginalBytes = unencoded.size() << 1;
			size_t nNewBytes = __ADD(unencoded.size(), (size_t)3);

			// penalty if block size differs
			if (nWriteSize != nCurrBlockSize)
				__INC(nNewBytes, (size_t)2);

			// replace if better
			if (nNewBytes <= nOriginalBytes)
			{
				// check if current block size differs from write size
				if (nCurrBlockSize != nWriteSize)
				{
					// push command
					ret.append<unsigned char>(0, false);
					ret.append<unsigned char>((unsigned char)nWriteSize, false);

					nCurrBlockSize = nWriteSize;
				}

				// push content
				ret.append<unsigned char>(1, false);
				ret.append(unencoded.data(), nWriteSize, false);
			}
			// copy identical otherwise
			else
			{
				// restore 1 bytes block size
				if (nCurrBlockSize != 1)
				{
					ret.append<unsigned char>(0, false);
					ret.append<unsigned char>(1, false);

					nCurrBlockSize = 1;
				}

				ret.append(rle8.ptr<unsigned char>(i), nOriginalBytes, false);
			}

			// skip bytes
			__INC(i, nOriginalBytes);
		}
		// otherelse copy data
		else
		{
			// restore 1 bytes block size
			if (nCurrBlockSize != 1)
			{
				ret.append<unsigned char>(0, false);
				ret.append<unsigned char>(1, false);

				nCurrBlockSize = 1;
			}

			// copy data
			ret.append(rle8.ptr<unsigned char>(i), 2, false);

			__INC(i, (size_t)2);
		}
	}

	// return new buffer
	return ret;
}

// RLE0 decode
static StorageBuffer rle0_decode(const StorageBuffer& rBuffer)
{
	// create temp buffer
	StorageBuffer ret;

	// current block size
	size_t nCurrBlockSize = 1;

	// loop through file
	size_t i = 0;

	while (i < rBuffer.size())
	{
		// get occurence
		auto occurence = *rBuffer.ptr<unsigned char>(i++);

		// check if command
		if (occurence == 0)
		{
			// read new block size
			nCurrBlockSize = (size_t)*rBuffer.ptr<unsigned char>(i++);

			// read occurence
			occurence = *rBuffer.ptr<unsigned char>(i++);
		}

		// get current block
		StorageBuffer block(rBuffer.ptr<unsigned char>(i), nCurrBlockSize);

		// add N times
		while (occurence--)
			ret.append(block, false);

		// increment index
		i += nCurrBlockSize;
	}

	// return buffer
	return ret;
}