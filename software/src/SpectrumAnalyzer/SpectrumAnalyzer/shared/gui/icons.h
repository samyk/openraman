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

#include <map>
#include <string>
#include <functional>

#include <Windows.h>
#include <ShlObj.h>

#include "../utils/exception.h"
#include "../storage/storage.h"

// IconNotFoundException exception class
class IconNotFoundException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Cannot find icon!";
    }
};

// IconSetNotFoundException exception class
class IconSetNotFoundException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Icon set not found!";
    }
};

// IconNotInListException exception clalss
class IconNotInListException : public IException
{
public:
    IconNotInListException(const std::string& rName)
    {
        this->m_name = rName;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Icon \"") + this->m_name + std::string("\" was not found!");
    }

private:
    std::string m_name;
};

// InvalidIconSizeException exception class
class InvalidIconSizeException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "Unsupported icon size!";
    }
};

// types of supported size
enum class IconSize
{
    _16x16=0,
    _24x24,
    _32x32,
    _48x48,
};

// imported icon class
class ImportedIcon : public IStoreableObject
{
public:

    // implement storage mechanism
    IMPLEMENT_STORAGE;

    // default ctor
    ImportedIcon(void)
    {
        this->m_hIcon = NULL;

        this->m_file = "%SystemRoot%\\System32\\SHELL32.dll";
        this->m_nSize = 32;
        this->m_iIndex = 0;

        update();
    }

    // ctor
    ImportedIcon(const std::string& rFile, size_t nSize, int iIndex)
    {
        this->m_hIcon = NULL;

        this->m_file = rFile;
        this->m_nSize = nSize;
        this->m_iIndex = iIndex;

        update();
    }

    // copy constructor
    ImportedIcon(const ImportedIcon& rIcon) : ImportedIcon()
    {
        this->operator=(rIcon);
    }

    // move constructor
    ImportedIcon(ImportedIcon&& rIcon) : ImportedIcon()
    {
        this->operator=(std::move(rIcon));
    }

    // clear on destruction
    ~ImportedIcon(void)
    {
        NOTHROW(clear());
    }

    // copy operator
    const ImportedIcon& operator=(const ImportedIcon& rIcon)
    {
        this->m_file = rIcon.m_file;
        this->m_iIndex = rIcon.m_iIndex;
        this->m_nSize = rIcon.m_nSize;

        update();

        return *this;
    }

    // move operator
    const ImportedIcon& operator=(ImportedIcon&& rIcon)
    {
        this->m_file = rIcon.m_file;
        this->m_iIndex = rIcon.m_iIndex;
        this->m_nSize = rIcon.m_nSize;

        update();

        return *this;
    }

    // update on pop
    virtual void customPop(const StorageObject& rContainer) override
    {
        update();
    }

    // get icon
    HICON getIcon(void) const
    {
        return this->m_hIcon;
    }

    // set file
    void setFile(const std::string& rFile)
    {
        this->m_file = rFile;

        update();
    }

    // get file
    std::string getFile(void) const
    {
        return this->m_file;
    }

    // set index
    void setIndex(int iIndex)
    {
        this->m_iIndex = iIndex;

        update();
    }

    // get index
    int getIndex(void) const
    {
        return this->m_iIndex;
    }

    // set size
    void setSize(size_t nSize)
    {
        this->m_nSize = nSize;

        update();
    }

    // get size
    size_t getSize(void) const
    {
        return this->m_nSize;
    }

private:

    // update icon
    void update(void)
    {
        // clear previous
        clear();

        // convert filename
        char szPath[MAX_PATH];

        ExpandEnvironmentStringsA(this->m_file.c_str(), szPath, sizeof(szPath));

        // extract icon
        if (SHDefExtractIconA(szPath, -this->m_iIndex, 0, &this->m_hIcon, NULL, (UINT)(this->m_nSize & 0xffff)) != S_OK)
            throw IconNotFoundException();
    }

    // clear icon
    void clear(void)
    {
        if (this->m_hIcon != NULL)
            DestroyIcon(this->m_hIcon);

        this->m_hIcon = NULL;
    }

    storage_string m_file;
    int m_iIndex;
    size_t m_nSize;

    HICON m_hIcon;
};

// imported icon set class
class ImportedIconSet : public IStoreableObject
{
public:
    ImportedIconSet(IconSize eIconSize = IconSize::_24x24)
    {
        this->m_eIconSize = eIconSize;
    }

    // load from file
    void loadFromFile(const std::string& rFilename)
    {
        StorageContainer container;
        container.unpack(loadBufferFromFile(rFilename));
        
        loadFromStorageContainer(container);
    }

    // save to file
    void saveToFile(const std::string& rFilename)
    {
        StorageContainer container;

        saveToStorageContainer(container);

        container.saveToFile(rFilename);
    }

    // load from resource
    void loadFromResource(int iResourceID)
    {
        StorageContainer container;
        container.unpack(loadBufferFromResource(iResourceID));

        loadFromStorageContainer(container);
    }

    // load from registry
    void loadFromRegistry(RegistryRootKey eRootKey, const char* pszRegistryKey, const char* pszValueName)
    {
        StorageContainer container;
        container.unpack(loadBufferFromRegistry(eRootKey, pszRegistryKey, pszValueName));

        loadFromStorageContainer(container);
    }

    // save to registry
    void saveToRegistry(RegistryRootKey eRootKey, const char* pszRegistryKey, const char* pszValueName)
    {
        StorageContainer container;

        saveToStorageContainer(container);

        auto buffer = container.pack();

        saveDataToRegistry(eRootKey, pszRegistryKey, pszValueName, buffer.data(), buffer.size());
    }

    // load from storage container
    void loadFromStorageContainer(StorageContainer& rContainer)
    {
        StorageObject* pObject = rContainer.get("", "icons");

        if (pObject == nullptr)
            throw IconSetNotFoundException();

        pop(*pObject);
    }

    // save data to storage container
    void saveToStorageContainer(StorageContainer& rContainer)
    {
        StorageObject obj("", "icons");

        push(obj);

        rContainer.emplace_back(std::move(obj));
    }

    // return true if empty
    bool empty(void) const
    {
        return this->m_list.empty();
    }

    // clear
    void clear(void)
    {
        this->m_list.clear();
    }

    // remove specific item
    void remove(const std::string& rLabel)
    {
        this->m_list.erase(rLabel);
    }

    // return icon size
    size_t getIconSize(void) const
    {
        switch (this->m_eIconSize)
        {
        case IconSize::_16x16:
            return 16;

        default:
        case IconSize::_24x24:
            return 24;

        case IconSize::_32x32:
            return 32;

        case IconSize::_48x48:
            return 48;
        }
    }

    // resize icons
    void resize(IconSize eIconSize)
    {
        this->m_eIconSize = eIconSize;

        for (auto& v : this->m_list)
            v.second.setSize(getIconSize());
    }

    // add
    void addIcon(const std::string& rName)
    {
        this->m_list.emplace(std::make_pair(rName, ImportedIcon()));
    }

    // add icon
    void addIcon(const std::string& rName, const std::string& rFile, int iIndex)
    {
        this->m_list.emplace(std::make_pair(rName, ImportedIcon(rFile, getIconSize(), iIndex)));
    }

    // return true if icon exist
    bool hasIcon(const std::string& rName) const
    {
        return this->m_list.find(rName) != this->m_list.end();
    }

    // populate list
    void populate(std::function<void(const std::string&, const ImportedIcon& rIcon)> rCallback)
    {
        // skip if no callback
        if (!rCallback)
            return;

        // browse list
        for (auto& v : this->m_list)
            rCallback(v.first, v.second);
    }

    // get
    ImportedIcon& get(const std::string& rName)
    {
        // throw exception if not found
        if (!hasIcon(rName))
            throw IconNotInListException(rName);

        // return object
        return this->m_list[rName];
    }

    // get icon
    HICON getIcon(const std::string& rName) const
    {
        // throw exception if not found
        if (!hasIcon(rName))
            throw IconNotInListException(rName);

        // return HICON
        return this->m_list.find(rName)->second.getIcon();
    }

    // push to storage object
    virtual void push(StorageObject& rContainer) const
    {
        // set typename
        rContainer.setTypeName(getClassName());

        // write icon size
        rContainer.addVariable(getClassName(), "IconSize", typeid(IconSize).name(), sizeof(IconSize), (void*)&this->m_eIconSize);

        // loop all objects
        for (auto it = this->m_list.begin(); it != this->m_list.end(); it++)
        {
            auto pSubContainer = rContainer.createSubObject(getClassName(), it->first.c_str());

            if (pSubContainer != nullptr)
                it->second.push(*pSubContainer);
        }
    }

    // pop from storage object
    virtual void pop(const StorageObject& rContainer)
    {
        // clear first
        clear();

        // check typename
        if (rContainer.getTypeName() != getClassName())
            throw WrongTypeException();

        // get all children
        auto children = rContainer.getChildren();

        for (auto& v : children)
        {
            ImportedIcon icon;
            icon.pop(*v);

            this->m_list.emplace(std::make_pair(v->getVarName(), icon));
        }

        // get icon size
        if (!rContainer.readVariable(getClassName(), "IconSize", typeid(IconSize).name(), sizeof(IconSize), &this->m_eIconSize))
        {
            // if field is not set, import from first icon
            if (this->m_list.begin() != this->m_list.end())
            {
                size_t nIconSize = this->m_list.begin()->second.getSize();

                switch (nIconSize)
                {
                case 16:
                    this->m_eIconSize = IconSize::_16x16;
                    break;

                case 24:
                    this->m_eIconSize = IconSize::_24x24;
                    break;

                case 32:
                    this->m_eIconSize = IconSize::_32x32;
                    break;

                case 48:
                    this->m_eIconSize = IconSize::_48x48;
                    break;

                default:
                    throw InvalidIconSizeException();
                }
            }
            // otherelse default to 24x24
            else
                this->m_eIconSize = IconSize::_24x24;
        }
    }

private:
    std::map<std::string, ImportedIcon> m_list;
    IconSize m_eIconSize;
};