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

#include "exception.h"

// NoSingletonException class
class NoSingletonException : public IException
{
public:
	NoSingletonException(const std::string& rType)
	{
		this->m_sType = rType;
	}

	virtual std::string toString(void) const override
	{
		return std::string("Cannot access singleton instance of type \"") + this->m_sType + std::string("\" !");
	}

private:
	std::string m_sType;
};

// Singleton class
template<class Type> class Singleton
{
public:
	static Type* pInstance;

	static auto getInstance(void)
	{
		if (pInstance == nullptr)
			pInstance = new Type();

		return pInstance;
	}

	// do not allow copy and move operators
	Singleton<Type>(const Singleton<Type>&) = delete;
	Singleton<Type>& operator=(const Singleton<Type>&) = delete;
	Singleton<Type>& operator=(const Singleton<Type>&&) = delete;

protected:
	// do not allow other creation than via getInstance
	Singleton<Type>(void) {}

private:
	// only main class can initialize
	void* operator new(size_t nSize)
	{
		return ::operator new(nSize);
	}
};

// automatically create and destroy singleton instance using RIAA idiom
template<class Type> class AutoSingleton
{
public:
	// create instance on initialization
	AutoSingleton(void)
	{
		// create instance
		Type::getInstance();
	}

	// destruct pointer on dtor
	~AutoSingleton(void)
	{
		delete Type::pInstance;
		Type::pInstance = nullptr;
	}
};

// return instance or throw error
template<typename Type> decltype(auto) getInstance(void)
{
	auto pInstance = Type::getInstance();

	if (pInstance == nullptr)
		throwException(NoSingletonException, typeid(Type).name());

	return pInstance;
}

// initialize singleton automatically on program startup
#define INITIALIZE_SINGLETON(Type)	Type *Singleton<Type> :: pInstance = nullptr;									\
																													\
									AutoSingleton<Type> m_cAutoSingleton##Type;										\