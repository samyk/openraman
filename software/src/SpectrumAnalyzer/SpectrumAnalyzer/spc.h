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
#include <memory>

#include "shared/math/vector.h"

#include "shared/storage/storage.h"

#include "exception.h"

 // SPC File
class SpectreFile : public IStoreableObject
{
public:
    // implement storage mechanism
    IMPLEMENT_STORAGE;

    // default constructor
    SpectreFile(void) {}

    // construct from data
    SpectreFile(const vector_t& rData, const vector_t& rBlank, const std::string& rUID)
    {
        this->m_data = rData;
        this->m_blank = rBlank;
        this->m_uid = rUID;
    }

    // get data
    vector_t getData(void) const
    {
        return this->m_data;
    }

    // get blank
    vector_t getBlank(void) const
    {
        return this->m_blank;
    }

    // get UID
    std::string getUID(void) const
    {
        return this->m_uid;
    }

private:
    storage_vector<double> m_data, m_blank;
    storage_string m_uid;
};