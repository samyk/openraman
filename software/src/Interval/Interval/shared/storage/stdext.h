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
#include <array>

#include "../utils/safe.h"

#include "storeable_object.h"

// storeable version of std::string
class storage_string : public std::string, public IStoreableObject
{
public:
	using std::string::string;
	using std::string::operator=;

	// push to storage object
	virtual void push(StorageObject &rContainer) const
	{
		size_t nSize = __ADD(length(), (size_t)1);

		// write size
		rContainer.addVariable(getClassName(), "size", "size_t", sizeof(nSize), (void*)&nSize);

		// write data
		rContainer.addVariable(getClassName(), "data", "char", nSize * sizeof(char), (void*)c_str());

		// set typename
		rContainer.setTypeName(getClassName());
	}

	// pop from storage object
	virtual void pop(const StorageObject &rContainer)
	{
		// check typename
		if (rContainer.getTypeName() != getClassName())
			throwException(WrongTypeException);

		size_t nSize = 0;

		// read size
		rContainer.readVariable(getClassName(), "size", "size_t", sizeof(nSize), (void*)&nSize);

		// allocate space and read data
		char *string = (char*)malloc(nSize);

		if(string == nullptr)
			throwException(MemoryAllocationException, nSize);

		try
		{
			rContainer.readVariable(getClassName(), "data", "char", __MULT(nSize, sizeof(char)), string);

			this->operator=(std::string(string));
		}
		catch (...)
		{
			// free temporary string
			if (string != nullptr)
				free(string);

			string = nullptr;

			throwException(UnknownVarException);
		}

		// free temporary string
		if(string != nullptr)
			free(string);
		
		string = nullptr;
	}
};

// storeable version of std::vector
template<typename Type> class storage_vector : public std::vector<Type>, public IStoreableObject
{
public:
	using std::vector<Type>::vector;
	using std::vector<Type>::operator=;

	// push to storage object
	virtual void push(StorageObject &rContainer) const
	{
		// set typename
		rContainer.setTypeName(getClassName());

		// loop all objects
		for (auto it=this->begin();it!=this->end();it++)
		{
			void *pBase = (void*)it.operator->();

			if (pBase == nullptr)
				continue;

			// write object or child
			if (is_storeable_object_t<Type>::test())
			{
				IStoreableObject *pObject = (IStoreableObject*)__ADDRESS((unsigned char*)pBase, storeable_object_ofs_t<Type>::get());
				
				auto pSubContainer = rContainer.createSubObject(getClassName(), "");

				if(pSubContainer != nullptr)
					pObject->push(*pSubContainer);
			}
			else
				rContainer.addVariable(getClassName(), "", typeid(Type).name(), sizeof(Type), pBase);
		}
	}

	// pop from storage object
	virtual void pop(const StorageObject &rContainer)
	{
		// clear first
		this->clear();

		// check typename
		if (rContainer.getTypeName() != getClassName())
			throwException(WrongTypeException);

		// get all variable/children
		if (is_storeable_object_t<Type>::test())
		{
			auto children = rContainer.getChildren();

			for (auto& v : children)
			{
				Type base;

				IStoreableObject *pObject = (IStoreableObject*)__ADDRESS((unsigned char*)&base, storeable_object_ofs_t<Type>::get());

				pObject->pop(*v);

				this->emplace_back(std::move(base));
			}
		}
		else
		{
			const std::string sVarType(typeid(Type).name());

			auto vars = rContainer.getVars();

			for (auto& v : vars)
			{
				if (v.owner != getClassName() || v.type != sVarType || v.nBytesSize != sizeof(Type))
					throwException(WrongTypeException);

				this->emplace_back(*(Type*)v.pData);
			}
		}
	}
};