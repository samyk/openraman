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
#include <vector>
#include <functional>

#include <Windows.h>

#include "../utils/singleton.h"
#include "../math/map.h"

// version should match between exe and dll
#define CAMINTERFACEVERSION     MAKEWORD(1,0)

#if _USRDLL
extern "C" _declspec(dllexport) unsigned long version(void);
#else
unsigned long version(void);
#endif

// CameraNotFoundException class
class CameraNotFoundException : public IException
{
public:
    CameraNotFoundException(const std::string& rCameraName)
    {
        this->m_sCameraName = rCameraName;
    }

    virtual std::string toString(void) const override
    {
        return std::string("Cannot find camera \"") + this->m_sCameraName + std::string("!");
    }

private:
    std::string m_sCameraName;
};

// ICamera class
class ICamera
{
public:
    virtual void load(void) = 0;
    virtual void save(void) = 0;

    virtual void init(void) = 0;
    virtual void open(void) = 0;
    virtual void close(void) = 0;

    virtual void setUserData(unsigned char* pData, size_t nSize) = 0;
    virtual void getUserData(unsigned char* pData, size_t nSize) const = 0;

    virtual void setParam(const std::string& rKey, const std::string& rValue) = 0;
    virtual std::string getParam(const std::string& rKey) const = 0;

    virtual void setExposure(double fExposureSeconds) = 0;
    virtual double getExposure(void) const = 0;
    virtual double getExposureMin(void) const = 0;
    virtual double getExposureMax(void) const = 0;

    virtual void setGain(double fGainDB) = 0;
    virtual double getGain(void) const = 0;
    virtual double getGainMin(void) const = 0;
    virtual double getGainMax(void) const = 0;

    virtual void setROI(int iHeight) = 0;
    virtual int getROI(void) const = 0;
    virtual int getMinROI(void) const = 0;
    virtual int getMaxROI(void) const = 0;

    virtual void beginAcquisition(void) const = 0;
    virtual void trigger(void) = 0;
    virtual image_t acquireImage(void) = 0;
    virtual void endAcquisition(void) const = 0;

    virtual std::string uid(void) const = 0;
};

// ICameraInterface class
class ICameraInterface
{
public:
    virtual ~ICameraInterface(void) {}

    virtual bool hasCamera(const std::string& rLabel) const = 0;
    virtual std::shared_ptr<ICamera> getCameraByName(const std::string& rLabel) const = 0;

    virtual std::vector<std::string> listCameras(void) = 0;
};

// CameraManager singleton class
class CameraManager : public Singleton<CameraManager>
{
    friend class Singleton<CameraManager>;

public:

    // destructor
    ~CameraManager(void)
    {
        // throw error message if interfaces have not been freed on app exit
        if (this->m_interfaces.size() > 0)
            MessageBox(NULL, TEXT("Application has quit but camera interfaces have not been freed!"), TEXT("critical error"), MB_ICONHAND | MB_OK);
    }

    // get current camera
    std::shared_ptr<ICamera> getCurrentCamera(void) const
    {
        return this->m_pCurrentCamera;
    }

    // set current camera
    void setCurrentCamera(const std::string& rLabel)
    {
        // save state and close current camera
        if (this->m_pCurrentCamera != nullptr)
        {
            this->m_pCurrentCamera->save();
            this->m_pCurrentCamera->close();
        }

        this->m_pCurrentCamera = nullptr;

        // skip if name is null
        if (rLabel.length() == 0)
            return;

        // get camera
        this->m_pCurrentCamera = getCameraByName(rLabel);

        // open camera, load state and init if found
        if (this->m_pCurrentCamera != nullptr)
        {
            this->m_pCurrentCamera->open();
            this->m_pCurrentCamera->load();
            this->m_pCurrentCamera->init();
        }
    }

    // list cameras accross all interfaces
    std::vector<std::string> listCameras(void)
    {
        std::vector<std::string> list;

        // browse all interfaces
        for (auto& v : this->m_interfaces)
            if (v.pInterface != nullptr)
            {
                // get list
                auto tmp = v.pInterface->listCameras();

                // append to result
                list.insert(list.end(), tmp.begin(), tmp.end());
            }

        // return list
        return list;
    }

    // load all interfaces
    void loadInterfaces(HINSTANCE hInstance)
    {
        // clear previously loaded interfaces
        clearInterfaces();

        // get path of calling exe
        char szExeFilename[MAX_PATH];

        GetModuleFileNameA(hInstance, szExeFilename, sizeof(szExeFilename));

        auto fileparts = splitFileParts(szExeFilename);

        std::string folder = fileparts.sDirectory;

        if (folder != "" && !strEndsWith(fileparts.sDirectory.c_str(), "/") && !strEndsWith(fileparts.sDirectory.c_str(), "\\"))
            folder += "/";

        // build search string
        std::string searchstring = folder + std::string("*.dll");

        // load all .dll files
        WIN32_FIND_DATAA FindFileData;

        HANDLE hFind = FindFirstFileA(searchstring.c_str(), &FindFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                // build complete filename and convert to wchar_t
                std::string sFilename = folder + std::string(FindFileData.cFileName);

                // debug or release version
#ifdef _DEBUG
                if (!strEndsWith(sFilename.c_str(), "_dbg.dll"))
                    continue;
#else
                if (strEndsWith(sFilename.c_str(), "_dbg.dll"))
                    continue;
#endif

                // load interface
                addInterface(sFilename);

            } while (FindNextFileA(hFind, &FindFileData));

            FindClose(hFind);
        }

        // check that some interfaces were created
        if (this->m_interfaces.size() == 0)
            MessageBoxA(NULL, "No camera interfaces found!", "error", MB_ICONHAND | MB_OK);
    }

    // free all interfaces
    void clearInterfaces(void)
    {
        // set camera to null
        setCurrentCamera("");

        // delete interfaces and free libraries
        for (auto& v : this->m_interfaces)
        {
            if (v.pInterface != nullptr)
                delete v.pInterface;

            if(v.hLibrary != NULL)
                FreeLibrary(v.hLibrary);
        }

        // clear list
        this->m_interfaces.clear();
    }

private:

    // empty constructor
    CameraManager(void) { }

    // get camera by name
    std::shared_ptr<ICamera> getCameraByName(const std::string& rLabel)
    {
        // browse all interfaces
        for (auto& v : this->m_interfaces)
            if (v.pInterface != nullptr && v.pInterface->hasCamera(rLabel))
                return v.pInterface->getCameraByName(rLabel);

        // otherelse throw error
        throw CameraNotFoundException(rLabel);
    }

    // add interface to list
    void addInterface(const std::string& rFilename)
    {
        // load library
        HMODULE hLibrary = LoadLibraryA(rFilename.c_str());

        // skip if null
        if (hLibrary == NULL)
            return;

        // try to add library
        do
        {
            // find pointer to version
            pfnVersion pVersionFunc = (pfnVersion)GetProcAddress(hLibrary, "version");

            if (pVersionFunc == nullptr)
                break;

            // check that version is compatible
            auto lib_version = (*pVersionFunc)();

            if (HIWORD(lib_version) != HIWORD(version()) || LOWORD(lib_version) < HIWORD(version()))
                break;

            // find pointer to interfaces creator func
            pfnCreateInterface pInterfaceFunc = (pfnCreateInterface)GetProcAddress(hLibrary, "createCameraInterface");

            if (pInterfaceFunc == nullptr)
                break;

            try
            {
                // create interface
                struct interface_s s;

                s.hLibrary = hLibrary;
                s.pInterface = (*pInterfaceFunc)();

                if (s.pInterface == nullptr)
                    break;

                // add to list
                this->m_interfaces.emplace_back(std::move(s));
            }
            catch (...)
            {
                char szTmp[512];

                sprintf_s(szTmp, "Cannot load interface \"%s\"!", rFilename.c_str());

                MessageBoxA(NULL, szTmp, "error", MB_ICONHAND | MB_OK);
            }

            // return successfully
            return;

        } while (false);

        // free library in case of issues
        FreeLibrary(hLibrary);
    }

    using pfnCreateInterface = ICameraInterface* (*)(void);
    using pfnVersion = unsigned long (*)(void);

    // structure to hold interfaces
    struct interface_s
    {
        HMODULE hLibrary;
        ICameraInterface* pInterface;
    };

    // list of interfaces
    std::vector<struct interface_s> m_interfaces;

    // current camera
    std::shared_ptr<ICamera> m_pCurrentCamera;
};