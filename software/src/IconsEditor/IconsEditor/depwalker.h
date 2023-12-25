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
#include <vector>
#include <map>
#include <functional>

#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>

#include "shared/utils/exception.h"

 // callback to enumerate resource from file
using string_list = std::vector<int>;

BOOL CALLBACK EnumResourceCallback(HMODULE hModule, LPCSTR pszType, LPSTR pszName, LONG_PTR lParam);

// InvalidModuleException exception class
class InvalidModuleException : public IException
{
public:
    InvalidModuleException(const std::string& rLibrary)
    {
        this->m_sLibrary = rLibrary;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Cannot load library \"") + this->m_sLibrary + std::string("\"!");
    }

private:
    std::string m_sLibrary;
};

// LibraryNotFoundException
class LibraryNotFoundException : public IException
{
public:
    LibraryNotFoundException(const std::string& rLibrary)
    {
        this->m_sLibrary = rLibrary;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Library \"") + this->m_sLibrary + std::string("\" not found!");
    }

private:
    std::string m_sLibrary;
};

// EnumResourcesException exception class
class EnumResourcesException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Cannot enumerate resources!";
    }
};

// IconDependencyWalker class
class IconDependencyWalker
{
public:
    class Library
    {
    public:

        // default ctor
        Library(void)
        {
            this->m_hModule = NULL;
        }

        // ctor
        Library(const std::string& rLibrary)
        {
            // load library
            this->m_hModule = LoadLibraryA(rLibrary.c_str());

            if (this->m_hModule == NULL)
                throw InvalidModuleException(rLibrary);

            this->m_library = rLibrary;
        }

        // copy ctor
        Library(const Library& rLibrary)
        {
            this->m_hModule = NULL;

            this->operator=(rLibrary);
        }

        // move ctor
        Library(Library&& rLibrary)
        {
            this->m_hModule = NULL;

            this->operator=(std::move(rLibrary));
        }

        // dtor
        ~Library(void)
        {
            // clear current icons
            clear();

            // free library
            if (this->m_hModule != NULL)
                FreeLibrary(this->m_hModule);

            this->m_hModule = NULL;
        }

        // copy operator
        const Library& operator=(const Library& rLibrary)
        {
            // free library if existing
            if (this->m_hModule != NULL)
                FreeLibrary(this->m_hModule);

            this->m_hModule = NULL;

            this->m_data = rLibrary.m_data;
            this->m_library = rLibrary.m_library;

            return *this;
        }

        // move operator
        const Library& operator=(Library&& rLibrary)
        {
            // free library if existing
            if (this->m_hModule != NULL)
                FreeLibrary(this->m_hModule);

            this->m_hModule = rLibrary.m_hModule;
            rLibrary.m_hModule = NULL;

            this->m_data = std::move(rLibrary.m_data);
            this->m_library = std::move(rLibrary.m_library);

            return *this;
        }

        // clear
        void clear(void)
        {
            // clear icons allocated
            for (auto& v : this->m_data)
                DestroyIcon(v.hIcon);

            this->m_data.clear();
        }

        // return true if has icons
        bool hasIcons(void) const
        {
            return this->m_data.size() > 0;
        }

        // import icons
        void importIcons(const size_t nIconSize)
        {
            // clear current icons
            clear();

            // enumerate resources
            string_list list;

            if (EnumResourceNamesA(this->m_hModule, MAKEINTRESOURCEA(3), &EnumResourceCallback, (LONG_PTR)&list) != TRUE)
                throw EnumResourcesException();

            // convert to HICON list
            for (auto& v : list)
            {
                // try to load icon
                struct data_s s;

                s.hIcon = NULL;
                s.iIndex = v;

                if (SHDefExtractIconA(this->m_library.c_str(), -v, 0, &s.hIcon, NULL, (UINT)nIconSize) == S_OK)
                    this->m_data.emplace_back(std::move(s));
            }
        }

        // populate data
        void populate(std::function<void(int, HICON)> callback)
        {
            // skip invalid callbacks
            if (!callback)
                return;

            // browse list
            for (auto& v : this->m_data)
                callback(v.iIndex, v.hIcon);
        }

    private:
        struct data_s
        {
            int iIndex;
            HICON hIcon;
        };

        std::vector<struct data_s> m_data;

        HMODULE m_hModule;
        std::string m_library;
    };

    // clear all
    void clear(void)
    {
        this->m_list.clear();
    }

    // add library
    void addLibrary(const std::string& rLibrary)
    {
        // skip if already existing
        if (hasLibrary(rLibrary))
            return;

        // add library
        this->m_list.emplace(std::make_pair(rLibrary, Library(rLibrary)));
    }

    // return true if library exists
    bool hasLibrary(const std::string& rLibrary)
    {
        return this->m_list.find(rLibrary) != this->m_list.end();
    }

    // get library
    Library& getLibrary(const std::string& rLibrary)
    {
        if (!hasLibrary(rLibrary))
            throw LibraryNotFoundException(rLibrary);

        return this->m_list[rLibrary];
    }

private:
    std::map<std::string, Library> m_list;
};
