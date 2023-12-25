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

 // change endianness for camera data
unsigned short myhtons(unsigned short val)
{
#ifdef BIGENDIAN_SYSTEM
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
#else
    return val;
#endif
}

// root nodemap types
enum class RootNodeMap
{
    Camera,
    TransportLayer,
    TLStream,
};

// access flags for properties
#define ACCESS_READ			(1 << 0)
#define ACCESS_WRITE		(1 << 1)
#define ACCESS_ALL			(ACCESS_READ | ACCESS_WRITE)