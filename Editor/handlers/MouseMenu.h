#pragma once

namespace UltraEd
{
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
