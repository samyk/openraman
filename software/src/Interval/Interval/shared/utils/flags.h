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

#include <map>
#include <functional>

// flag implementation class
class FlagImpl
{
public:
    using solver_t = std::function<bool(void)>;

    // insert dynamic flag
    void setFlag(int iFlag, solver_t callback)
    {
        // check if flag already exists
        auto it = this->m_flags.find(iFlag);

        // change flag if existing
        if (it != this->m_flags.end())
        {
            it->second = callback;

            return;
        }

        // otherelse insert
        this->m_flags.emplace(std::make_pair(iFlag, callback));
    }

    // insert static flag
    void setFlag(int iFlag, bool bStatus)
    {
        // create dummy function and insert it
        setFlag(iFlag, [=](void)
            {
                return bStatus;
            });
    }

    // check if flag exists
    bool hasFlag(int iFlag)
    {
        return this->m_flags.find(iFlag) != this->m_flags.end();
    }

    // test flag
    bool testFlag(int iFlag)
    {
        // retrieve flag
        auto it = this->m_flags.find(iFlag);

        // always return false if flag does not exists
        if (it == this->m_flags.end())
            return false;

        // return false if function is invalid
        if (!it->second)
            return false;

        // otherelse return flag status
        return it->second();
    }

private:
    std::map<int, solver_t> m_flags;
};