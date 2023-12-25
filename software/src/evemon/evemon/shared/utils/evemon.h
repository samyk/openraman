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

#include <mutex>

#include "exception.h"
#include "singleton.h"
#include "format.h"
#include "rlock.h"

#include "../storage/storage.h"

#include <Windows.h>

#define EVEMON_CLASSNAME    "EVEMON"

// event monitor types
enum class EventType
{
    DBG_MESSAGE = 0,
    DBG_WARNING = 1,
    DBG_ERROR = 2,
    DBG_CRITICAL = 3,
    DBG_EXCEPTION = 4,
};

/* EventData class for event monitoring */
class EventData : public IStoreableObject
{
public:
    IMPLEMENT_STORAGE;

public:
    EventData(void)
    {
        this->type = EventType::DBG_MESSAGE;
        this->filename = "";
        this->line = 0;
        this->procname = "";
        this->message = "";
        this->ltime = 0;
    }

    EventData(const EventData& rEvent)
    {
        this->operator=(rEvent);
    }

    const EventData& operator=(const EventData& rEvent)
    {
        this->type = rEvent.type;
        this->filename = rEvent.filename;
        this->line = rEvent.line;
        this->procname = rEvent.procname;
        this->message = rEvent.message;
        this->ltime = rEvent.ltime;

        return *this;
    }

    EventType type;
    unsigned int line;
    storage_string filename, procname, message;
    unsigned __int64 ltime;

    static const unsigned int VERSION = 0;
};

/* EventMonitor singleton class */
class EventMonitor : public Singleton<EventMonitor>
{
    friend class Singleton<EventMonitor>;

public:

    // nice cleanup
    ~EventMonitor(void)
    {
        this->m_hWnd = NULL;
    }

    // notify message
    void notify(EventType eEventType, const char *pszFilename, unsigned int uiLineNumber, const std::string& rMessage) const
    {
        auto filename = splitFilePath(pszFilename).sFile;

        // always localy display in debug mode
#ifdef _DEBUG
        try
        {
            std::string line = event2string(eEventType) + format(" (%s:%d): ", filename.c_str(), uiLineNumber) + rMessage + std::string("\r\n");

#ifdef _WIN32
            OutputDebugStringA(line.c_str());
#else
            fprintf(stderr, "%s", line.c_str());
#endif
        }
        catch (...) {}
#endif

        // skip if no monitor agent connected
        if (!hasMonitor())
            return;

        // skip if recursive call
        if (this->m_lock.isLocked())
            return;

        AUTORLOCK(this->m_lock);

        try
        {
            // get process info
            char szModuleName[MAX_PATH];

            GetModuleFileNameA(NULL, szModuleName, sizeof(szModuleName));

            // build event data
            EventData evdata;

            evdata.type = eEventType;
            evdata.filename = filename;
            evdata.line = uiLineNumber;
            evdata.message = rMessage;
            evdata.procname = splitFilePath(szModuleName).sFile;

            time_t tmp;
            time(&tmp);
            evdata.ltime = tmp;

            // pack to storage container
            StorageContainer container;
            StorageObject obj("", "event");

            evdata.push(obj);

            container.emplace_back(std::move(obj));

            auto buffer = container.pack(0);

            // send message
            COPYDATASTRUCT data;

            data.dwData = (int)EventData::VERSION;
            data.cbData = safe_cast<DWORD>(buffer.size());
            data.lpData = (void*)buffer.data();

            SendMessage(this->m_hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&data);
        }
        catch (...) {}
    }

    // return true if monitor is connected
    bool hasMonitor(void) const
    {
        return this->m_hWnd != NULL;
    }

    // reload event monitor
    void reload(void)
    {
#ifdef __EVENT_MONITORING__
        this->m_hWnd = FindWindow(TEXT(EVEMON_CLASSNAME), NULL);
#endif
    }

    // convert event type to a string
    static std::string event2string(EventType eEventType)
    {
        switch (eEventType)
        {
        case EventType::DBG_MESSAGE:
            return "message";

        case EventType::DBG_WARNING:
            return "warning";

        case EventType::DBG_ERROR:
            return "error";

        case EventType::DBG_CRITICAL:
            return "critical";

        case EventType::DBG_EXCEPTION:
            return "exception";

        default:
            return "unknown";
        }
    }

private:

    // find window on initialization
    EventMonitor(void)
    {
        this->m_hWnd = NULL;

        // reload connection
        reload();
    }

    HWND m_hWnd;

    mutable RecursiveLock m_lock;
};

// macro to notify events
#define _event(type, file, line, szfmt, ...)	{											                        \
									                auto p = EventMonitor::getInstance();	                        \
																			                                        \
									                if (p != nullptr)						                        \
										                p->notify(type, file, line, format(szfmt, __VA_ARGS__));	\
								                }

#define _debug(szfmt, ...)		                 _event(EventType::DBG_MESSAGE, __FILE__, __LINE__, szfmt, __VA_ARGS__);
#define _warning(szfmt, ...)	                 _event(EventType::DBG_WARNING, __FILE__, __LINE__, szfmt, __VA_ARGS__);
#define _error(szfmt, ...)		                 _event(EventType::DBG_ERROR, __FILE__, __LINE__, szfmt, __VA_ARGS__);
#define _critical(szfmt, ...)	                 _event(EventType::DBG_CRITICAL, __FILE__, __LINE__, szfmt, __VA_ARGS__);