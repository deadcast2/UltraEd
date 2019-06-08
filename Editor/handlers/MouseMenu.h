#pragma once

#include <windows.h>
#include "../resource.h"
#include "../Scene.h"

#define IDM_MENU_DELETE_OBJECT 9001
#define IDM_MENU_DUPLICATE_OBJECT 9002
#define IDM_MENU_MODIFY_SCRIPT_OBJECT 9003
#define IDM_MENU_ADD_TEXTURE 9004

namespace UltraEd
{
    void MouseMenuHandler(DLGPROC ScriptEditorProc, HWND hWnd, WPARAM wParam, CScene &scene)
    {
        switch (LOWORD(wParam))
        {
            case IDM_MENU_DELETE_OBJECT:
                scene.Delete();
                break;
            case IDM_MENU_DUPLICATE_OBJECT:
                scene.Duplicate();
                break;
            case IDM_MENU_MODIFY_SCRIPT_OBJECT:
                DialogBox(NULL, MAKEINTRESOURCE(IDD_SCRIPT_EDITOR), hWnd, ScriptEditorProc);
                break;
            case IDM_MENU_ADD_TEXTURE:
                scene.OnApplyTexture();
                break;
        }
    }
}
