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

#include <Windows.h>

#include "exception.h"

// InvalidEventException class
class InvalidEventException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "unknown event error!";
	}
};

// Event class
class Event
{
public:
	
	// default constructor
	Event(void)
	{
		// create event
		this->m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	// default destructor
	~Event(void)
	{
		// close handle
		if(this->m_hEvent != NULL)
			CloseHandle(this->m_hEvent);

		this->m_hEvent = NULL;
	}

	// wait for the event, return false if timeout
	bool wait(double fTimeoutDelay = 0) volatile const
	{
		// throw exception for bad events, should never happen
		if (this->m_hEvent == NULL)
			throw InvalidEventException();

		// get timeout
		DWORD dwTimeout = INFINITE;

		if (fTimeoutDelay > 0)
			dwTimeout = (DWORD)(fTimeoutDelay * 1000.0);

		// check status
		return WaitForSingleObject(this->m_hEvent, dwTimeout) == WAIT_OBJECT_0;
	}

	// trigger the event
	void trigger(void)
	{
		// throw exception for bad events, should never happen
		if (this->m_hEvent == NULL)
			throw InvalidEventException();

		// set status
		SetEvent(this->m_hEvent);
	}

	// reset the event
	void reset(void)
	{
		// throw exception for bad events, should never happen
		if (this->m_hEvent == NULL)
			throw InvalidEventException();

		// reset status
		ResetEvent(this->m_hEvent);
	}

	// return true if the event is pending
	bool isPending(void) const
	{
		// throw exception for bad events, should never happen
		if (this->m_hEvent == NULL)
			throw InvalidEventException();

		// check status
		return WaitForSingleObject(this->m_hEvent, 0) != WAIT_OBJECT_0;
	}

private:
	HANDLE m_hEvent;
};