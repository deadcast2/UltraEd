#pragma once

#include <windows.h>
#include "Menu.h"
#include "MouseMenu.h"
#include "Toolbar.h"
#include "Treeview.h"
#include "ScriptEditor.h"

namespace UltraEd
{
    HWND parentWindow, renderWindow, statusBar;
    HCURSOR hcSizeCursor;
    DWORD mouseClickTick = 0;
    const int mouseWaitPeriod = 250; // milliseconds

    LRESULT CALLBACK WindowHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        CScene *scene = (CScene*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        switch (message)
        {
            case WM_CREATE:
            {
                hcSizeCursor = LoadCursor(NULL, IDC_SIZEWE);
                break;
            }
            case WM_KEYDOWN:
            {
                if (scene != NULL)
                {
                    switch (LOWORD(wParam))
                    {
                        case VK_DELETE:
                            scene->Delete();
                            break;
                        case 'D':
                            if (GetKeyState(VK_CONTROL) & 0x8000) scene->Duplicate();
                            break;
                    }
                }
                break;
            }
            case WM_COMMAND:
            {
                UltraEd::MenuHandler(statusBar, hWnd, wParam, scene);
                UltraEd::ToolbarHandler(wParam, scene);
                UltraEd::TreeviewHandler(treeview, wParam, lParam, scene);
                UltraEd::MouseMenuHandler(UltraEd::ScriptEditorHandler, hWnd, wParam, scene);
                break;
            }
            case WM_NOTIFY:
            {
                UltraEd::TreeviewHandler(treeview, ((LPNMHDR)lParam)->code, lParam, scene);
                break;
            }
            case WM_MOUSEMOVE:
            {
                if (UltraEd::IsMouseOverSplitter(treeview, wParam, lParam)) SetCursor(hcSizeCursor);
                if (UltraEd::resizingTreeView && wParam == MK_LBUTTON)
                {
                    // Track new width of treeview.
                    RECT parentRect;
                    GetClientRect(renderWindow, &parentRect);
                    RECT treeviewRect;
                    GetClientRect(treeview, &treeviewRect);
                    int xPosition = LOWORD(lParam);
                    if (xPosition > parentRect.right) xPosition -= USHRT_MAX;
                    UltraEd::treeviewWidth = treeviewRect.right + xPosition;

                    // Draw resize preview rectangle.
                    RECT renderRect;
                    GetClientRect(renderWindow, &renderRect);
                    RECT toolbarRect;
                    GetClientRect(toolbarWindow, &toolbarRect);
                    RECT resizeBox;
                    HDC hDC = GetDC(parentWindow);
                    SetRect(&resizeBox, 0, toolbarRect.bottom + treeviewBorder, UltraEd::treeviewWidth,
                        renderRect.bottom - treeviewBorder);
                    DrawFocusRect(hDC, &resizeBox);
                    ReleaseDC(hWnd, hDC);
                }
                break;
            }
            case WM_MOUSEWHEEL:
            {
                if (scene != NULL) scene->OnMouseWheel(HIWORD(wParam));
                break;
            }
            case WM_LBUTTONDOWN:
            {
                if (UltraEd::IsMouseOverSplitter(treeview, wParam, lParam))
                {
                    SetCursor(hcSizeCursor);
                    SetCapture(hWnd);
                    UltraEd::resizingTreeView = true;
                    break;
                }
                POINT point = { LOWORD(lParam), HIWORD(lParam) };
                if (scene != NULL) scene->Pick(point);
                break;
            }
            case WM_LBUTTONUP:
            {
                if (UltraEd::resizingTreeView)
                {
                    RECT parentRect;
                    ReleaseCapture();
                    GetClientRect(parentWindow, &parentRect);
                    PostMessage(parentWindow, WM_SIZE, wParam, MAKELPARAM(parentRect.right, parentRect.bottom));
                    UltraEd::resizingTreeView = false;
                }
                break;
            }
            case WM_RBUTTONDOWN:
            {
                mouseClickTick = GetTickCount();
                break;
            }
            case WM_RBUTTONUP:
            {
                // Only show menu when doing a fast click so
                // it doesn't show after dragging.
                if (GetTickCount() - mouseClickTick < mouseWaitPeriod)
                {
                    POINT point = { LOWORD(lParam), HIWORD(lParam) };
                    CActor *selectedActor = NULL;

                    if (scene->Pick(point, &selectedActor))
                    {
                        ClientToScreen(hWnd, &point);
                        HMENU menu = CreatePopupMenu();

                        if (selectedActor != NULL && selectedActor->GetType() == ActorType::Model)
                        {
                            AppendMenu(menu, MF_STRING, IDM_MENU_ADD_TEXTURE, _T("Add Texture"));
                        }

                        HMENU colliderMenu = CreatePopupMenu();
                        AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)colliderMenu, _T("Collider"));
                        AppendMenu(colliderMenu, MF_STRING, IDM_MENU_ADD_BOX_COLLIDER, _T("Add Box"));
                        AppendMenu(colliderMenu, MF_STRING, IDM_MENU_ADD_SPHERE_COLLIDER, _T("Add Sphere"));
                        AppendMenu(colliderMenu, MF_SEPARATOR, NULL, NULL);
                        AppendMenu(colliderMenu, MF_STRING, IDM_MENU_DELETE_COLLIDER, _T("Delete"));

                        AppendMenu(menu, MF_STRING, IDM_MENU_MODIFY_SCRIPT_OBJECT, _T("Modify Script"));
                        AppendMenu(menu, MF_STRING, IDM_MENU_DELETE_OBJECT, _T("Delete"));
                        AppendMenu(menu, MF_STRING, IDM_MENU_DUPLICATE_OBJECT, _T("Duplicate"));
                        TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
                        DestroyMenu(menu);
                        DestroyMenu(colliderMenu);
                    }
                }
                break;
            }
            case WM_SIZE:
            {
                if (wParam != SIZE_MINIMIZED && hWnd == parentWindow)
                {
                    // Resize the child windows and the scene.
                    RECT toolbarRect;
                    GetClientRect(toolbarWindow, &toolbarRect);
                    RECT statusRect;
                    GetClientRect(statusBar, &statusRect);

                    MoveWindow(toolbarWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);
                    MoveWindow(treeview, 0, toolbarRect.bottom + treeviewBorder, UltraEd::treeviewWidth,
                        HIWORD(lParam) - (toolbarRect.bottom + statusRect.bottom + treeviewBorder), 1);
                    MoveWindow(statusBar, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);

                    RECT treeviewRect;
                    GetClientRect(treeview, &treeviewRect);
                    MoveWindow(renderWindow, treeviewRect.right + treeviewBorder, 0,
                        LOWORD(lParam) - treeviewRect.right, HIWORD(lParam) - statusRect.bottom, 1);
                    if(scene != NULL) scene->Resize();
                }
                break;
            }
            case WM_ERASEBKGND:
            {
                return 1;
            }
            case WM_DESTROY:
            {
                PostQuitMessage(0);
                break;
            }
            default:
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        return 0;
    }
}
