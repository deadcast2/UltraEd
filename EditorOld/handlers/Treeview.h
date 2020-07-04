#pragma once

namespace UltraEd
{
    HWND treeview;
    BOOL resizingTreeView = false;
    int treeviewWidth = 160; // Starting width
    const int treeviewBorder = 2;

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

        TreeView_InsertItem(treeview, (LPTVINSERTSTRUCT)& tvins);
    }

    void TreeviewHandler(HWND treeview, HWND hWnd, WPARAM wParam, LPARAM lParam, CScene *scene)
    {
        switch (wParam)
        {
            case TV_ADD_ACTOR:
                AddItemToTree(treeview, (CActor *)lParam);
                break;
            case TV_CLEAR_ACTORS:
                TreeView_DeleteAllItems(treeview);
                break;
            case TV_SELECT_ACTOR:
            {
                TVITEM tvitem = { 0 };
                auto selectedActor = (CActor *)lParam;
                auto currentItem = TreeView_GetRoot(treeview);

                while (currentItem != NULL)
                {
                    tvitem.hItem = currentItem;
                    TreeView_GetItem(treeview, &tvitem);
                    auto actor = (CActor *)tvitem.lParam;

                    if (actor->GetId() == selectedActor->GetId())
                    {
                        TreeView_SelectItem(treeview, tvitem.hItem);
                        SetFocus(treeview);
                        break;
                    }

                    currentItem = TreeView_GetNextSibling(treeview, currentItem);
                }

                break;
            }
            case TVN_SELCHANGED:
            {
                auto pnmtv = (LPNMTREEVIEW)lParam;
                if (pnmtv->action != 0)
                {
                    auto actor = (CActor *)pnmtv->itemNew.lParam;
                    if (scene != NULL) scene->SelectActorById(actor->GetId());
                }
                break;
            }
            case TVN_KEYDOWN:
            {
                auto info = (LPNMTVKEYDOWN)lParam;
                KeyDownHandler(info->wVKey, scene);
                break;
            }
            case NM_RCLICK:
            {
                TVITEM tvitem = { 0 };
                tvitem.hItem = TreeView_GetSelection(treeview);
                TreeView_GetItem(treeview, &tvitem);
                auto actor = (CActor *)tvitem.lParam;

                POINT point = { 0 };
                GetCursorPos(&point);

                if (actor != NULL) MouseMenuCreate(hWnd, point, actor);
                break;
            }
        }
    }
}
