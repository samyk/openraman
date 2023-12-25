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

#include <atomic>

#include <Windows.h>

#include "exception.h"

// InvalidThreadException class
class InvalidThreadException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Invalid thread!";
	}
};

// default time for normal closing before forcing
#define THREAD_CLOSE_TIMEOUT_DELAY		10.0

// callback function
class IThread;

void pfnRunThread(IThread* pThread);
DWORD WINAPI pfnRunThreadWinAPI(LPVOID lpParam);

// IThread
class IThread
{
	friend void pfnRunThread(IThread* pThread);

public:
	
	// default constructor
	IThread(void)
	{
		this->m_hThread = NULL;
		this->m_bQuit = false;
	}

	// default destructor
	virtual ~IThread(void)
	{
		// stop on quit
		try
		{
			stop();
		}
		catch (...) {}
	}

	// wait for thread
	bool wait(double fDelaySecond=0) const
	{
		return WaitForSingleObject(this->m_hThread, (fDelaySecond) == 0 ? INFINITE : (DWORD)(fDelaySecond * 1000.0)) == WAIT_OBJECT_0;
	}

	// start thread
	void start(void)
	{
		// skip if already started
		if (isRunning())
			return;

		// reset quit var
		this->m_bQuit = false;

		// create thread
		DWORD dwThreadID;

		this->m_hThread = CreateThread(NULL, 0, &pfnRunThreadWinAPI, (LPVOID)this, 0, &dwThreadID);

		// call onStart
		onStart();
	}

	// stop thread
	void stop(void)
	{
		// set quit to true
		this->m_bQuit = true;

		// trigger exception if thread is null (e.g. thread has already been stopped)
		if (this->m_hThread == NULL)
			throwException(InvalidThreadException);

		// wait for thread completion
		if (WaitForSingleObject(this->m_hThread, (DWORD)(THREAD_CLOSE_TIMEOUT_DELAY * 1000.0)) != WAIT_OBJECT_0)
		{
			// force thread to terminate
			TerminateThread(this->m_hThread, 0);

			// wait until done
			WaitForSingleObject(this->m_hThread, INFINITE);
		}

		// close thread
		CloseHandle(this->m_hThread);

		this->m_hThread = NULL;

		// call onStop
		onStop();
	}

	// return true if thread is running
	bool isRunning(void) const
	{
		if (this->m_hThread == NULL)
			return false;

		DWORD dwExitCode = 0;

		GetExitCodeThread(this->m_hThread, &dwExitCode);

		return dwExitCode == STILL_ACTIVE;
	}

	// sleep
	void sleep(double fDelaySeconds)
	{
		Sleep((unsigned long)(max(0, fDelaySeconds) * 1000.0));
	}

protected:

	// quit condition
	virtual bool stopCondition(void) const
	{
		return false;
	}

	// every thread MUST implement this function
	virtual void run(void) = 0;

	// called after thread has started
	virtual void onStart(void) {}

	// called after thread has stopped
	virtual void onStop(void) {}

private:

	// internal loop
	void loop(void)
	{
		// call run function as long as quit is not set to true
		while (!this->m_bQuit && !stopCondition())
			run();
	}

	HANDLE m_hThread;

	volatile std::atomic<bool> m_bQuit;
};