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
#include <vector>
#include <memory>

#include <Windows.h>
#include <CommCtrl.h>

#include "shared/utils/exception.h"

// NoSelectionException exception class
class NoSelectionException : public IException
{
public:
    virtual std::string toString(void) const override
    {
        return "no data selected!";
    }
};

// ListView control class
class ListViewControl
{
public:
    // default ctor
    ListViewControl(void)
    {
        this->m_hWnd = NULL;
        this->m_hImageList = NULL;
    }

    // ctor by giving HWND component
    ListViewControl(HWND hWnd)
    {
        this->m_hWnd = hWnd;
        this->m_hImageList = NULL;
    }

    // copy ctor
    ListViewControl(const ListViewControl& rListViewControl) : ListViewControl()
    {
        this->operator=(rListViewControl);
    }

    // move ctor
    ListViewControl(ListViewControl&& rListViewControl) : ListViewControl()
    {
        this->operator=(std::move(rListViewControl));
    }

    // dtor
    ~ListViewControl(void)
    {
        // clear all on destruction
        try
        {
            clear();
        }
        catch (...) {}
    }
    
    // copy operator
    const ListViewControl& operator=(const ListViewControl& rListViewControl)
    {
        this->m_hWnd = rListViewControl.m_hWnd;
        this->m_data = rListViewControl.m_data;

        return *this;
    }

    // move operator
    const ListViewControl& operator=(ListViewControl&& rListViewControl)
    {
        this->m_hWnd = rListViewControl.m_hWnd;
        rListViewControl.m_hWnd = NULL;

        this->m_hImageList = rListViewControl.m_hImageList;
        rListViewControl.m_hImageList = NULL;

        this->m_data = std::move(rListViewControl.m_data);

        return *this;
    }

    // clear list
    void clear(void)
    {
        this->m_data.clear();

        reset();
    }

    // build list
    void build(size_t nIconSizes)
    {
        // reset everything first
        reset();

        // skip if no HWND
        if (this->m_hWnd == NULL)
            return;

        // recreate list
        LVITEMA item;

        item.pszText = 0;
        item.mask = LVIF_IMAGE | LVIF_TEXT;
        item.stateMask = (UINT)-1;
        item.iSubItem = 0;

        this->m_hImageList = ImageList_Create(nIconSizes, nIconSizes, ILC_COLOR32, this->m_data.size(), 0);

        for (size_t i = 0; i < this->m_data.size(); i++)
        {
            // add icon to list
            ImageList_AddIcon(this->m_hImageList, this->m_data[i].hIcon);

            // create item
            item.iItem = i;
            item.iImage = i;
            item.pszText = (char*)this->m_data[i].label.c_str();

            // insert item
            SendMessageA(this->m_hWnd, LVM_INSERTITEMA, (WPARAM)0, (LPARAM)&item);
        }

        // set image list
        ListView_SetImageList(this->m_hWnd, this->m_hImageList, LVSIL_NORMAL);
        ListView_SetImageList(this->m_hWnd, this->m_hImageList, LVSIL_SMALL);
    }

    // add item
    void add(const std::string& rLabel, HICON hIcon)
    {
        struct data_s s;

        s.label = rLabel;
        s.hIcon = hIcon;

        this->m_data.emplace_back(std::move(s));
    }

    // return true if selection exists
    bool hasSelection(void) const
    {
        return ListView_GetSelectedCount(this->m_hWnd) == 1;
    }

    // return label of selection
    std::string getSelection(void) const
    {
        for (size_t i = 0; i < this->m_data.size(); i++)
        {
            // skip if not selected
            int mask = ListView_GetItemState(this->m_hWnd, i, LVIS_SELECTED);

            if ((mask & LVIS_SELECTED) == 0)
                continue;
            
            return this->m_data[i].label;
        }

        // throw exception if nothing return
        throw NoSelectionException();
    }

    // select specific item, return true when succesful
    bool select(const std::string& rLabel)
    {
        // find label
        for (size_t i = 0; i < this->m_data.size(); i++)
        {
            // skip if not element
            if (this->m_data[i].label != rLabel)
                continue;

            // set state
            ListView_SetItemState(this->m_hWnd, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

            // scroll to element
            ListView_EnsureVisible(this->m_hWnd, i, TRUE);

            // quit sucessfuly
            return true;
        }

        // element was not found
        return false;
    }

    // update current selection icon
    bool updateIcon(const std::string& rLabel, HICON hIcon)
    {
        // browse list
        for (size_t i = 0; i < this->m_data.size(); i++)
        {
            // skip if not label
            if (this->m_data[i].label != rLabel)
                continue;

            // set icon
            this->m_data[i].hIcon = hIcon;

            ImageList_ReplaceIcon(this->m_hImageList, i, hIcon);

            return true;
        }

        return false;
    }

private:

    // reset list
    void reset(void)
    {
        // clear all items
        if (this->m_hWnd != NULL)
            ListView_DeleteAllItems(this->m_hWnd);

        // clear image list
        if (this->m_hImageList != NULL)
            ImageList_Destroy(this->m_hImageList);

        this->m_hImageList = NULL;
    }

    struct data_s
    {
        std::string label;
        HICON hIcon;
    };

    std::vector<struct data_s> m_data;

    HWND m_hWnd;
    HIMAGELIST m_hImageList;
};