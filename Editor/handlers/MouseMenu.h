#pragma once

namespace UltraEd
{
    void MouseMenuCreate(HWND hWnd, const POINT point, CActor *selectedActor)
    {
        if (selectedActor == NULL) return;

        HMENU menu = CreatePopupMenu();

        if (selectedActor->GetType() == ActorType::Model)
        {
            AppendMenu(menu, MF_STRING, IDM_MENU_ADD_TEXTURE, _T("Add Texture"));
            if (reinterpret_cast<CModel *>(selectedActor)->HasTexture())
            {
                AppendMenu(menu, MF_STRING, IDM_MENU_REMOVE_TEXTURE, _T("Remove Texture"));
            }
        }

        HMENU colliderMenu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)colliderMenu, _T("Collider"));
        AppendMenu(colliderMenu, MF_STRING, IDM_MENU_ADD_BOX_COLLIDER, _T("Add Box"));
        AppendMenu(colliderMenu, MF_STRING, IDM_MENU_ADD_SPHERE_COLLIDER, _T("Add Sphere"));
        if (selectedActor->HasCollider())
        {
            AppendMenu(colliderMenu, MF_SEPARATOR, NULL, NULL);
            AppendMenu(colliderMenu, MF_STRING, IDM_MENU_DELETE_COLLIDER, _T("Delete"));
        }

        AppendMenu(menu, MF_STRING, IDM_MENU_MODIFY_SCRIPT_OBJECT, _T("Modify Script"));
        AppendMenu(menu, MF_STRING, IDM_MENU_DELETE_OBJECT, _T("Delete"));
        AppendMenu(menu, MF_STRING, IDM_MENU_DUPLICATE_OBJECT, _T("Duplicate"));
        TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
        DestroyMenu(menu);
        DestroyMenu(colliderMenu);
    }

    void MouseMenuHandler(DLGPROC ScriptEditorProc, HWND hWnd, WPARAM wParam, CScene *scene)
    {
        if (scene == NULL) return;

        switch (LOWORD(wParam))
        {
            case IDM_MENU_DELETE_OBJECT:
                scene->Delete();
                break;
            case IDM_MENU_DUPLICATE_OBJECT:
                scene->Duplicate();
                break;
            case IDM_MENU_MODIFY_SCRIPT_OBJECT:
                DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_SCRIPT_EDITOR), hWnd, ScriptEditorProc, (LPARAM)scene);
                break;
            case IDM_MENU_ADD_TEXTURE:
                scene->OnApplyTexture();
                break;
            case IDM_MENU_REMOVE_TEXTURE:
                scene->OnRemoveTexture();
                break;
            case IDM_MENU_ADD_BOX_COLLIDER:
                scene->OnAddCollider(ColliderType::Box);
                break;
            case IDM_MENU_ADD_SPHERE_COLLIDER:
                scene->OnAddCollider(ColliderType::Sphere);
                break;
            case IDM_MENU_DELETE_COLLIDER:
                scene->OnDeleteCollider();
                break;
        }
    }
}
