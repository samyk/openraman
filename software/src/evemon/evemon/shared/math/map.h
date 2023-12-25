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

#include "vector.h"

#include <fstream>

// ArrayOutOfBoundException class
class ArrayOutOfBoundException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "";
	}
};

// template class for 2D arrays of any types
template<typename Type> class Map2D
{
public:

	// default constructor
	Map2D(void)
	{
		this->m_nWidth = 0;
		this->m_nHeight = 0;
		this->m_pData = nullptr;
	}

	// allocate width x height elements
	Map2D(size_t nWidth, size_t nHeight)
	{
		this->m_nWidth = nWidth;
		this->m_nHeight = nHeight;

		this->m_pData = new Type[__MULT(this->m_nWidth, this->m_nHeight)];
	}

	// copy constructor
	Map2D(const Map2D& rMap)
	{
		this->m_nWidth = 0;
		this->m_nHeight = 0;
		this->m_pData = nullptr;

		this->operator=(rMap);
	}

	// move operator
	Map2D(Map2D&& rMap) noexcept
	{
		this->m_nWidth = 0;
		this->m_nHeight = 0;
		this->m_pData = nullptr;

		this->operator=(std::move(rMap));
	}

	// destructor
	~Map2D(void)
	{
		clear();
	}

	// copy operator
	const Map2D<Type>& operator=(const Map2D<Type>& rMap)
	{
		// delete previous data if any
		clear();

		// allocate size
		this->m_nWidth = rMap.m_nWidth;
		this->m_nHeight = rMap.m_nHeight;

		auto nNumElements = __MULT(this->m_nWidth, this->m_nHeight);

		this->m_pData = new Type[nNumElements];

		// copy data
		for (size_t n = 0; n < nNumElements; n++)
			this->m_pData[n] = (double)rMap.m_pData[n];

		return *this;
	}

	// move operator
	const Map2D<Type>& operator=(Map2D<Type>&& rMap) noexcept
	{
		// delete previous data if any
		clear();

		// move things
		this->m_nWidth = rMap.m_nWidth;
		this->m_nHeight = rMap.m_nHeight;
		this->m_pData = rMap.m_pData;

		// clear moved object
		rMap.m_nWidth = 0;
		rMap.m_nHeight = 0;
		rMap.m_pData = nullptr;

		return *this;
	}

	void clear(void)
	{
		if (this->m_pData != nullptr)
			delete[] this->m_pData;

		this->m_pData = nullptr;
	}

	bool isValid(void) const
	{
		return this->m_pData != nullptr;
	}

	// return width
	size_t getWidth(void) const
	{
		return this->m_nWidth;
	}

	// return height
	size_t getHeight(void) const
	{
		return this->m_nHeight;
	}

	// fill map with a single value
	const Map2D<Type>& operator=(const Type fValue)
	{
		size_t nNumElements = __MULT(this->m_nWidth, this->m_nHeight);

		for (size_t n = 0; n < nNumElements; n++)
			this->m_pData[n] = fValue;

		return *this;
	}

	// apply function for each pixel
	void perpixel(std::function<Type(size_t, size_t)> func, size_t margin_x=0, size_t margin_y=0)
	{
		// skip if no function
		if (!func)
			return;

		// skip if size if below margin
		if (this->m_nWidth <= margin_x || this->m_nHeight <= margin_y)
			return;

		// browse all points
		for (size_t y = margin_y; y < (this->m_nHeight - margin_y); y++)
			for (size_t x = margin_x; x < (this->m_nWidth - margin_x); x++)
				this->m_pData[x + y * this->m_nWidth] = func(x, y);
	}

	// get pixel (non-const version)
	double& operator()(size_t x, size_t y)
	{
		// throw error if beyond dimensions
		if (x >= this->m_nWidth || y >= this->m_nHeight)
			throwException(ArrayOutOfBoundException);

		// return pixel reference
		return this->m_pData[__ADD(x, __MULT(y, this->m_nWidth))];
	}

	// get pixel (const version)
	const double operator()(size_t x, size_t y) const
	{
		// throw error if beyond dimensions
		if (x >= this->m_nWidth || y >= this->m_nHeight)
			throwException(ArrayOutOfBoundException);

		// return pixel data
		return this->m_pData[__ADD(x, __MULT(y, this->m_nWidth))];
	}

private:
	size_t m_nWidth, m_nHeight;

	Type* m_pData;
};

// image_t type is a Map2D<double> type
using image_t = Map2D<double>;

// maximum size for the median filtering kernel
#define MAX_MEDFILT2_KERNEL_SIZE		10

// median filtering, brute force algorithm
static image_t medfilt2(const image_t& rInput, size_t nKernelSize=3)
{
	// skip if kernel is below or equal to 1 (1=no effect, 0=undefined)
	if (nKernelSize <= 1)
		return rInput;

	// restrict to a maximum kernel size
	nKernelSize = min(nKernelSize, MAX_MEDFILT2_KERNEL_SIZE);

	// allocate size
	image_t ret(rInput.getWidth(), rInput.getHeight());

	// fill with zeros
	ret = 0;

	// apply function to each pixel
	int lo = -(int)(nKernelSize >> 1);
	int hi = lo + (int)nKernelSize - 1;

	ret.perpixel([&](size_t x, size_t y)
		{
			// get data around the pixel
			double temp[MAX_MEDFILT2_KERNEL_SIZE * MAX_MEDFILT2_KERNEL_SIZE];
			size_t n = 0;

			for (int yy = ((int)y + lo); yy <= ((int)y + hi); yy++)
				for (int xx = ((int)x + lo); xx <= ((int)x + hi); xx++)
					temp[n++] = rInput(bound(xx, 0, (int)rInput.getWidth() - 1), bound(yy, 0, (int)rInput.getHeight() - 1));

			// return median of array
			return ::median(temp, n);
		});

	// return map
	return ret;
}

// create a vector by summing columns of the image
static auto sum_cols(const image_t& rImage)
{
	vector_t vec(rImage.getWidth());

	for (size_t x = 0; x < rImage.getWidth(); x++)
	{
		vec[x] = rImage(x, 0);

		for (size_t y = 1; y < rImage.getHeight(); y++)
			vec[x] += rImage(x, y);
	}

	return vec;
}

// create a vector by summing rows of the image
static auto sum_rows(const image_t& rImage)
{
	vector_t vec(rImage.getHeight());

	for (size_t y = 0; y < rImage.getHeight(); y++)
	{
		vec[y] = rImage(0, y);

		for (size_t x = 1; x < rImage.getWidth(); x++)
			vec[y] += rImage(x, y);
	}

	return vec;
}

// create a vector by getting the maximum value in each column on the image
static auto max_cols(const image_t& rImage)
{
	vector_t vec(rImage.getWidth());

	for (size_t x = 0; x < rImage.getWidth(); x++)
	{
		vec[x] = rImage(x, 0);

		for (size_t y = 1; y < rImage.getHeight(); y++)
			vec[x] = max(vec[x], rImage(x, y));
	}

	return vec;
}

// create a vector by getting the maximum value in each row on the image
static auto max_rows(const image_t& rImage)
{
	vector_t vec(rImage.getHeight());

	for (size_t y = 0; y < rImage.getHeight(); y++)
	{
		vec[y] = rImage(0, y);

		for (size_t x = 1; x < rImage.getWidth(); x++)
			vec[y] = max(vec[y], rImage(x, y));
	}

	return vec;
}

// save image to bitmap
static void imsave(const image_t& rMap, const std::string& rFilename)
{
	BITMAPFILEHEADER bmp_header;
	BITMAPINFOHEADER bmp_info;

	// open file stream
	std::ofstream file(rFilename, std::ios::out | std::ios::binary | std::ios::trunc);

	// exit if cannot open file
	if (!file.is_open())
		return;

		// number of elements
		size_t nStride = rMap.getWidth();

	if (nStride % 4 != 0)
		nStride += 4 - (nStride % 4);

	// create header
	bmp_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * 4;
	bmp_header.bfReserved1 = 0;
	bmp_header.bfReserved2 = 0;
	bmp_header.bfType = 'MB';
	bmp_header.bfSize = safe_cast<DWORD>((__ADD(__MULT(nStride, rMap.getHeight()), sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * 4)));

	file.write((char*)&bmp_header, sizeof(bmp_header));

	// create info struct
	bmp_info.biBitCount = 8;
	bmp_info.biClrImportant = 0;
	bmp_info.biClrUsed = 0;
	bmp_info.biCompression = BI_RGB;
	bmp_info.biHeight = safe_cast<LONG>(rMap.getHeight());
	bmp_info.biPlanes = 1;
	bmp_info.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info.biSizeImage = 0;
	bmp_info.biWidth = safe_cast<LONG>(rMap.getWidth());
	bmp_info.biXPelsPerMeter = 2834;
	bmp_info.biYPelsPerMeter = 2834;

	file.write((char*)&bmp_info, sizeof(bmp_info));

	// write palette
	for (int i = 0; i < 256; i++)
	{
		file.put((char)i);
		file.put((char)i);
		file.put((char)i);
		file.put((char)0xff);
	}

	// write all data
	for (size_t y = 0; y < rMap.getHeight(); y++)
	{
		// write row
		for (size_t x = 0; x < rMap.getWidth(); x++)
			file.put(((char)(rMap(x,y) * 255.0)) % 256);

		// add padding
		for (size_t x = rMap.getWidth(); x < nStride; x++)
			file.put((char)0);
	}
}