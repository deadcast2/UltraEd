#pragma once

#include <windows.h>
#include "../FileIO.h"
#include "../resource.h"

namespace UltraEd
{
    void MenuHandler(HWND statusBar, HWND hWnd, WPARAM wParam, LPARAM lParam, CScene *scene)
    {
        if (scene == NULL) return;

        switch (LOWORD(wParam))
        {
            case ID_FILE_NEWSCENE:
                scene->OnNew();
                break;
            case ID_FILE_SAVESCENE:
                scene->OnSave();
                break;
            case ID_FILE_LOADSCENE:
                scene->OnLoad();
                break;
            case ID_FILE_SCENESETTINGS:
                DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_SCENE_SETTINGS), hWnd, (DLGPROC)SceneSettingsProc, (LPARAM)scene);
                break;
            case ID_FILE_EXIT:
                SendMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            case ID_FILE_BUILDROM:
                CUtil::RunAction(statusBar, "Building ROM...", [scene] { scene->OnBuildROM(BuildFlag::_); });
                break;
            case ID_FILE_BUILDROM_AND_RUN:
                CUtil::RunAction(statusBar, "Building ROM...", [scene] { scene->OnBuildROM(BuildFlag::Run); });
                break;
            case ID_FILE_BUILDROM_AND_LOAD:
                CUtil::RunAction(statusBar, "Building ROM...", [scene] { scene->OnBuildROM(BuildFlag::Load); });
                break;
            case ID_INSTALL_BUILD_TOOLS:
            {
                CUtil::RunAction(statusBar, "Installing build tools...", [hWnd] {
                    char pathBuffer[128];
                    GetFullPathName("..\\Engine\\tools.bin", 128, pathBuffer, NULL);
                    if (UltraEd::CFileIO::Unpack(pathBuffer))
                    {
                        MessageBox(hWnd, "Build tools successfully installed.", "Success!", MB_OK);
                    }
                    else
                    {
                        MessageBox(hWnd, "Could not find build tools.", "Error", MB_OK);
                    }
                });
                break;
            }
            case ID_EDIT_UNDO:
                scene->Undo();
                break;
            case ID_EDIT_REDO:
                scene->Redo();
                break;
            case ID_EDIT_DUPLICATE:
                scene->Duplicate();
                break;
            case ID_EDIT_DELETE:
                scene->Delete();
                break;
            case ID_EDIT_SELECTALL:
                scene->SelectAll();
                break;
            case ID_ACTOR_CAMERA:
                scene->OnAddCamera();
                break;
            case ID_ACTOR_MODEL:
                scene->OnAddModel();
                break;
            case ID_ACTOR_TEXTURE_ADD:
                scene->OnAddTexture();
                break;
            case ID_ACTOR_TEXTURE_DELETE:
                scene->OnDeleteTexture();
                break;
            case ID_ACTOR_COLLIDER_BOX:
                scene->OnAddCollider(ColliderType::Box);
                break;
            case ID_ACTOR_COLLIDER_SPHERE:
                scene->OnAddCollider(ColliderType::Sphere);
                break;
            case ID_ACTOR_COLLIDER_DELETE:
                scene->OnDeleteCollider();
                break;
            case ID_RENDER_SOLID:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene->ToggleFillMode();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
            case ID_MOVEMENT_WORLDSPACE:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene->ToggleMovementSpace();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
            case ID_MOVEMENT_SNAPTOGRID:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene->ToggleSnapToGrid();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
            case UPDATE_UNDO:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    MENUITEMINFO info = { 0 };
                    info.cbSize = sizeof(MENUITEMINFO);
                    info.fMask = MIIM_STRING;
                    info.dwTypeData = (LPSTR)lParam;
                    SetMenuItemInfo(menu, ID_EDIT_UNDO, 0, &info);
                }
                break;
            }
            case UPDATE_REDO:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    MENUITEMINFO info = { 0 };
                    info.cbSize = sizeof(MENUITEMINFO);
                    info.fMask = MIIM_STRING;
                    info.dwTypeData = (LPSTR)lParam;
                    SetMenuItemInfo(menu, ID_EDIT_REDO, 0, &info);
                }
                break;
            }
        }
    }
}
