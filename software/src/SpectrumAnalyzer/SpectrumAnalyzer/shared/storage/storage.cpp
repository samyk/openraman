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
#include "storage.h"

#include <string>

// store object
void IStoreableObject :: store(const std::string& rFilename) const
{
	// push to storage object
	StorageObject obj;
	push(obj);

	// add storage object to container
	StorageContainer list;
	list.emplace_back(std::move(obj));

	// save container
	list.saveToFile(rFilename);
}

// restore object
void IStoreableObject::restore(const std::string& rFilename)
{
	// load container
	StorageContainer list;

	list.unpack(loadBufferFromFile(rFilename));

	// check that container has exactly 1 entry
	if (list.size() != 1)
		throw MultipleContainerException();

	// restore data front content
	pop(list.front());
}