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

#include "../utils/utils.h"
#include "../utils/exception.h"

#include "storage_object.h"

#include <vector>
#include <string>
#include <ios>
#include <fstream>

 // NotAStoreableObjectException exception class
class NotAStoreableObjectException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Object is not an IStoreableObject!";
	}
};

// WrongTypeException exception class
class WrongTypeException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Incorrect typename!";
	}
};

// UnknownVarException exception class
class UnknownVarException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Unknown variable";
	}
};

// SFINAE to check if storeable object
template<typename Type> struct is_storeable_object_t
{
	static bool check(IStoreableObject*)
	{
		return true;
	}

	static bool check(...)
	{
		return false;
	}

	static bool test(void)
	{
		return check(static_cast<Type*>(nullptr));
	}
};

template<typename Type> static bool is_storeable_object(const Type&)
{
	return is_storeable_object_t<Type> ::test();
}

// get base offset of multiple-inheritance classes
template<class Class, class Parent> size_t getBaseOfs(void)
{
	Class tmp;

	Parent *pParent = static_cast<Parent*>(&tmp);

	return (size_t)((unsigned char*)pParent - (unsigned char*)&tmp);
}

// structure to hold storage list
struct storage_list_s
{
	const char *szOwner;
	const char *szName;
	const char *szType;

	size_t nBytesSize;
	size_t nBaseOffset;

	bool bIsObject;
	size_t nObjectBaseOffset;
};

// IStoreableObject class
class IStoreableObject
{
public:
	virtual std::string getClassName(void) const
	{
		return typeid(*this).name();
	}

	virtual void push(StorageObject &rContainer) const = 0;
	virtual void pop(const StorageObject &rContainer) = 0;

	virtual void customPush(StorageObject &rContainer) const { }
	virtual void customPop(const StorageObject &rContainer) { }

	void store(const std::string& rFilename) const;
	void restore(const std::string& rFilename);

	// generic push method
	void generic_push(StorageObject &rContainer, const void *pBasePtr, struct storage_list_s *pStorageList, size_t nNumElements) const
	{
		if (pStorageList == nullptr)
			return;

		rContainer.setTypeName(getClassName());

		for (size_t i = 0; i < nNumElements; i++)
		{
			if (pStorageList[i].bIsObject)
			{
				IStoreableObject *pSubObject = ((IStoreableObject*)(__ADDRESS((unsigned char*)pBasePtr, __ADD(pStorageList[i].nBaseOffset, pStorageList[i].nObjectBaseOffset))));

				auto pSubContainer = rContainer.createSubObject(pStorageList[i].szOwner, pStorageList[i].szName);

				if (pSubContainer != nullptr)
					pSubObject->push(*pSubContainer);
			}
			else
				rContainer.addVariable(pStorageList[i].szOwner, pStorageList[i].szName, pStorageList[i].szType, pStorageList[i].nBytesSize, (void*)__ADDRESS((unsigned char*)pBasePtr, pStorageList[i].nBaseOffset));
		}

		customPush(rContainer);
	}

	// generic pop method
	void generic_pop(const StorageObject &rContainer, const void *pBasePtr, struct storage_list_s *pStorageList, size_t nNumElements)
	{
		if (pStorageList == nullptr)
			return;

		if (rContainer.getTypeName() != getClassName())
			throw WrongTypeException();

		for (size_t i = 0; i < nNumElements; i++)
		{
			if (!pStorageList[i].bIsObject)
			{
				if (!rContainer.readVariable(pStorageList[i].szOwner, pStorageList[i].szName, pStorageList[i].szType, pStorageList[i].nBytesSize, (void*)__ADDRESS((unsigned char*)pBasePtr, pStorageList[i].nBaseOffset)))
					throw UnknownVarException();
			}
			else
			{
				auto pSubContainer = rContainer.getSubObject(pStorageList[i].szOwner, pStorageList[i].szName);

				if (pSubContainer == nullptr)
					throw UnknownVarException();

				IStoreableObject *pSubObject = ((IStoreableObject*)__ADDRESS((unsigned char*)pBasePtr, __ADD(pStorageList[i].nBaseOffset, pStorageList[i].nObjectBaseOffset)));

				pSubObject->pop(*pSubContainer);
			}
		}

		customPop(rContainer);
	}
};

// SFINAE for offset to storeable object
template<typename Type> struct storeable_object_ofs_t
{
	static size_t getOfs(IStoreableObject*)
	{
		return getBaseOfs<Type, IStoreableObject>();
	}

	static size_t getOfs(...)
	{
		return 0;
	}

	static size_t get(void)
	{
		Type tmp;

		return getOfs(&tmp);
	}
};

template<typename Type> size_t storeable_object_ofs(const Type&)
{
	return storeable_object_ofs_t<Type> ::get();
}

// macro to implement storage mechanisms
#define IMPLEMENT_STORAGE							static struct storage_list_s storage_data[];						\
																														\
													virtual void push(StorageObject &rContainer) const override;		\
													virtual void pop(const StorageObject &rContainer) override;			\


#define BEGIN_STORAGE(Class)						struct storage_list_s Class::storage_data[] = 

//#define DECLARE_STORAGE(Class, Var)					{ typeid(Class).name(), #Var, typeid(Class::Var).name(), sizeof(Class::Var), (size_t)&((Class*)0)->Var, is_storeable_object(((Class*)0)->Var), storeable_object_ofs(((Class*)0)->Var) }

#define DECLARE_STORAGE_EX(Class, Var, Type)		{ typeid(Class).name(), #Var, Type, sizeof(Class::Var), (size_t)&((Class*)0)->Var, is_storeable_object(((Class*)0)->Var), storeable_object_ofs(((Class*)0)->Var) }
#define DECLARE_STORAGE(Class, Var)					DECLARE_STORAGE_EX(Class, Var, typeid(Class::Var).name())

#define END_STORAGE(Class, Parent)					;																																																			\
																																																																\
													void Class:: push(StorageObject &rContainer) const																																							\
													{																																																			\
														if(strcmp(#Class, #Parent))																																												\
															Parent :: push(rContainer);																																											\
																																																																\
														size_t nNumElements = sizeof(storage_data) / sizeof(struct storage_list_s);																																\
																																																																\
														generic_push(rContainer, this, storage_data, nNumElements);																																				\
													}																																																			\
																																																																\
													void Class:: pop(const StorageObject &rContainer)																																							\
													{																																																			\
														if(strcmp(#Class, #Parent))																																												\
															Parent :: pop(rContainer);																																											\
																																																																\
														size_t nNumElements = sizeof(storage_data) / sizeof(struct storage_list_s);																																\
																																																																\
														generic_pop(rContainer, this, storage_data, nNumElements);																																				\
													}