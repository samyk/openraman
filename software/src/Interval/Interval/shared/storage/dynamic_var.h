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
#include <functional>

#include "../utils/exception.h"
#include "../utils/safe.h"

#include "storeable_object.h"

template<typename Type> class dynamic_var : public IStoreableObject
{
public:
    using func_get = std::function<Type(void)>;
    using func_set = std::function<void(Type)>;

    // default ctor
    dynamic_var(void) {}

    // copy constructor
    dynamic_var(const dynamic_var<Type>& rVar)
    {
        this->m_get = rVar.m_get;
        this->m_set = rVar.m_set;
    }

    // set function constructor
    dynamic_var(func_get fget, func_set fset)
    {
        bind(fget, fset);
    }

    // bind functions
    void bind(func_get fget, func_set fset)
    {
        this->m_get = fget;
        this->m_set = fset;
    }

    // get content
    Type get(void) const
    {
        // NoFunctionException exception class
        class NoFunctionException : public IException
        {
        public:
            virtual std::string toString(void) const override
            {
                return "Cannot read from dynamic_var!";
            }
        };

        // check if function exist
        if (!this->m_get)
            throwException(NoFunctionException);

        // get content
        return this->m_get();
    }

    // cast operator
    operator Type(void) const
    {
        return get();
    }

    // set content
    void set(Type value) const
    {
        // NoFunctionException exception class
        class NoFunctionException : public IException
        {
        public:
            virtual std::string toString(void) const override
            {
                return "Cannot write to dynamic_var!";
            }
        };

        // check if function exist
        if (!this->m_set)
            throwException(NoFunctionException);

        // set content
        this->m_set(value);
    }

    // assignment operator
    void operator=(const Type& value)
    {
        set(value);
    }

    // delete object assignment operator due to ambiguous behaviour
    void operator=(const dynamic_var<Type>& rObject) = delete;

    // move operator
    void operator=(Type&& value)
    {
        set(value);
    }

    // delete object move operator due to ambiguous behaviour
    void operator=(dynamic_var<Type>&& rObject) = delete;

    // push object to container
    virtual void push(StorageObject& rContainer) const
    {
        // get content
        auto temp = get();

        // set typename
        rContainer.setTypeName(getClassName());

        // create subobject if Type is IStoreableObject
        if (is_storeable_object(temp))
        {
            // create sub container
            auto pSubContainer = rContainer.createSubObject("", "object");

            // push data to subcontainer
            if (pSubContainer != nullptr)
            {
                // get storeable object base
                IStoreableObject* pObject = (IStoreableObject*)(__ADDRESS((unsigned char*)&temp, storeable_object_ofs(temp)));

                if (pObject != nullptr)
                    pObject->push(*pSubContainer);
            }
        }
        // otherelse push var
        else
            rContainer.addVariable("", "data", typeid(temp).name(), sizeof(temp), &temp);
    }

    // pop object from container
    virtual void pop(const StorageObject& rContainer)
    {
        // check typename
        if (rContainer.getTypeName() != getClassName())
            throwException(WrongTypeException);

        // pop var
        Type temp;

        // read object if storeable object
        if (is_storeable_object(temp))
        {
            auto pSubContainer = rContainer.getSubObject("", "object");

            if (pSubContainer == nullptr)
                throwException(UnknownVarException);

            // get storeable object base
            IStoreableObject* pObject = (IStoreableObject*)(__ADDRESS((unsigned char*)&temp, storeable_object_ofs(temp)));

            if(pObject != nullptr)
                pObject->pop(*pSubContainer);
        }
        // otherelse pop var
        else
        {
            // throw exception if cannot read variable
            if (!rContainer.readVariable("", "data", typeid(temp).name(), sizeof(temp), &temp))
                throwException(UnknownVarException);
        }

        // set content
        set(temp);
    }

private:
    func_get m_get;
    func_set m_set;
};

#define BIND_DYNAMIC_VAR(class, var, getfunc, setfunc)     var.bind(std::bind(&class::getfunc, this), std::bind(&class::setfunc, this, std::placeholders::_1));