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

    void AddItemToTree(HWND treeview, CActor *actor)
    {
        TVITEM tvi;
        tvi.mask = TVIF_TEXT | TVIF_PARAM;

        string actorName = actor->GetName();
        tvi.pszText = (LPSTR)actorName.c_str();

        tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);
        tvi.lParam = (LPARAM)actor;

        TVINSERTSTRUCT tvins;
        tvins.item = tvi;
        tvins.hInsertAfter = (HTREEITEM)TVI_FIRST;
        tvins.hParent = TVI_ROOT;

        SendMessage(treeview, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
    }

    void TreeviewHandler(HWND treeview, WPARAM wParam, LPARAM lParam, CScene &scene)
    {
        switch (wParam)
        {
            case TV_ADD_ACTOR:
                AddItemToTree(treeview, (CActor*)lParam);
                break;
            case TV_CLEAR_ACTORS:
                TreeView_DeleteAllItems(treeview);
                break;
            case TVN_SELCHANGED:
            {
                auto pnmtv = (LPNMTREEVIEW)lParam;
                if (pnmtv->action != 0)
                {
                    auto actor = (CActor*)pnmtv->itemNew.lParam;
                    scene.SelectActorById(actor->GetId());
                }
                break;
            }
            case TVN_KEYDOWN:
            {
                auto info = (LPNMTVKEYDOWN)lParam;
                switch (info->wVKey)
                {
                    case VK_DELETE:
                        scene.Delete();
                        break;
                }
                break;
            }
        }
    }
}
