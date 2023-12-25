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
#include <CommCtrl.h>

#include "shared/utils/exception.h"
#include "shared/gui/dialogs.h"
#include "shared/gui/icons.h"

#include "listview.h"
#include "depwalker.h"
#include "resource.h"

void redraw(HWND hWnd, UINT uiComponent = 0);

// UnsupportedIconSize exception class
class UnsupportedIconSize : public IException
{
public:
	virtual std::string toString(void) const override
	{
		return "Icon size is not supported!";
	}
};

// icons editor dialog class
class wndIconsEditorDialog : public IDialog
{
public:

	// events
	enum
	{
		EVENT_INIT_DONE,
		EVENT_CLOSE,
		EVENT_DESTROY,
		EVENT_UPDATE,
		EVENT_NEW,
		EVENT_OPEN,
		EVENT_SAVE,
		EVENT_SELECTION,
		EVENT_ADD,
		EVENT_REMOVE,
		EVENT_MNEMONIC,
		EVENT_FILE,
		EVENT_SIZE,
		EVENT_LIST,
		EVENT_ENABLEALL,
		EVENT_DISABLEALL,
	};

	// using IDialog constructors
	using IDialog::IDialog;

	// resource ID for dialog
	static const UINT RESOURCE_ID = IDD_DIALOG;

	// set text on initialization
	virtual void init(void) override
	{
		// current icon is null
		this->m_pCurrIcon = nullptr;

		// all components enabled by default
		this->m_bEnable = true;

		// listen to event
		listen(EVENT_CLOSE, SELF(wndIconsEditorDialog::onClose));
		listen(EVENT_DESTROY, SELF(wndIconsEditorDialog::onDestroy));
		listen(EVENT_UPDATE, SELF(wndIconsEditorDialog::onUpdate));
		listen(EVENT_NEW, SELF(wndIconsEditorDialog::onNew));
		listen(EVENT_OPEN, SELF(wndIconsEditorDialog::onOpen));
		listen(EVENT_SAVE, SELF(wndIconsEditorDialog::onSave));
		listen(EVENT_SELECTION, SELF(wndIconsEditorDialog::onSelection));
		listen(EVENT_ADD, SELF(wndIconsEditorDialog::onAdd));
		listen(EVENT_REMOVE, SELF(wndIconsEditorDialog::onRemove));
		listen(EVENT_MNEMONIC, SELF(wndIconsEditorDialog::onMnemonic));
		listen(EVENT_FILE, SELF(wndIconsEditorDialog::onFile));
		listen(EVENT_SIZE, SELF(wndIconsEditorDialog::onSize));
		listen(EVENT_LIST, SELF(wndIconsEditorDialog::onList));
		listen(EVENT_ENABLEALL, SELF(wndIconsEditorDialog::onEnableAll));
		listen(EVENT_DISABLEALL, SELF(wndIconsEditorDialog::onDisableAll));

		// initialize components
		this->m_selection = ListViewControl(getItemHandle(IDC_SELECTION));
		this->m_list = ListViewControl(getItemHandle(IDC_LIST));

		SendMessageA(getItemHandle(IDC_ICONSIZE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"16x16");
		SendMessageA(getItemHandle(IDC_ICONSIZE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"24x24");
		SendMessageA(getItemHandle(IDC_ICONSIZE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"32x32");
		SendMessageA(getItemHandle(IDC_ICONSIZE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"48x48");
		SendMessageA(getItemHandle(IDC_ICONSIZE), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

		setIconSize(IconSize::_24x24);

		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\imageres.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\shell32.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\pifmgr.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\explorer.exe");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\accessibilitycpl.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\ddores.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\moricons.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\mmcndmgr.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\mmres.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\netcenter.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\netshell.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\networkexplorer.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\pnidui.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\sensorscpl.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\setupapi.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\wmploc.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\wpdshext.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\compstui.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\ieframe.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\dmdskres.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\dsuiext.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\mstscax.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\wiashext.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\comres.dll");
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)"%SystemRoot%\\system32\\mstsc.exe");
		SendMessageA(getItemHandle(IDC_FILE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		// set icon of program
		this->m_iconSmall = ImportedIcon("%SystemRoot%\\system32\\imageres.dll", 16, 87);
		this->m_iconLarge = ImportedIcon("%SystemRoot%\\system32\\imageres.dll", 48, 87);
	
		SendMessage(getWindowHandle(), WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)this->m_iconSmall.getIcon());
		SendMessage(getWindowHandle(), WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)this->m_iconLarge.getIcon());

		// notify init done
		notify(EVENT_INIT_DONE);
	}

	// hide on close
	virtual INT_PTR dialogProc(UINT uiMessage, WPARAM wParam, LPARAM lParam) override
	{
		switch (uiMessage)
		{
		case WM_CLOSE:
			notify(EVENT_CLOSE);
			return true;

		case WM_DESTROY:
			notify(EVENT_DESTROY);
			return true;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case ID_FILE_NEW:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_NEW);

					return true;
				}
				break;

			case ID_FILE_OPEN:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_OPEN);

					return true;
				}
				break;

			case ID_FILE_SAVE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_SAVE);

					return true;
				}
				break;

			case IDC_ADD:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_ADD);

					return true;
				}
				break;

			case IDC_MNEMONIC:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					notify(EVENT_MNEMONIC);

					return true;
				}
				break;

			case IDC_FILE:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					notify(EVENT_FILE);

					return true;
				}
				break;

			case IDC_ICONSIZE:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					notify(EVENT_SIZE);

					return true;
				}
				break;

			case IDC_REMOVE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					notify(EVENT_REMOVE);

					return true;
				}
				break;
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->idFrom)
			{
			case IDC_LIST:
				if (((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
				{
					notify(EVENT_LIST);

					return true;
				}
				break;

			case IDC_SELECTION:
				if (((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
				{
					notify(EVENT_SELECTION);

					return true;
				}
				break;
			}
			break;
		}

		return FALSE;
	}

	// update enable state
	void enableAll(bool bEnable)
	{
		bEnable &= this->m_bEnable;

		_enableGroup(bEnable);
		_enableRemove(bEnable);
		_enableAdd(bEnable);
		_enableSize(bEnable);
		_enableSelection(bEnable);
		_enableSave(bEnable);
		_enableOpen(bEnable);
		_enableNew(bEnable);
	}

	/* load & save */

	// load file
	void loadFile(const std::string& rFilename)
	{
		// cleanup filename
		char szFilename[MAX_PATH];
		strcpy_s(szFilename, rFilename.c_str());
		char* pszFilename = &szFilename[0];

		while (*pszFilename == '\"')
			pszFilename++;

		while (strlen(pszFilename) > 0 && pszFilename[strlen(pszFilename) - 1] == '\"')
			pszFilename[strlen(pszFilename) - 1] = '\0';

		// reset all elements
		reset();

		// load file
		this->m_icons.loadFromFile(pszFilename);

		// update
		notify(EVENT_UPDATE);
	}

	// save file
	void saveFile(const std::string& rFilename)
	{
		this->m_icons.saveToFile(rFilename);
	}

	/* get & set */

	// set icon size
	void setIconSize(IconSize eIconSize)
	{
		switch (eIconSize)
		{
		case IconSize::_16x16:
			SendMessage(getItemHandle(IDC_ICONSIZE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			break;

		case IconSize::_24x24:
			SendMessage(getItemHandle(IDC_ICONSIZE), CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
			break;

		case IconSize::_32x32:
			SendMessage(getItemHandle(IDC_ICONSIZE), CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
			break;

		case IconSize::_48x48:
			SendMessage(getItemHandle(IDC_ICONSIZE), CB_SETCURSEL, (WPARAM)3, (LPARAM)0);
			break;

		default:
			throw UnsupportedIconSize();
		}

		notify(EVENT_SIZE);
	}

	// get icon size
	IconSize getIconSize(void) const
	{
		int sel = (int)SendMessageA(getItemHandle(IDC_ICONSIZE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

		switch (sel)
		{
		case 0:
			return IconSize::_16x16;

		case 1:
			return IconSize::_24x24;

		case 2:
			return IconSize::_32x32;

		case 3:
			return IconSize::_48x48;
		}

		throw UnsupportedIconSize();
	}

	// set current mnemonic
	void setMnemonic(const std::string& rMnemonic)
	{
		SetDlgItemTextA(getWindowHandle(), IDC_MNEMONIC, rMnemonic.c_str());

		notify(EVENT_MNEMONIC);
	}

	// get current Mnemonic
	std::string getMnemonic(void) const
	{
		char szMnemonic[256];

		GetDlgItemTextA(getWindowHandle(), IDC_MNEMONIC, szMnemonic, sizeof(szMnemonic));

		return std::string(szMnemonic);
	}

	// set current file
	void setFile(const std::string& rFilePath)
	{
		// get number of item
		size_t nMaxCount = (size_t)(int)SendMessageA(getItemHandle(IDC_FILE), CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

		// browse
		for (size_t i = 0; i < nMaxCount; i++)
		{
			auto curr = getFile(i);

			// check it match
			if (_strcmpi(rFilePath.c_str(), curr.c_str()) == 0)
			{
				// select item and quit
				SendMessageA(getItemHandle(IDC_FILE), CB_SETCURSEL, (WPARAM)i, (LPARAM)0);

				// notify event
				notify(EVENT_FILE);

				// skip rest
				return;
			}
		}

		// insert new item and select it
		SendMessageA(getItemHandle(IDC_FILE), CB_ADDSTRING, (WPARAM)0, (LPARAM)rFilePath.c_str());
		SendMessageA(getItemHandle(IDC_FILE), CB_SETCURSEL, (WPARAM)nMaxCount, (LPARAM)0);

		// notify event
		notify(EVENT_FILE);
	}

	// get selection id
	std::string getFile(size_t nIndex) const
	{
		// check boundaries
		size_t nMaxCount = (size_t)(int)SendMessageA(getItemHandle(IDC_FILE), CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

		if (nMaxCount == 0)
			return "";

		nIndex = min(nIndex, nMaxCount - 1);

		// get text length
		int len = (int)SendMessageA(getItemHandle(IDC_FILE), CB_GETLBTEXTLEN, (WPARAM)nIndex, (LPARAM)0);

		if (len < 0)
			return "";

		// allocate
		char* pTemp = new char[len+1];

		// get content
		SendMessageA(getItemHandle(IDC_FILE), CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)pTemp);

		// copy
		std::string ret(pTemp);

		// free memory
		delete[] pTemp;

		// return result
		return ret;
	}

	// get current file
	std::string getFile(void) const
	{
		return getFile((size_t)SendMessageA(getItemHandle(IDC_FILE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0));
	}

	// return full path
	std::string getFullPathName(void) const
	{
		char szPath[MAX_PATH];
		auto filename = getFile();

		ExpandEnvironmentStringsA(filename.c_str(), szPath, sizeof(szPath));

		return std::string(szPath);
	}

private:

	// update components state on notifications
	virtual void onNotify(int iEvent) override
	{
		enableAll(true);
	}

	/* enable components */

	// enable new
	void _enableNew(bool bEnable)
	{
		EnableMenuItem(GetMenu(getWindowHandle()), ID_FILE_NEW, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	// enable open
	void _enableOpen(bool bEnable)
	{
		EnableMenuItem(GetMenu(getWindowHandle()), ID_FILE_OPEN, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	// enable save
	void _enableSave(bool bEnable)
	{
		bEnable &= !this->m_icons.empty();

		EnableMenuItem(GetMenu(getWindowHandle()), ID_FILE_SAVE, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	// enable selection
	void _enableSelection(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_SELECTION), (bEnable & !this->m_icons.empty()) ? TRUE : FALSE);
	}

	// enable size
	void _enableSize(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_SZ_ICONSIZE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_ICONSIZE), bEnable ? TRUE : FALSE);
	}

	// enable add
	void _enableAdd(bool bEnable)
	{
		EnableWindow(getItemHandle(IDC_SZ_MNEMONIC), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_MNEMONIC), bEnable ? TRUE : FALSE);

		bEnable &= (getMnemonic().length() > 0) && !this->m_icons.hasIcon(getMnemonic());

		EnableWindow(getItemHandle(IDC_ADD), bEnable ? TRUE : FALSE);
	}

	// enable remove
	void _enableRemove(bool bEnable)
	{
		bEnable &= this->m_pCurrIcon != nullptr;

		EnableWindow(getItemHandle(IDC_REMOVE), bEnable ? TRUE : FALSE);
	}

	// enable group
	void _enableGroup(bool bEnable)
	{
		bEnable &= (this->m_pCurrIcon != nullptr);

		EnableWindow(getItemHandle(IDC_GROUP), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_FILE), bEnable ? TRUE : FALSE);
		EnableWindow(getItemHandle(IDC_LIST), bEnable ? TRUE : FALSE);
	}

	/* actions */

	// enable all components
	void onEnableAll(void)
	{
		this->m_bEnable = true;
	}

	// disable all components
	void onDisableAll(void)
	{
		this->m_bEnable = false;
	}

	// close action
	void onClose(void)
	{
		show(false);

		DestroyWindow(getWindowHandle());
	}

	// destroy action
	void onDestroy(void)
	{
		// clear list
		this->m_selection.clear();
		this->m_list.clear();

		// post quit message
		PostQuitMessage(0);
	}

	// update action
	void onUpdate(void)
	{
		// clear list
		this->m_selection.clear();

		// get all icons
		auto callback = [&](const std::string& rMnemonic, const ImportedIcon& rIcon)
		{
			this->m_selection.add(rMnemonic, rIcon.getIcon());
		};

		this->m_icons.populate(callback);

		// build list
		this->m_selection.build(this->m_icons.getIconSize());

		// redraw
		redraw(getWindowHandle(), IDC_SELECTION);
	}

	// new action
	void onNew(void)
	{
		// popup message if current list is not empty
		if (!this->m_icons.empty() && MessageBox(getWindowHandle(), TEXT("Discard current content?"), TEXT("new"), MB_ICONWARNING | MB_YESNO) == IDNO)
			return;

		// reset all elements
		reset();

		// notify update
		notify(EVENT_UPDATE);
	}

	// reset elements
	void reset(void)
	{
		// clear list
		this->m_icons.clear();
		this->m_pCurrIcon = nullptr;

		this->m_list.clear();
		this->m_selection.clear();

		// clear text
		SetDlgItemTextA(getWindowHandle(), IDC_MNEMONIC, "");
		SetDlgItemTextA(getWindowHandle(), IDC_FILE, "");
	}

	// open action
	void onOpen(void)
	{
		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[] = "OpenRAMAN Icons Set (.iis)\0*.iis\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = getWindowHandle();
		sFile.hInstance = GetModuleHandle(NULL);
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

	// save action
	void onSave(void)
	{
		// cannot save empty
		if (this->m_icons.empty())
			return;

		char szFilename[MAX_PATH];
		char szTitle[256];
		char szFilter[] = "OpenRAMAN Icons Set (.iis)\0*.iis\0\0";

		OPENFILENAMEA sFile;

		memset(&sFile, 0, sizeof(OPENFILENAME));

		*szFilename = 0;

		sFile.lStructSize = sizeof(OPENFILENAME);
		sFile.hwndOwner = getWindowHandle();
		sFile.hInstance = GetModuleHandle(NULL);
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
			if (!strEndsWith(szFilename, ".iis"))
				strcat_s(szFilename, ".iis");
			break;
		}

		// save file
		saveFile(szFilename);
	}

	// selection action
	void onSelection(void)
	{
		// check if selection exists
		if (!this->m_selection.hasSelection())
		{
			this->m_pCurrIcon = nullptr;

			SetDlgItemTextA(getWindowHandle(), IDC_MNEMONIC, "");
			SetDlgItemTextA(getWindowHandle(), IDC_FILE, "");

			this->m_list.clear();

			return;
		}

		// get selection mnemonic
		auto label = this->m_selection.getSelection();

		// get icon
		this->m_pCurrIcon = &this->m_icons.get(label);

		// copy to mnemonic
		setMnemonic(label);

		// set filename
		setFile(this->m_pCurrIcon->getFile());
	}

	// add action
	void onAdd(void)
	{
		// get name
		auto mnemonic = getMnemonic();

		// skip if names already exists
		if (this->m_icons.hasIcon(mnemonic))
			return;

		// create new entry
		this->m_icons.addIcon(mnemonic);

		// update list
		notify(EVENT_UPDATE);

		// select item
		this->m_selection.select(mnemonic);
	}

	// mnemonic change action
	void onMnemonic(void) {}

	// file change action
	void onFile(void)
	{
		// clear list view
		this->m_list.clear();

		// get current filename
		std::string fullpathname = getFullPathName();

		// disable all components
		notify(EVENT_DISABLEALL);

		// add library to dependency walker
		try
		{
			this->m_libraries.addLibrary(fullpathname);

			if (this->m_libraries.hasLibrary(fullpathname))
			{
				// get library
				auto& library = this->m_libraries.getLibrary(fullpathname);

				// import icons if non-existing
				if (!library.hasIcons())
					library.importIcons(this->m_pCurrIcon->getSize());

				// populate list
				this->m_list.clear();

				auto callback = [&](int iIndex, HICON hIcon)
				{
					// add current to list
					char szTmp[64];
					sprintf_s(szTmp, "%d", iIndex);

					this->m_list.add(szTmp, hIcon);
				};

				library.populate(callback);

				// build list
				this->m_list.build(this->m_icons.getIconSize());
			}
		}
		catch (...) {}

		// enable all components
		notify(EVENT_ENABLEALL);

		// select current index
		char szTmp[64];
		sprintf_s(szTmp, "%d", this->m_pCurrIcon->getIndex());

		this->m_list.select(szTmp);
	}

	// remove action
	void onRemove(void)
	{
		// skip if no selection
		if (!this->m_selection.hasSelection())
			return;

		// remove selection
		this->m_icons.remove(this->m_selection.getSelection());
		this->m_pCurrIcon = nullptr;

		// notify update event to redraw all icons
		notify(EVENT_UPDATE);
	}

	// size action
	void onSize(void)
	{
		if (!this->m_icons.empty() && MessageBox(getWindowHandle(), TEXT("Changing size may affect some of the icons that have already been added. Proceed anyway?"), TEXT("icon size"), MB_ICONASTERISK | MB_YESNO) == IDYES)
		{
			// change icon size
			this->m_icons.resize(getIconSize());

			// notify update event to redraw all icons
			notify(EVENT_UPDATE);
		}
	}

	// list selection change action
	void onList(void)
	{
		// skip if no selection
		if (!this->m_list.hasSelection())
			return;

		// skip if no current icon
		if (this->m_pCurrIcon == nullptr)
			return;

		// get current selection
		auto list_selection = this->m_list.getSelection();
		auto selection_label = this->m_selection.getSelection();

		// update icon
		try
		{
			this->m_pCurrIcon->setFile(getFile());
		}
		catch (IconNotFoundException) {}

		// update icons set
		this->m_pCurrIcon->setIndex(atol(list_selection.c_str()));

		// update list
		this->m_selection.updateIcon(selection_label, this->m_pCurrIcon->getIcon());

		// redraw element
		redraw(getWindowHandle(), IDC_SELECTION);
	}

	// member variable
	bool m_bEnable;

	ImportedIconSet m_icons;
	ImportedIcon* m_pCurrIcon;
	ImportedIcon m_iconSmall, m_iconLarge;
	
	ListViewControl m_selection, m_list;
	IconDependencyWalker m_libraries;
};