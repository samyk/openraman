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

#include <functional>

// template class for any property that calls a callback function on change
template<typename Type> class guiProperty
{
public:
	guiProperty(void)
	{

	}

	guiProperty(const guiProperty<Type>& rProperty)
	{
		this->m_callback = rProperty.m_callback();

		this->operator=(rProperty);
	}

	guiProperty(guiProperty<Type>&& rProperty)
	{
		this->m_callback = rProperty.m_callback();
		rProperty.m_callback._Reset();

		this->operator=(std::move(rProperty));
	}

	// bind callback function to property
	void bind(std::function<void(void)> callback)
	{
		this->m_callback = callback;
	}

	// assignment operator
	const guiProperty<Type>& operator=(const guiProperty<Type>& rProperty)
	{
		set(rProperty.get());

		return *this;
	}

	// move operator
	const guiProperty<Type>& operator=(guiProperty<Type>&& rProperty) noexcept
	{
		set(rProperty.get());

		return *this;
	}

	// set operator
	const guiProperty<Type>& operator=(Type data)
	{
		set(data);

		return *this;
	}

	// get operator
	operator Type(void)
	{
		return get();
	}

	// set function
	void set(Type data)
	{
		// copy data
		this->m_data = data;
		
		// call callback function
		if(this->m_callback)
			this->m_callback();
	}

	// get data
	const Type& get(void) const
	{
		return this->m_data;
	}

private:
	Type m_data;

	std::function<void(void)> m_callback;
};

// macro to bind to a class 'update' member function
#define WATCH(class,var)		var.bind(std::bind(&class::update, this))