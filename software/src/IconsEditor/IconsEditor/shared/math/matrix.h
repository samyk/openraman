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

#include <string>

#include "../utils/exception.h"
#include "../utils/safe.h"

#include "map.h"
#include "vector.h"

// MatrixSizeMismatchException exception class
class MatrixSizeMismatchException : public IException
{
public:
	MatrixSizeMismatchException(size_t nRows1, size_t nColumns1, size_t nRows2, size_t nColumns2)
	{
		this->m_nRows1 = nRows1;
		this->m_nColumns1 = nColumns1;

		this->m_nRows2 = nRows2;
		this->m_nColumns2 = nColumns2;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[128];

		sprintf_s(szTmp, "Cannot perform operation with matrices %zux%zu and %zux%zu!", this->m_nColumns1, this->m_nRows1, this->m_nColumns2, this->m_nRows2);

		return std::string(szTmp);
	}

private:
	size_t m_nRows1, m_nColumns1, m_nRows2, m_nColumns2;
};

// MatrixWrongSizeException exception class
class MatrixWrongSizeException : public IException
{
public:
	MatrixWrongSizeException(size_t nRows, size_t nColumns)
	{
		this->m_nRows = nRows;
		this->m_nColumns = nColumns;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[128];

		sprintf_s(szTmp, "Cannot perform operation on %zux%zu matrix!", this->m_nColumns, this->m_nRows);

		return std::string(szTmp);
	}

private:
	size_t m_nRows, m_nColumns;
};

// InvalidRowException exception class
class InvalidRowException : public IException
{
public:
	InvalidRowException(size_t nRow)
	{
		this->m_nRow = nRow;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[128];

		sprintf_s(szTmp, "Invalid row #%zu!", this->m_nRow);

		return std::string(szTmp);
	}

private:
	size_t m_nRow;
};

// InvalidColumnException exception class
class InvalidColumnException : public IException
{
public:
	InvalidColumnException(size_t nColumn)
	{
		this->m_nColumn = nColumn;
	}

	virtual std::string toString(void) const override
	{
		char szTmp[128];

		sprintf_s(szTmp, "Invalid column #%zu!", this->m_nColumn);

		return std::string(szTmp);
	}

private:
	size_t m_nColumn;
};

// MatrixNotInvertibleException exception class
class MatrixNotInvertibleException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Matrix is not invertible!";
	}
};

// Matrix class is built on top of Map2D
class Matrix : public Map2D<double>
{
public:
	Matrix(void) : Map2D<double>() {}

	// allows to move a map into matrix type for fast upgrade
	Matrix(Map2D<double>&& rrMap) : Map2D<double>(std::move(rrMap)) {}

	Matrix(size_t nColumns, size_t nRows) : Map2D<double>(nColumns, nRows)
	{
		// initialize with zeros
		this->operator=(0);
	}

	// add matrix
	const Matrix& operator+=(const Matrix& rMap)
	{
		if (rMap.getWidth() != getWidth() || rMap.getHeight() != getHeight())
			throw MatrixSizeMismatchException(rMap.getWidth(), rMap.getHeight(), getWidth(), getHeight());

		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
			this->operator()(x,y) += rMap.operator()(x, y);

		return *this;
	}

	// add constant
	const Matrix& operator+=(const double fValue)
	{
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) += fValue;

		return *this;
	}

	// subtract matrix
	const Matrix& operator-=(const Matrix& rMap)
	{
		if (rMap.getWidth() != getWidth() || rMap.getHeight() != getHeight())
			throw MatrixSizeMismatchException(rMap.getWidth(), rMap.getHeight(), getWidth(), getHeight());

		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) -= rMap.operator()(x, y);

		return *this;
	}

	// subtract constant
	const Matrix& operator-=(const double fValue)
	{
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) -= fValue;

		return *this;
	}

	// multiplication by constant
	const Matrix& operator*=(const double fValue)
	{
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) *= fValue;

		return *this;
	}

	// division by constant
	const Matrix& operator/=(const double fValue)
	{
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) /= fValue;

		return *this;
	}

	// assignment
	const Matrix& operator=(const double fValue)
	{
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				this->operator()(x, y) = fValue;

		return *this;
	}

	// transpose matrix
	auto transpose(void) const
	{
		Matrix ret(getHeight(), getWidth());

		for (size_t i = 0; i < getWidth(); i++)
			for (size_t j = 0; j < getHeight(); j++)
				ret(j, i) = this->operator()(i, j);

		return ret;
	}

	// cofactor matrix
	auto comatrix(size_t col, size_t row) const
	{
		// cannot apply on null matrix
		if (getWidth() == 0 || getHeight() == 0)
			throw MatrixWrongSizeException(numRows(), numColumns());

		if (col >= numColumns())
			throw InvalidColumnException(col);

		if (row >= numRows())
			throw InvalidRowException(row);

		// prepare output matrix
		Matrix ret(getWidth() - 1, getHeight() - 1);

		// remove column and row
		for (size_t y = 0; y < numRows(); y++)
		{
			if (y == row)
				continue;

			for (size_t x = 0; x < numColumns(); x++)
			{
				if (x == col)
					continue;

				size_t i = (x < col) ? x : x - 1;
				size_t j = (y < row) ? y : y - 1;

				ret(i, j) = this->operator()(x, y);
			}
		}

		// return matrix
		return ret;
	}

	// compute determinant
	double determinant(void) const
	{
		// trigger error if at least one dimension is null
		if (numRows() == 0 || numColumns() == 0)
			throw MatrixWrongSizeException(numRows(), numColumns());

		// special case for size 1
		if (numRows() == 1 && numColumns() == 1)
			return this->operator()(0, 0);

		// otherelse apply formula by browsing columns
		double fSum = 0;

		for (size_t x = 0; x < numColumns(); x++)
		{
			// compute minor of current column
			auto fTemp = comatrix(x, 0).determinant();

			// add to sum
			if ((x % 2) == 0)
				fSum += this->operator()(x, 0) * fTemp;
			else
				fSum -= this->operator()(x, 0) * fTemp;
		}

		// return sum
		return fSum;
	}

	// compute minor matrix
	auto minor(void) const
	{
		// prepare output
		Matrix ret(getWidth(), getHeight());

		// browse all items
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				ret(x,y) = comatrix(x, y).determinant();

		// return matrix
		return ret;
	}

	// compute cofactor matrix
	auto cofactor(void) const
	{
		// compute minor matrix first
		auto ret = minor();

		// change sign of odd elements
		for (size_t y = 0; y < getHeight(); y++)
			for (size_t x = 0; x < getWidth(); x++)
				if (((x + y) % 2) != 0)
					ret(x, y) = -ret(x, y);

		// return matrix
		return ret;
	}

	// extract row
	vector_t extractRow(size_t row) const
	{
		if (row >= numRows())
			throw InvalidRowException(row);

		vector_t ret(numColumns());

		for (size_t x = 0; x < numColumns(); x++)
			ret[x] = this->operator()(x, row);

		return ret;
	}

	// extract column
	vector_t extractColumn(size_t col) const
	{
		if (col >= numColumns())
			throw InvalidColumnException(col);

		vector_t ret(numRows());

		for (size_t y = 0; y < numRows(); y++)
			ret[y] = this->operator()(col, y);

		return ret;
	}

	// return number of rows
	size_t numRows(void) const
	{
		return getHeight();
	}

	// return number of columns
	size_t numColumns(void) const
	{
		return getWidth();
	}
};

// addition of two matrix
static auto operator+(const Matrix& rA, const Matrix& rB)
{
	if (rA.numRows() != rB.numRows() || rA.numColumns() != rB.numColumns())
		throw MatrixSizeMismatchException(rA.numRows(), rA.numColumns(), rB.numRows(), rB.numColumns());

	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) + rB(i, j);

	return ret;
}

// addition of matrix and constant
static auto operator+(const Matrix& rA, const double fValue)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) + fValue;

	return ret;
}

// addition of constant and matrix
static auto operator+(const double fValue, const Matrix& rA)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = fValue + rA(i, j);

	return ret;
}

// subtraction of two matrix
static auto operator-(const Matrix& rA, const Matrix& rB)
{
	if (rA.numRows() != rB.numRows() || rA.numColumns() != rB.numColumns())
		throw MatrixSizeMismatchException(rA.numRows(), rA.numColumns(), rB.numRows(), rB.numColumns());

	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) - rB(i, j);

	return ret;
}

// subtraction and matrix and constant
static auto operator-(const Matrix& rA, const double fValue)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) - fValue;

	return ret;
}

// subtraction of constant and matrix
static auto operator-(const double fValue, const Matrix& rA)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = fValue - rA(i, j);

	return ret;
}

// multiplication of two matrix
static auto operator*(const Matrix& rA, const Matrix& rB)
{
	if (rA.numColumns() != rB.numRows())
		throw MatrixSizeMismatchException(rA.numRows(), rA.numColumns(), rB.numRows(), rB.numColumns());

	Matrix ret(rA.numRows(), rB.numColumns());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
		{
			ret(i, j) = 0;

			for (size_t k = 0; k < rA.numColumns(); k++)
				ret(i, j) += rA(k, i) * rB(j, k);
		}

	return ret;
}

// multiplication matrix and vector
static auto operator*(const Matrix& rMatrix, const vector_t& rVector)
{
	if (rMatrix.numRows() != rVector.size())
		throw MatrixSizeMismatchException(rMatrix.numRows(), rMatrix.numColumns(), 1, rVector.size());

	vector_t ret(rMatrix.numColumns());

	for (size_t i = 0; i < ret.size(); i++)
	{
		ret[i] = 0;

		for (size_t k = 0; k < rMatrix.numRows(); k++)
			ret[i] += rMatrix(i, k) * rVector[k];
	}

	return ret;
}

// multiplication vector and matrix
static auto operator*(const vector_t& rVector, const Matrix& rMatrix)
{
	return rMatrix.transpose() * rVector;
}

// multiplication matrix and constant
static auto operator*(const Matrix& rA, const double fValue)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) * fValue;

	return ret;
}

// multiplication constant and matrix
static auto operator*(const double fValue, const Matrix& rA)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = fValue * rA(i, j);

	return ret;
}

// division matrix and constant
static auto operator/(const Matrix& rA, const double fValue)
{
	Matrix ret(rA.numColumns(), rA.numRows());

	for (size_t i = 0; i < ret.numColumns(); i++)
		for (size_t j = 0; j < ret.numRows(); j++)
			ret(i, j) = rA(i, j) / fValue;

	return ret;
}

// convert 1D vector to 2D matrix
static auto reshape(const vector_t& rVector, size_t nRows, size_t nColumns)
{
	// NullVectorException exception class
	class NullVectorException : public IException
	{
	public:
		virtual std::string toString(void) const override
		{
			return "Vector is null";
		}
	};

	// WrongDimensionVectorException exception class
	class WrongDimensionVectorException : public IException
	{
	public:
		WrongDimensionVectorException(size_t nRows, size_t nColumns, size_t nNumElements)
		{
			this->m_nRows = nRows;
			this->m_nColumns = nColumns;
			this->m_nNumElements = nNumElements;
		}

		virtual std::string toString(void) const override
		{
			char szTmp[512];

			sprintf_s(szTmp, "Cannot map %zu elements as %zux%zu matrix!", this->m_nNumElements, this->m_nRows, this->m_nColumns);

			return std::string(szTmp);
		}

	private:
		size_t m_nRows, m_nColumns, m_nNumElements;
	};

	if (rVector.size() == 0)
		throw NullVectorException();

	if (rVector.size() != __MULT(nRows, nColumns))
		throw WrongDimensionVectorException(nRows, nColumns, rVector.size());

	Matrix ret(nRows, nColumns);

	for (size_t j = 0; j < nColumns; j++)
		for (size_t i = 0; i < nRows; i++)
			ret(i, 0) = rVector[__ADD(__MULT(j, nRows), i)];

	return ret;
}

// convert 1D vector to row matrix
static auto vec2rows(const vector_t& rVector)
{
	return reshape(rVector, rVector.size(), 1);
}

// convert 1D vector to column matrix
static auto vec2cols(const vector_t& rVector)
{
	return reshape(rVector, 1, rVector.size());
}

// transpose of matrix
static auto transpose(const Matrix& rMatrix)
{
	return rMatrix.transpose();
}

// inverse of matrix
static auto inv(const Matrix& rMatrix)
{
	// matrix must be square
	if (rMatrix.numRows() != rMatrix.numColumns())
		throw MatrixNotInvertibleException();

	// simple value case
	if (rMatrix.numRows() == 1)
	{
		Matrix ret(1, 1);

		if (fabs(rMatrix(0, 0)) < 1e-12)
			throw MatrixNotInvertibleException();

		ret(0, 0) = 1.0 / rMatrix(0, 0);

		return ret;
	}

	// compute determinant
	auto fDeterminant = rMatrix.determinant();

	// trigger error if too small
	if (fabs(fDeterminant) < 1e-12)
		throw MatrixNotInvertibleException();

	// compute inverse
	return rMatrix.cofactor().transpose() / fDeterminant;
}