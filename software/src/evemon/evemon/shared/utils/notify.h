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

#include <memory>
#include <vector>
#include <map>
#include <functional>

// NotifyImpl class implements the observer pattern
class NotifyImpl
{
public:
	using observant_t = std::function<void(void)>;
	using observant_list_t = std::vector<observant_t>;

	// register observers
	void listen(int iEvent, observant_t callback)
	{
		// skip if callback if invalid
		if (!callback)
			return;

		// check if entries already exists
		auto it = this->m_observers.find(iEvent);

		// add to list if event is already listenned
		if (it != this->m_observers.end())
		{
			it->second.emplace_back(callback);

			return;
		}

		// create if no data yet
		this->m_observers.emplace(std::make_pair(iEvent, observant_list_t({ callback })));
	}

protected:

	// called after every notifications
	virtual void onNotify(int iEvent) {}

	// notify event
	void notify(int iEvent)
	{
		// open list
		auto it = this->m_observers.find(iEvent);

		// browse list and call functions
		if (it != this->m_observers.end())
		{
			for (auto& v : it->second)
				if (v) { v(); }
		}

		// notify owner object
		onNotify(iEvent);
	}

	// listen self
	void listen_self(int iEvent, std::function<void(NotifyImpl*)> pCallback)
	{
		listen(iEvent, std::bind(pCallback, this));
	}

private:

	// callbacks are stored in map for fast access
	std::map<int, observant_list_t> m_observers;
};

#define SELF(func)	std::bind(&func, this)