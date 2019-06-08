#pragma once

#include <windows.h>
#include "../resource.h"
#include "../Scene.h"

namespace UltraEd
{
    void MenuHandler(HWND statusBar, HWND hWnd, WPARAM wParam, CScene &scene)
    {
        switch (LOWORD(wParam))
        {
            case ID_FILE_NEWSCENE:
                if (MessageBox(hWnd, "All scene data will be erased.", "Are you sure?", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
                {
                    scene.OnNew();
                }
                break;
            case ID_FILE_SAVESCENE:
                scene.OnSave();
                break;
            case ID_FILE_LOADSCENE:
                scene.OnLoad();
                break;
            case ID_FILE_EXIT:
                PostQuitMessage(0);
                break;
            case ID_FILE_BUILDROM:
                CUtil::RunAction(statusBar, "Building ROM...", [&scene] { scene.OnBuildROM(BuildFlag::_); });
                break;
            case ID_FILE_BUILDROM_AND_RUN:
                CUtil::RunAction(statusBar, "Building ROM...", [&scene] { scene.OnBuildROM(BuildFlag::Run); });
                break;
            case ID_FILE_BUILDROM_AND_LOAD:
                CUtil::RunAction(statusBar, "Building ROM...", [&scene] { scene.OnBuildROM(BuildFlag::Load); });
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
            case ID_ADD_CAMERA:
                scene.OnAddCamera();
                break;
            case ID_ADD_MODEL:
                scene.OnImportModel();
                break;
            case ID_ADD_TEXTURE:
                scene.OnApplyTexture();
                break;
            case ID_RENDER_SOLID:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene.ToggleFillMode();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
            case ID_MOVEMENT_WORLDSPACE:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene.ToggleMovementSpace();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
            case ID_MOVEMENT_SNAPTOGRID:
            {
                HMENU menu = GetMenu(hWnd);
                if (menu != NULL)
                {
                    bool toggled = scene.ToggleSnapToGrid();
                    CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
                }
                break;
            }
        }
    }
}
