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

#include "exception.h"

// SafeCastException exception class
class SafeCastException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "safe_cast operation failed!";
    }
};

// generic safe cast will not compile
template<typename Type1, typename Type2> Type1 safe_cast(const Type2 value)
{
    auto ret = (Type1)value;

    if ((Type2)ret != value)
        throw SafeCastException();

    return ret;
}

#define __SIZE_T(value) safe_cast<size_t>(value)
#define __PTRDIFF_T(value) safe_cast<ptrdiff_t>(value)
#define __UINTPTR_T(value) safe_cast<uintptr_t>(value)

#define __WCHAR_T(value) safe_cast<wchar_t>(value)

#define __SHORT(value) safe_cast<short>(value)
#define __INT(value) safe_cast<int>(value)
#define __LONG(value) safe_cast<long>(value)
#define __LONGLONG(value) safe_cast<long long>(value)

#define __USHORT(value) safe_cast<unsigned short>(value)
#define __UINT(value) safe_cast<unsigned int>(value)
#define __ULONG(value) safe_cast<unsigned long>(value)
#define __ULONGLONG(value) safe_cast<unsigned long long>(value)

#define __INT8(value) safe_cast<__int8>(value)
#define __INT16(value) safe_cast<__int16>(value)
#define __INT32(value) safe_cast<__int32>(value)
#define __INT64(value) safe_cast<__int64>(value)
#define __INT128(value) safe_cast<__int128>(value)

#define __UINT8(value) safe_cast<unsigned __int8>(value)
#define __UINT16(value) safe_cast<unsigned __int16>(value)
#define __UINT32(value) safe_cast<unsigned __int32>(value)
#define __UINT64(value) safe_cast<unsigned __int64>(value)
#define __UINT128(value) safe_cast<unsigned __int128>(value)

// SafeAddException exception class
class SafeAddException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "safe addition operation failed!";
    }
};

template<typename Type> Type unsigned_max_val(void)
{
    static_assert(false, "Type not implemented!");
};

#define DEFINE_UINT_MAXVAL(Type, MaxValue)  template<> inline Type unsigned_max_val(void)     \
                                            {                                                 \
                                                return MaxValue;                              \
                                            }

DEFINE_UINT_MAXVAL(unsigned char, UCHAR_MAX)
DEFINE_UINT_MAXVAL(unsigned short, USHRT_MAX)
DEFINE_UINT_MAXVAL(unsigned int, UINT_MAX)
DEFINE_UINT_MAXVAL(unsigned long, ULONG_MAX)
DEFINE_UINT_MAXVAL(unsigned long long, ULLONG_MAX)

template<typename Type> Type safe_add(const Type val_a, const Type val_b)
{
    if (val_a >= unsigned_max_val<Type>() - val_b)
        throw SafeAddException();
    
    return val_a + val_b;
}

#define __ADD(val_a, val_b) safe_add(val_a, val_b)
#define __INC(val_a, val_b) val_a = __ADD(val_a, val_b)

// SafeSubtractException exception class
class SafeSubtractException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "safe subtraction operation failed!";
    }
};

template<typename Type> Type safe_sub(const Type val_a, const Type val_b)
{
    if (val_a < val_b)
        throw SafeAddException();

    return val_a - val_b;
}

#define __SUB(val_a, val_b) safe_sub(val_a, val_b)
#define __DEC(val_a, val_b) val_a = __SUB(val_a, val_b)

// SafeMultiplicationException exception class
class SafeMultiplicationException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "safe multiplication operation failed!";
    }
};

// safe multiplication
template<typename Type> Type safe_mult(const Type val_a, const Type val_b)
{
    auto countbits = [](Type value)
    {
        size_t i = 0;

        while (((Type)1 << i) < value && (i+1) < 8 * sizeof(Type))
            i++;

        return i;
    };

    if (countbits(val_a) + countbits(val_b) > 8 * sizeof(Type))
        throw SafeMultiplicationException();

    return val_a * val_b;
}

#define __MULT(val_a, val_b) safe_mult(val_a, val_b)

// SafeAddressException exception class
class SafeAddressException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "safe address operation failed!";
    }
};

// safe address computation
template<typename Type> Type* safe_address(const Type *pBase, const uintptr_t nOffset)
{
    if ((uintptr_t)pBase >= UINTPTR_MAX - nOffset)
        throw SafeAddressException();

    return (Type*)((unsigned char*)pBase + nOffset);
}

#define __ADDRESS(base, ofs) safe_address(base, __UINTPTR_T(ofs))