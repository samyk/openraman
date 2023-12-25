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
#include "../utils/safe.h"

#include "storage_buffer.h"

#include <vector>
#include <string>
#include <list>
#include <memory>

class IStoreableObject;

struct data_var_s
{
	std::string owner;
	std::string name;
	std::string type;

	size_t nBytesSize;

	void *pData;
};

// StorageObject class
class StorageObject
{
public:
	StorageObject(void) {}

	StorageObject(const std::string& rOwnerName, const std::string& rVarName)
	{
		this->m_sOwnerName = rOwnerName;
		this->m_sVarName = rVarName;
	}

	StorageObject(const StorageObject&) = default;
	StorageObject(StorageObject&&) = default;
	~StorageObject(void) = default;

	// return handle to variables list for raw loaders
	const std::list<struct data_var_s>& getVars(void) const
	{
		return this->m_pVars;
	}

	// return handle to children list for raw loaders
	const std::list< std::shared_ptr<StorageObject> >& getChildren(void) const
	{
		return this->m_pChildren;
	}

	// get var name
	std::string getVarName(void) const
	{
		return this->m_sVarName;
	}

	// get owner name
	std::string getOwnerName(void) const
	{
		return this->m_sOwnerName;
	}

	// set type name
	void setTypeName(const std::string& rName)
	{
		this->m_sTypeName = rName;
	}

	// get type name
	std::string getTypeName(void) const
	{
		return this->m_sTypeName;
	}

	// create child
	std::shared_ptr<StorageObject> createSubObject(const std::string& rOwnerName, const std::string& rVarName)
	{
		auto pSubObject = std::make_shared<StorageObject>(rOwnerName, rVarName);

		if (pSubObject == nullptr)
			throw MemoryAllocationException(sizeof(StorageObject));

		this->m_pChildren.emplace_back(pSubObject);

		return pSubObject;
	}

	// add variable to list
	void addVariable(const std::string& rOwner, const std::string& rName, const std::string& rType, size_t nBytesSize, void *pData)
	{
		struct data_var_s s;

		s.owner = rOwner;
		s.name = rName;
		s.type = rType;

		s.nBytesSize = nBytesSize;

		s.pData = malloc(s.nBytesSize);

		if (s.pData == nullptr)
			throw MemoryAllocationException(s.nBytesSize);

		memcpy(s.pData, pData, s.nBytesSize);

		this->m_pVars.emplace_back(std::move(s));
	}

	// read variable from list
	bool readVariable(const std::string& rOwner, const std::string& rName, const std::string& rType, size_t nBytesSize, void *pData) const
	{
		// browse list
		for (auto& v : this->m_pVars)
		{
			// skip if different
			// HACK: had to disable type-safeness to make it work 32/64 bits platforms
			if (v.name != rName || v.owner != rOwner/* || v.type != rType*/)
				continue;

			// check specific types if size do not match
			if (nBytesSize != v.nBytesSize)
			{
				if (_strcmpi(rType.c_str(), "size_t") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((size_t*)pData) = __SIZE_T(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((size_t*)pData) = __SIZE_T(*(unsigned __int64*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "ptrdiff_t") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((ptrdiff_t*)pData) = __PTRDIFF_T(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((ptrdiff_t*)pData) = __PTRDIFF_T(*(unsigned __int64*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "wchar_t") == 0)
				{
					switch (v.nBytesSize)
					{
					case 2:
						*((wchar_t*)pData) = __WCHAR_T(*(unsigned __int16*)v.pData);
						return true;

					case 4:
						*((wchar_t*)pData) = __WCHAR_T(*(unsigned __int32*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "unsigned long") == 0 || _strcmpi(rType.c_str(), "unsigned long int") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((unsigned long*)pData) = __ULONG(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((unsigned long*)pData) = __ULONG(*(unsigned __int64*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "long") == 0 || _strcmpi(rType.c_str(), "signed long") == 0 || _strcmpi(rType.c_str(), "long int") == 0 || _strcmpi(rType.c_str(), "signed long int") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((long*)pData) = __LONG(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((long*)pData) = __LONG(*(unsigned __int64*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "unsigned int") == 0 || _strcmpi(rType.c_str(), "unsigned") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((unsigned int*)pData) = __UINT(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((unsigned int*)pData) = __UINT(*(unsigned __int64*)v.pData);
						return true;
					}
				}
				else if (_strcmpi(rType.c_str(), "int") == 0 || _strcmpi(rType.c_str(), "signed int") == 0 || _strcmpi(rType.c_str(), "signed") == 0)
				{
					switch (v.nBytesSize)
					{
					case 4:
						*((int*)pData) = __INT(*(unsigned __int32*)v.pData);
						return true;

					case 8:
						*((int*)pData) = __INT(*(unsigned __int64*)v.pData);
						return true;
					}
				}

				// throw exception for unhandled cases
				return false;
			}

			memcpy(pData, v.pData, nBytesSize);

			return true;
		}

		// return false if not found
		return false;
	}

	// get sub object from list
	std::shared_ptr<StorageObject> getSubObject(const std::string& rOwner, const std::string& rName) const
	{
		// browse list
		for (auto p : this->m_pChildren)
		{
			if (p == nullptr)
				continue;

			// skip if different
			if (p->m_sVarName != rName || p->m_sOwnerName != rOwner)
				continue;

			return p;
		}

		return nullptr;
	}

	// pack data into a buffer
	StorageBuffer pack(void)
	{
		// buffers
		StorageBuffer strings, vars, data, children, out;

		// declare header
		size_t nHeaderOfs = out.allocate<struct pack_header_s>();

		out.ptr<struct pack_header_s>(nHeaderOfs)->nNumVars = 0;
		out.ptr<struct pack_header_s>(nHeaderOfs)->nNumChildren = 0;

		out.ptr<struct pack_header_s>(nHeaderOfs)->nOwnerOfs = __UINT64(strings.append(this->m_sOwnerName, true));
		out.ptr<struct pack_header_s>(nHeaderOfs)->nNameOfs = __UINT64(strings.append(this->m_sVarName, true));
		out.ptr<struct pack_header_s>(nHeaderOfs)->nTypeNameOfs = __UINT64(strings.append(this->m_sTypeName, true));

		// browse all vars
		for (auto& v : this->m_pVars)
		{
			auto pCurrVar = vars.ptr<struct pack_var_s>(vars.allocate<struct pack_var_s>());

			pCurrVar->nOwnerOfs = __UINT64(strings.append(v.owner, true));
			pCurrVar->nNameOfs = __UINT64(strings.append(v.name, true));
			pCurrVar->nTypeOfs = __UINT64(strings.append(v.type, true));

			pCurrVar->nDataOfs = __UINT64(data.append((unsigned char*)v.pData, v.nBytesSize, false));
			pCurrVar->nBytesSize = __UINT64(v.nBytesSize);

			__INC(out.ptr<struct pack_header_s>(nHeaderOfs)->nNumVars, (unsigned __int64)1);
		}

		// browse all children
		for (auto p : this->m_pChildren)
		{
			if (p == nullptr)
				continue;

			StorageBuffer sub_buffer = p->pack();

			auto pCurrChild = children.ptr<struct pack_child_s>(children.allocate<struct pack_child_s>());

			pCurrChild->nOwnerOfs = __UINT64(strings.append(p->m_sOwnerName, true));
			pCurrChild->nNameOfs = __UINT64(strings.append(p->m_sVarName, true));

			pCurrChild->nDataOfs = __UINT64(data.append(sub_buffer, false));
			pCurrChild->nSize = __UINT64(sub_buffer.size());

			__INC(out.ptr<struct pack_header_s>(nHeaderOfs)->nNumChildren, (unsigned __int64)1);
		}

		// add temporary buffers to final buffer
		out.ptr<struct pack_header_s>(nHeaderOfs)->strings.nOffset = __UINT64(out.append(strings, false));
		out.ptr<struct pack_header_s>(nHeaderOfs)->strings.nSize = __UINT64(strings.size());

		out.ptr<struct pack_header_s>(nHeaderOfs)->vars.nOffset = __UINT64(out.append(vars, false));
		out.ptr<struct pack_header_s>(nHeaderOfs)->vars.nSize = __UINT64(vars.size());

		out.ptr<struct pack_header_s>(nHeaderOfs)->children.nOffset = __UINT64(out.append(children, false));
		out.ptr<struct pack_header_s>(nHeaderOfs)->children.nSize = __UINT64(children.size());

		out.ptr<struct pack_header_s>(nHeaderOfs)->data.nOffset = __UINT64(out.append(data, false));
		out.ptr<struct pack_header_s>(nHeaderOfs)->data.nSize = __UINT64(data.size());

		// return buffer
		return out;
	}

	// unpack data
	void unpack(const StorageBuffer &rBuffer)
	{
		// get header
		auto pHeader = rBuffer.ptr<struct pack_header_s>(0, sizeof(struct pack_header_s));

		// get strings buffer
		auto pStrings = rBuffer.ptr<char>(__SIZE_T(pHeader->strings.nOffset), __SIZE_T(pHeader->strings.nSize));

		// get vars buffer
		if (__SIZE_T(pHeader->vars.nSize) != __MULT(sizeof(struct pack_var_s), __SIZE_T(pHeader->nNumVars)))
			throw BufferReadException();

		auto pVars = rBuffer.ptr<struct pack_var_s>(__SIZE_T(pHeader->vars.nOffset), __SIZE_T(pHeader->vars.nSize));

		// get children
		if (__SIZE_T(pHeader->children.nSize) != __MULT(sizeof(struct pack_child_s), __SIZE_T(pHeader->nNumChildren)))
			throw BufferReadException();

		auto pChildren = rBuffer.ptr<struct pack_child_s>(__SIZE_T(pHeader->children.nOffset), __SIZE_T(pHeader->children.nSize));

		// get data buffer
		auto pData = rBuffer.ptr<unsigned char>(__SIZE_T(pHeader->data.nOffset), __SIZE_T(pHeader->data.nSize));

		// read data
		assert_string(pStrings, __SIZE_T(pHeader->nNameOfs), __SIZE_T(pHeader->strings.nSize));
		this->m_sVarName = __ADDRESS(pStrings, __SIZE_T(pHeader->nNameOfs));

		assert_string(pStrings, __SIZE_T(pHeader->nOwnerOfs), __SIZE_T(pHeader->strings.nSize));
		this->m_sOwnerName = __ADDRESS(pStrings, __SIZE_T(pHeader->nOwnerOfs));

		assert_string(pStrings, __SIZE_T(pHeader->nTypeNameOfs), __SIZE_T(pHeader->strings.nSize));
		this->m_sTypeName = __ADDRESS(pStrings, __SIZE_T(pHeader->nTypeNameOfs));

		for (size_t i = 0; i < __SIZE_T(pHeader->nNumVars); i++)
		{
			auto &curr = pVars[i];

			assert_string(pStrings, __SIZE_T(curr.nOwnerOfs), __SIZE_T(pHeader->strings.nSize));
			assert_string(pStrings, __SIZE_T(curr.nNameOfs), __SIZE_T(pHeader->strings.nSize));
			assert_string(pStrings, __SIZE_T(curr.nTypeOfs), __SIZE_T(pHeader->strings.nSize));

			addVariable(__ADDRESS(pStrings, __SIZE_T(curr.nOwnerOfs)), __ADDRESS(pStrings, __SIZE_T(curr.nNameOfs)), __ADDRESS(pStrings, __SIZE_T(curr.nTypeOfs)), __SIZE_T(curr.nBytesSize), (void*)__ADDRESS(pData, __SIZE_T(curr.nDataOfs)));
		}

		for (size_t i = 0; i < __SIZE_T(pHeader->nNumChildren); i++)
		{
			auto& curr = pChildren[i];

			assert_string(pStrings, __SIZE_T(curr.nOwnerOfs), __SIZE_T(pHeader->strings.nSize));
			assert_string(pStrings, __SIZE_T(curr.nNameOfs), __SIZE_T(pHeader->strings.nSize));

			auto pSubObject = createSubObject(__ADDRESS(pStrings, __SIZE_T(curr.nOwnerOfs)), __ADDRESS(pStrings, __SIZE_T(curr.nNameOfs)));

			if(__SIZE_T(__ADD(curr.nDataOfs, curr.nSize)) > __SIZE_T(pHeader->data.nSize))
				throw BufferReadException();

			StorageBuffer temp_buffer;

			temp_buffer.append(__ADDRESS(pData, __SIZE_T(curr.nDataOfs)), __SIZE_T(curr.nSize), false);

			pSubObject->unpack(temp_buffer);

			this->m_pChildren.emplace_back(pSubObject);
		}
	}

private:
	// BufferReadException exception class
	class BufferReadException : public IException
	{
	public:
		virtual std::string toString(void) const override
		{
			return "Erroneous buffer read exception!";
		}
	};

#pragma pack(push, 1)
	struct pack_buffer_s
	{
		unsigned __int64 nOffset;
		unsigned __int64 nSize;
	};

	struct pack_header_s
	{
		unsigned __int64 nTypeNameOfs;
		unsigned __int64 nOwnerOfs;
		unsigned __int64 nNameOfs;

		unsigned __int64 nNumVars;
		unsigned __int64 nNumChildren;

		struct pack_buffer_s strings;
		struct pack_buffer_s vars;
		struct pack_buffer_s children;
		struct pack_buffer_s data;
	};

	struct pack_var_s
	{
		unsigned __int64 nOwnerOfs;
		unsigned __int64 nNameOfs;
		unsigned __int64 nTypeOfs;

		unsigned __int64 nDataOfs;
		unsigned __int64 nBytesSize;

	};

	struct pack_child_s
	{
		unsigned __int64 nOwnerOfs;
		unsigned __int64 nNameOfs;

		unsigned __int64 nDataOfs;
		unsigned __int64 nSize;
	};
#pragma pack(pop)

	void assert_string(const char *pBase, size_t nOffset, size_t nMaxSize)
	{
		if (nOffset >= nMaxSize)
			throw BufferReadException();

		const char *p = __ADDRESS(pBase, nOffset);

		while (*(p++) != '\0')
			if (nOffset++ > nMaxSize)
				throw BufferReadException();
	}

	std::string m_sOwnerName;
	std::string m_sVarName;
	std::string m_sTypeName;

	std::list<struct data_var_s> m_pVars;
	std::list< std::shared_ptr<StorageObject> > m_pChildren;
};