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
#include "depwalker.h"

BOOL CALLBACK EnumResourceCallback(HMODULE hModule, LPCSTR pszType, LPSTR pszName, LONG_PTR lParam)
{
	// convert lparam to list pointer
	string_list* pList = (string_list*)lParam;

	if (pList == nullptr)
		return FALSE;

	if (IS_INTRESOURCE(pszName))
	{
		// cast to int and add to list
		pList->emplace_back((int)pszName);
	}

	return TRUE;
}