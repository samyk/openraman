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

#include <Windows.h>
#include <commctrl.h>
#include <Uxtheme.h>
#include <strsafe.h>

#include <list>
#include <memory>
#include <string>

#include "shared/utils/exception.h"
#include "shared/utils/notify.h"
#include "shared/utils/evemon.h"
#include "shared/gui/icons.h"

#include "winmain.h"
#include "exception.h"

void redraw(HWND hWnd, UINT uiComponent = 0);

 // main application class
class EventMonitorApp : public NotifyImpl
{
public:

	// event types
	enum
	{
		EVENT_CLOSE,
		EVENT_REBUILD,
		EVENT_REDRAW,
		EVENT_SAVE,
		EVENT_OPEN,
        EVENT_CLEAR,
        EVENT_MESSAGE,
        EVENT_WARNING,
        EVENT_ERROR,
        EVENT_CRITICAL,
        EVENT_EXCEPTION,
		EVENT_ENABLE_ALL,
		EVENT_DISABLE_ALL,
	};

	// constructor
	EventMonitorApp(HINSTANCE hInstance)
	{
		_debug("initializing EventMonitorApp");

		// copy instance
		this->m_hInstance = hInstance;

		// initialize all fields to null
		this->m_hWnd = NULL;
		this->m_hImageList = NULL;

		// all components enabled by default
		this->m_bEnable = true;

		// register events
		listen(EVENT_CLOSE, SELF(EventMonitorApp::onClose));
		listen(EVENT_REBUILD, SELF(EventMonitorApp::onRebuild));
		listen(EVENT_REDRAW, SELF(EventMonitorApp::onRedraw));
		listen(EVENT_SAVE, SELF(EventMonitorApp::onSave));
		listen(EVENT_OPEN, SELF(EventMonitorApp::onOpen));
		listen(EVENT_CLEAR, SELF(EventMonitorApp::onClear));
		listen(EVENT_MESSAGE, SELF(EventMonitorApp::onMessage));
		listen(EVENT_WARNING, SELF(EventMonitorApp::onWarning));
		listen(EVENT_ERROR, SELF(EventMonitorApp::onError));
		listen(EVENT_CRITICAL, SELF(EventMonitorApp::onCritical));
		listen(EVENT_EXCEPTION, SELF(EventMonitorApp::onException));
		listen(EVENT_ENABLE_ALL, SELF(EventMonitorApp::onEnableAll));
		listen(EVENT_DISABLE_ALL, SELF(EventMonitorApp::onDisableAll));

		// create window
		_debug("creating window");

		this->m_hWnd = CreateWindow(TEXT(EVEMON_CLASSNAME), TEXT(APP_NAME), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, SCREENWIDTH, SCREENHEIGHT, NULL, NULL, hInstance, NULL);

		if (this->m_hWnd == NULL)
			throwException(InitException);

		// center window
		SetWindowPos(this->m_hWnd, HWND_TOP, GetSystemMetrics(SM_CXSCREEN) / 2 - SCREENWIDTH / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - SCREENHEIGHT / 2, 0, 0, SWP_NOSIZE);

		// add items
		struct item_s
		{
			UINT id;
			const char* pszLabel;
			int spacing;
		};

		RECT client_rect;

		GetClientRect(this->m_hWnd, &client_rect);

		const int iMenuHeight = 32;

		const int iBorderLeft = 10;
		const int iBorderTop = 10;
		const int iBorderRight = 10;
		const int iBorderBottom = 10;

		const int iButtonWidth = 48;
		const int iButtonSpacing = 20;
		const int iButtonHeight = 24;		
		const int iCheckboxHeight = 24;
		const int iCheckboxWidth = 48;
		const int iLetterSpacing = 5;
		const int iCheckboxSpacing = 20;

		struct item_s checkboxes[] = {
			{IDC_MESSAGE, "messages"},
			{IDC_WARNING, "warnings"},
			{IDC_ERROR, "errors"},
			{IDC_CRITICAL, "critical errors"},
			{IDC_EXCEPTION, "exceptions"},
		};

		int x = iBorderLeft;
		int y = iBorderTop + (iMenuHeight - iCheckboxHeight) / 2;

		for (size_t i = 0; i < (sizeof(checkboxes) / sizeof(struct item_s)); i++)
		{
			int iWidth = iCheckboxWidth + iLetterSpacing * (int)strlen(checkboxes[i].pszLabel);

			HWND hControl = CreateWindowExA(0, "BUTTON", checkboxes[i].pszLabel, BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE, x, y, iWidth, iCheckboxHeight, this->m_hWnd, (HMENU)checkboxes[i].id, this->m_hInstance, NULL);

			ShowWindow(hControl, TRUE);

			x += iWidth + iCheckboxSpacing;
		}

		struct item_s buttons[] = {
			{IDC_SAVE, "Save"},
			{IDC_OPEN, "Open"},
			{IDC_CLEAR, "Clear"},
		};

		x = client_rect.right - iBorderRight;
		y = iBorderTop + (iMenuHeight - iButtonHeight) / 2;

		for (size_t i = 0; i < (sizeof(buttons) / sizeof(struct item_s)); i++)
		{
			int iWidth = iButtonWidth + iLetterSpacing * (int)strlen(buttons[i].pszLabel);

			HWND hControl = CreateWindowExA(0, "BUTTON", buttons[i].pszLabel, WS_CHILD | WS_VISIBLE | BS_FLAT | BS_CENTER | BS_DEFPUSHBUTTON | BS_VCENTER | BS_TEXT, x - iWidth, y, iWidth, iButtonHeight, this->m_hWnd, (HMENU)buttons[i].id, this->m_hInstance, NULL);

			ShowWindow(hControl, TRUE);

			x -= iWidth + iButtonSpacing;
		}

		// create list view control
		struct column_s
		{
			const char* text;
			int fraction;
			int align;
		};

		struct column_s columns[] = {
			{ "type", 10, LVCFMT_LEFT },
			{ "time", 8, LVCFMT_CENTER },
			{ "process", 11, LVCFMT_LEFT },
			{ "file", 11, LVCFMT_LEFT },
			{ "message", 60, LVCFMT_LEFT },
		};

		int iListWidth = client_rect.right - iBorderRight - iBorderLeft;

		HWND hControl = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "", WS_CHILD | WS_VISIBLE | LVS_REPORT, iBorderLeft, iBorderTop + iMenuHeight, iListWidth, client_rect.bottom - iBorderTop - iBorderBottom - iMenuHeight, this->m_hWnd, (HMENU)IDC_LIST, this->m_hInstance, NULL);

		LVCOLUMNA lvc;

		iListWidth -= 20;

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		for (size_t i = 0; i < (sizeof(columns) / sizeof(struct column_s)); i++)
		{
			lvc.iSubItem = (int)i;
			lvc.pszText = (LPSTR)columns[i].text;
			lvc.cx = (iListWidth * columns[i].fraction) / 100;
			lvc.fmt = columns[i].align;

			SendMessageA(hControl, LVM_INSERTCOLUMNA, (WPARAM)i, (LPARAM)&lvc);
		}

		ImportedIconSet icons;

		try
		{
			icons.loadFromResource(IDR_RCDATA1);
		}
		catch (...) {}

		this->m_hImageList = ImageList_Create((int)icons.getIconSize(), (int)icons.getIconSize(), ILC_COLOR32, 6, 0);

		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		ImageList_AddIcon(this->m_hImageList, LoadIcon(this->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)));

		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 0, icons.getIcon("message")));
		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 1, icons.getIcon("warning")));
		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 2, icons.getIcon("error")));
		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 3, icons.getIcon("critical")));
		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 4, icons.getIcon("exception")));
		NOTHROW(ImageList_ReplaceIcon(this->m_hImageList, 5, icons.getIcon("unknown")));

		ListView_SetImageList(hControl, this->m_hImageList, LVSIL_NORMAL);
		ListView_SetImageList(hControl, this->m_hImageList, LVSIL_SMALL);
		ListView_SetImageList(hControl, this->m_hImageList, LVSIL_STATE);

		ShowWindow(hControl, TRUE);

		// default state
		enableMessage(false);
		enableWarning(true);
		enableError(true);
		enableCritical(true);
		enableException(true);

		// update window and display
		UpdateWindow(this->m_hWnd);
		ShowWindow(this->m_hWnd, TRUE);
	}

	// destructor
	~EventMonitorApp(void)
	{
		_debug("clearing EventMonitorApp");

		// clear image list
		if (this->m_hImageList != NULL)
			ImageList_Destroy(this->m_hImageList);

		this->m_hImageList = NULL;

		// post quit message
		PostQuitMessage(0);

		this->m_hWnd = NULL;
	}

	// initialize window by issueing update operation
	void init(void)
	{
		_debug("finishing initialization");

		// update window
		notify(EVENT_REDRAW);
	}

	// update enable state of all components
	void updateEnable(void)
	{
		bool bEnable = this->m_bEnable;

		_enableMessage(bEnable);
		_enableWarning(bEnable);
		_enableError(bEnable);
		_enableCritical(bEnable);
		_enableException(bEnable);
		_enableClear(bEnable);
		_enableOpen(bEnable);
		_enableSave(bEnable);
	}

	// return window handle
	HWND getWindowHandle(void) const
	{
		return this->m_hWnd;
	}

	// check enabled state
	void enableMessage(bool bEnable)
	{
		CheckDlgButton(this->m_hWnd, IDC_MESSAGE, bEnable ? TRUE : FALSE);
		
		notify(EVENT_MESSAGE);
	}

	bool isMessageEnabled(void) const
	{
		return IsDlgButtonChecked(this->m_hWnd, IDC_MESSAGE) == TRUE;
	}

	void enableWarning(bool bEnable)
	{
		CheckDlgButton(this->m_hWnd, IDC_WARNING, bEnable ? TRUE : FALSE);

		notify(EVENT_WARNING);
	}

	bool isWarningEnabled(void) const
	{
		return IsDlgButtonChecked(this->m_hWnd, IDC_WARNING) == TRUE;
	}

	void enableError(bool bEnable)
	{
		CheckDlgButton(this->m_hWnd, IDC_ERROR, bEnable ? TRUE : FALSE);

		notify(EVENT_ERROR);
	}

	bool isErrorEnabled(void) const
	{
		return IsDlgButtonChecked(this->m_hWnd, IDC_ERROR) == TRUE;
	}

	void enableCritical(bool bEnable)
	{
		CheckDlgButton(this->m_hWnd, IDC_CRITICAL, bEnable ? TRUE : FALSE);

		notify(EVENT_CRITICAL);
	}

	bool isCriticalEnabled(void) const
	{
		return IsDlgButtonChecked(this->m_hWnd, IDC_CRITICAL) == TRUE;
	}

	void enableException(bool bEnable)
	{
		CheckDlgButton(this->m_hWnd, IDC_EXCEPTION, bEnable ? TRUE : FALSE);

		notify(EVENT_EXCEPTION);
	}

	bool isExceptionEnabled(void) const
	{
		return IsDlgButtonChecked(this->m_hWnd, IDC_EXCEPTION) == TRUE;
	}

	// event handling function
	bool windowProc(UINT uiMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			return true;

		case WM_DESTROY:
			PostQuitMessage(0);
			return true;

		case WM_COPYDATA:
			return processCopyData((HWND)wParam, (COPYDATASTRUCT*)lParam);

		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code)
			{
			case LVN_GETDISPINFOA:
				return setItemData((NMLVDISPINFOA*)lParam);
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_MESSAGE:
				notify(EVENT_MESSAGE);
				return true;

			case IDC_WARNING:
				notify(EVENT_WARNING);
				return true;

			case IDC_ERROR:
				notify(EVENT_ERROR);
				return true;

			case IDC_CRITICAL:
				notify(EVENT_CRITICAL);
				return true;

			case IDC_EXCEPTION:
				notify(EVENT_EXCEPTION);
				return true;

			case IDC_CLEAR:
				notify(EVENT_CLEAR);
				return true;

			case IDC_OPEN:
				notify(EVENT_OPEN);
				return true;

			case IDC_SAVE:
				notify(EVENT_SAVE);
				return true;

			default:
				break;
			}
			break;
		}

		return false;
	}

	void loadFile(const std::string& rFilename)
	{
		// clear
		notify(EVENT_CLEAR);

		// cleanup filename
		char szFilename[MAX_PATH];
		strcpy_s(szFilename, rFilename.c_str());
		char* pszFilename = &szFilename[0];

		while (*pszFilename == '\"')
			pszFilename++;

		while (strlen(pszFilename) > 0 && pszFilename[strlen(pszFilename) - 1] == '\"')
			pszFilename[strlen(pszFilename) - 1] = '\0';

		// create data builder object
		try
		{
			_debug("loading file %s", pszFilename);

			// .err case
			if (strEndsWith(pszFilename, ".err"))
			{
				// create storage container from file
				StorageContainer container;
				container.unpack(loadBufferFromFile(pszFilename));

				// list all objects
				pop(container);
			}
			// throw exception for unknown extensions
			else
				throwException(UnknownFileTypeException, pszFilename);
		}
		catch (IException& rException)
		{
			_error("%s", rException.toString().c_str());

			MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			_error("Unknown error!");

			MessageBox(this->m_hWnd, TEXT("Unknown error!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}

		// update plot data
		notify(EVENT_REDRAW);

		// update components
		updateEnable();
	}

private:

	// process copy data message
	bool processCopyData(HWND hWnd, COPYDATASTRUCT* pData)
	{
		_debug("receiving COPYDATASTRUCT data");

		// skip if null
		if (pData == nullptr)
			return false;

		// dispatch messages types
		switch (pData->dwData)
		{
		case EventData::VERSION:

			// unpack data
			StorageContainer container;

			container.unpack(StorageBuffer((unsigned char*)pData->lpData, pData->cbData));

			// add all events to list
			pop(container);

			// notify update
			// notify(EVENT_REDRAW);

			return true;
		}

		// message was not processed
		return false;
	}

	// restore data from a container
	void pop(StorageContainer& rContainer)
	{
		for (auto& v : rContainer)
		{
			try
			{
				// pop event
				EventData evdata;
				evdata.pop(v);

				// copy to list
				this->m_list.push_back(evdata);

				// add item to list
				if (filter(evdata.type))
					addItem(this->m_list.back());
			}
			catch (...) {}
		}
	}

	// save data to file
	void saveToFile(const std::string& rFile)
	{
		_debug("saving file");

		// write all data to file
		try
		{
			// .csv case
			if (strEndsWith(rFile.c_str(), ".csv"))
			{
				FILE* pFile = nullptr;

				// write file stream
				fopen_s(&pFile, rFile.c_str(), "wb+");

				if (pFile == nullptr)
				{
					_error("Unable to write file!");

					// do not show dialog box in silent mode
					MessageBox(this->m_hWnd, TEXT("Unable to write file!"), TEXT("error"), MB_ICONHAND | MB_OK);

					return;
				}

				// write all events
				for (auto& v : this->m_list)
				{
					if (!filter(v.type))
						continue;

					time_t ltime = (time_t)v.ltime;
					struct tm t;

					localtime_s(&t, &ltime);

					auto type = EventMonitor::event2string(v.type);

					fprintf(pFile, "\"%s\",\"%.02d:%.02d:%.02d\",\"%s\",\"%s:%d\",\"%s\"\r\n", type.c_str(), t.tm_hour, t.tm_min, t.tm_sec, v.procname.c_str(), v.filename.c_str(), v.line, v.message.c_str());
				}

				// close stream
				fclose(pFile);
			}
			// .bin case
			else if (strEndsWith(rFile.c_str(), ".err"))
			{
				StorageContainer container;

				for (auto& v : this->m_list)
				{
					if (!filter(v.type))
						continue;

					StorageObject buffer;
					v.push(buffer);

					container.emplace_back(std::move(buffer));
				}

				container.saveToFile(rFile.c_str(), 0);
			}
			else
				throwException(WrongFileTypeException, rFile);
		}
		catch (IException& rException)
		{
			_error("%s", rException.toString().c_str());

			// do not show dialog box in silent mode
			MessageBoxA(this->m_hWnd, rException.toString().c_str(), "error", MB_ICONHAND | MB_OK);
		}
		catch (...)
		{
			_error("Error when saving results!");

			// do not show dialog box in silent mode
			MessageBox(this->m_hWnd, TEXT("Error when saving results!"), TEXT("error"), MB_ICONHAND | MB_OK);
		}
	}

	// enable message checkbox
	void _enableMessage(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_MESSAGE), bEnable ? TRUE : FALSE);
	}

	// enable warning checkbox
	void _enableWarning(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_WARNING), bEnable ? TRUE : FALSE);
	}

	// enable error checkbox
	void _enableError(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_ERROR), bEnable ? TRUE : FALSE);
	}

	// enable critical checkbox
	void _enableCritical(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_CRITICAL), bEnable ? TRUE : FALSE);
	}

	// enable exception checkbox
	void _enableException(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_EXCEPTION), bEnable ? TRUE : FALSE);
	}

	// enable clear button
	void _enableClear(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_CLEAR), bEnable ? TRUE : FALSE);
	}

	// enable open button
	void _enableOpen(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_OPEN), bEnable ? TRUE : FALSE);
	}

	// enable save button
	void _enableSave(bool bEnable)
	{
		EnableWindow(GetDlgItem(this->m_hWnd, IDC_SAVE), bEnable ? TRUE : FALSE);
	}

	// on close action
	void onClose(void)
	{
		_debug("closing EventMonitorApp");

		// destroy
		DestroyWindow(this->m_hWnd);
	}

	// rebuild action
	void onRebuild(void)
	{
		// delete all elements
		HWND hControl = GetDlgItem(this->m_hWnd, IDC_LIST);

		ListView_DeleteAllItems(hControl);

		// add all items
		for (auto& v : this->m_list)
			if (filter(v.type))
				addItem(v);

		// perform redraw
		notify(EVENT_REDRAW);
	}

	// redraw action
	void onRedraw(void)
	{
		redraw(this->m_hWnd);
	}

	// message action
	void onMessage(void)
	{
		notify(EVENT_REBUILD);
	}

	// warning action
	void onWarning(void)
	{
		notify(EVENT_REBUILD);
	}

	// error action
	void onError(void)
	{
		notify(EVENT_REBUILD);
	}

	// critical action
	void onCritical(void)
	{
		notify(EVENT_REBUILD);
	}

	// exception action
	void onException(void)
	{
		notify(EVENT_REBUILD);
	}

	// clear action
	void onClear(void)
	{
		HWND hControl = GetDlgItem(this->m_hWnd, IDC_LIST);

		ListView_DeleteAllItems(hControl);

		this->m_list.clear();

		notify(EVENT_REBUILD);
	}

	// save action
	void onSave(void)
	{
		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[256] = "Binary data (.err)\0*.err\0Comma Separated (.csv)\0*.csv\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if (!GetSaveFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		// add extension if not set
		switch (sFile.nFilterIndex)
		{
		case 1:
			if (!strEndsWith(szFilename, ".err"))
				strcat_s(szFilename, ".err");
			break;

		case 2:
			if (!strEndsWith(szFilename, ".csv"))
				strcat_s(szFilename, ".csv");
			break;
		}

		// save file
		saveToFile(szFilename);
	}

	// open action
	void onOpen(void)
	{
		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[256] = "Binary data (.err)\0*.err\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = this->m_hWnd;
		sFile.hInstance = this->m_hInstance;
		sFile.lpstrFilter = szFilter;
		sFile.lpstrFile = szFilename;
		sFile.nMaxFile = sizeof(szFilename);
		sFile.nMaxFileTitle = sizeof(szTitle);
		sFile.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_EXPLORER;

		if (!GetOpenFileNameA(&sFile))
			return;

		if (*szFilename == '\0')
			return;

		loadFile(szFilename);
	}

	// enable all components
	void onEnableAll(void)
	{
		this->m_bEnable = true;

		updateEnable();
	}

	// disable all components
	void onDisableAll(void)
	{
		this->m_bEnable = false;

		updateEnable();
	}

	// called after each notification
	virtual void onNotify(int iEvent) override
	{
		// update enable state of components
		updateEnable();
	}

	bool setItemData(NMLVDISPINFOA *pDispInfo)
	{
		if (pDispInfo == nullptr)
			return false;

		EventData* pEvent = (EventData*)pDispInfo->item.lParam;

		if (pEvent == nullptr)
			return false;

		std::string temp;

		time_t ltime = (time_t)pEvent->ltime;
		struct tm t;

		localtime_s(&t, &ltime);

		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			temp = EventMonitor::event2string(pEvent->type);

			StringCchCopyA(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, temp.c_str());

			return true;

		case 1:
			temp = format("%.02d:%.02d:%.02d", t.tm_hour, t.tm_min, t.tm_sec);

			StringCchCopyA(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, temp.c_str());

			return true;

		case 2:
			pDispInfo->item.pszText = (LPSTR)pEvent->procname.c_str();

			return true;

		case 3:
			temp = pEvent->filename + format(":%d", pEvent->line);

			StringCchCopyA(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, temp.c_str());

			return true;

		case 4:
			pDispInfo->item.pszText = (LPSTR)pEvent->message.c_str();
			return true;

		default:
			return false;
		}
	}

	// add item to list
	void addItem(EventData &rEvent)
	{
		HWND hControl = GetDlgItem(this->m_hWnd, IDC_LIST);

		int iCurrentID = ListView_GetItemCount(hControl);

		// initialize default values for item
		LVITEMA item;

		item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM | LVIF_IMAGE;
		item.pszText = LPSTR_TEXTCALLBACKA;
		item.stateMask = (UINT)-1;
		item.iItem = iCurrentID;
		item.state = 0;
		item.iSubItem = 0;
		item.lParam = (LPARAM)&rEvent;

		switch (rEvent.type)
		{
		case EventType::DBG_MESSAGE:
			item.iImage = 0;
			break;

		case EventType::DBG_WARNING:
			item.iImage = 1;
			break;

		case EventType::DBG_ERROR:
			item.iImage = 2;
			break;

		case EventType::DBG_CRITICAL:
			item.iImage = 3;
			break;

		case EventType::DBG_EXCEPTION:
			item.iImage = 4;
			break;

		default:
			item.iImage = 5;
			break;
		}
		
		SendMessageA(hControl, LVM_INSERTITEMA, (WPARAM)0, (LPARAM)&item);

		// scroll to item
		ListView_EnsureVisible(hControl, iCurrentID, FALSE);
	}

	// return true if should process the message
	bool filter(EventType eType)
	{
		switch (eType)
		{
		case EventType::DBG_MESSAGE:
			return isMessageEnabled();

		case EventType::DBG_WARNING:
			return isWarningEnabled();

		case EventType::DBG_ERROR:
			return isErrorEnabled();

		case EventType::DBG_CRITICAL:
			return isCriticalEnabled();

		case EventType::DBG_EXCEPTION:
			return isExceptionEnabled();

		default:
			return false;
		}
	}

	bool m_bEnable;

	std::list<EventData> m_list;

	HINSTANCE m_hInstance;
	HWND m_hWnd;

	HIMAGELIST m_hImageList;
};