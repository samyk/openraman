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
#include <string>
#include <map>
#include <functional>

#include <Windows.h>

#include "../utils/singleton.h"
#include "../utils/notify.h"

// NoWindowException class
class NoWindowException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Invalid window handle!";
	}
};

// InvalidDialogException class
class InvalidDialogException : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Invalid dialog!";
	}
};

// generic dialog procedure to dispatch to dialog manager
INT_PTR CALLBACK genericDialogProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);

// interface for dialogs class
class IDialog : public NotifyImpl
{
public:
	// remove default constructor, copy constructor and move constructor
	IDialog(void) = delete;
	IDialog(const IDialog&) = delete;
	IDialog(IDialog&&) = delete;

	// create dialog in constructor
	IDialog(HWND hParentWnd, HINSTANCE hInstance, UINT uiDialogID)
	{
		this->m_hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(uiDialogID), hParentWnd, &genericDialogProc);
	}

	// destroy window in destructor
	virtual ~IDialog(void)
	{
		try
		{
			destroy();
		}
		catch (...) {}
	}

	// destroy dialog
	void destroy(void)
	{
		if (this->m_hWnd != NULL)
			DestroyWindow(this->m_hWnd);

		this->m_hWnd = NULL;
	}

	// show dialog
	virtual void show(bool bShow)
	{
		// throw exception if no window, should never happen
		if (this->m_hWnd == NULL)
			throw NoWindowException();

		// show or hide dialog
		ShowWindow(this->m_hWnd, bShow ? TRUE : FALSE);
	}

	// return true if dialog is visible
	virtual bool isVisible(void) const
	{
		// throw exception if no window, should never happen
		if (this->m_hWnd == NULL)
			throw NoWindowException();

		// return true if dialog is visible
		return IsWindowVisible(this->m_hWnd) == TRUE;
	}

	// send close message to dialog
	virtual void close(void)
	{
		// throw exception if no window, should never happen
		if (this->m_hWnd == NULL)
			throw NoWindowException();

		// send close message to dialog
		SendMessage(this->m_hWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
	}

	// initialization function
	virtual void init(void) = 0;

	// called on specific events
	virtual void onDestroy(void) {}

	// return handle to the window
	HWND getWindowHandle(void) const
	{
		return this->m_hWnd;
	}

	// return handle to a window item
	HWND getItemHandle(int iItemID) const
	{
		if (this->m_hWnd == NULL)
			throw NoWindowException();

		return GetDlgItem(this->m_hWnd, iItemID);
	}

	// dialog MUST have a procedure to handle messages
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) = 0;

private:

	// handle to dialog window
	HWND m_hWnd;
};

// UnknownDialogException class
class UnknownDialogException : public IException
{
public:
	UnknownDialogException(HWND hWnd)
	{
		this->m_hWnd = hWnd;
	}

	virtual std::string toString(void) const override
	{
		return "Unknown dialog!";
	}

	HWND m_hWnd;
};

// DialogsManager class is a sinleton
class DialogsManager : public Singleton<DialogsManager>
{
	friend class Singleton<DialogsManager>;

public:

	// register a new dialog
	void registerDialog(HWND hWnd, std::shared_ptr<IDialog> pDialog)
	{
		// check if handle already exists
		auto it = this->m_catalog.find(hWnd);

		// replace if true
		if (it != this->m_catalog.end())
			it->second = pDialog;

		// insert otherwise
		else
			this->m_catalog.emplace(std::make_pair(hWnd, pDialog));
	}

	// dispatch dialog procedure to registered dialogs
	INT_PTR dialogProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
	{
		// find dialog object from handle
		auto it = this->m_catalog.find(hWnd);

		// throw exception if dialog is not registered
		if (it == this->m_catalog.end())
			throw UnknownDialogException(hWnd);

		// return FALSE (default procedure) is dialog was destroyed
		if (it->second == nullptr)
			return FALSE;

		// apply dialog procedure
		auto ret = it->second->dialogProc(uiMessage, wParam, lParam);

		// remove dialog if destroyed
		if (uiMessage == WM_DESTROY)
			it->second = nullptr;

		// return result
		return ret;
	}

private:

	// list of dialog and handles
	std::map<HWND, std::shared_ptr<IDialog>> m_catalog;
};

// use this function to create a dialog
template<class Type> std::shared_ptr<Type> createDialog(HWND hParentWnd, HINSTANCE hInstance)
{
	// create a std::shared_ptr object
	auto pDialog = std::make_shared<Type>(hParentWnd, hInstance, Type::RESOURCE_ID);

	// throw error in case of issues
	if (pDialog == nullptr)
		throw InvalidDialogException();

	// register dialog to dialog manager
	getInstance<DialogsManager>()->registerDialog(pDialog->getWindowHandle(), pDialog);

	// return pointer
	return pDialog;
}