#pragma once

#include <windows.h>
#include <commctrl.h>

namespace UltraEd
{
    BOOL IsMouseOverSplitter(HWND treeview, WPARAM wParam, LPARAM lParam)
    {
        POINT point = { LOWORD(lParam), HIWORD(lParam) };
        RECT treeviewRect;
        GetClientRect(treeview, &treeviewRect);
        return point.x <= 8;
    }

    void AddItemToTree(HWND treeview, LPTSTR text)
    {
        TVITEM tvi;
        tvi.mask = TVIF_TEXT;
        tvi.pszText = text;
        tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);

        TVINSERTSTRUCT tvins;
        tvins.item = tvi;
        tvins.hInsertAfter = (HTREEITEM)TVI_FIRST;
        tvins.hParent = TVI_ROOT;

        SendMessage(treeview, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
    }

    void TreeviewHandler(HWND treeview, WPARAM wParam, LPARAM lParam)
    {
        switch (wParam)
        {
            case TV_ADD_ACTOR:
                AddItemToTree(treeview, (LPSTR)lParam);
                break;
            case TV_CLEAR_ACTORS:
                TreeView_DeleteAllItems(treeview);
                break;
        }
    }
}
