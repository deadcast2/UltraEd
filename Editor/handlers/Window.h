#pragma once

#include <windows.h>
#include "SceneSettings.h"
#include "Menu.h"
#include "MouseMenu.h"
#include "Toolbar.h"
#include "KeyDown.h"
#include "ScriptEditor.h"
#include "Treeview.h"
#include "Tabs.h"

namespace UltraEd
{
    HWND parentWindow, renderWindow, statusBar;
    HCURSOR hcSizeCursor;
    DWORD mouseClickTick = 0;
    const int mouseWaitPeriod = 250; // milliseconds

    LRESULT CALLBACK WindowHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        CScene *scene = (CScene *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        switch (message)
        {
            case WM_CREATE:
            {
                hcSizeCursor = LoadCursor(NULL, IDC_SIZEWE);
                break;
            }
            case WM_KEYDOWN:
            {
                KeyDownHandler(LOWORD(wParam), scene);
                break;
            }
            case WM_COMMAND:
            {
                UltraEd::MenuHandler(statusBar, hWnd, wParam, scene);
                UltraEd::ToolbarHandler(wParam, scene);
                UltraEd::TreeviewHandler(treeview, hWnd, wParam, lParam, scene);
                UltraEd::MouseMenuHandler(UltraEd::ScriptEditorHandler, hWnd, wParam, scene);
                UltraEd::TabHandler(wParam, lParam);
                break;
            }
            case WM_NOTIFY:
            {
                UltraEd::TreeviewHandler(treeview, hWnd, ((LPNMHDR)lParam)->code, lParam, scene);
                break;
            }
            case WM_MOUSEMOVE:
            {
                if (UltraEd::IsMouseOverSplitter(treeview, wParam, lParam)) SetCursor(hcSizeCursor);
                if (UltraEd::resizingTreeView && wParam == MK_LBUTTON)
                {
                    // Track new width of treeview.
                    RECT parentRect;
                    GetClientRect(parentWindow, &parentRect);
                    RECT treeviewRect;
                    GetClientRect(treeview, &treeviewRect);
                    int xPosition = LOWORD(lParam);
                    if (xPosition > parentRect.right) xPosition -= USHRT_MAX;
                    UltraEd::treeviewWidth = treeviewRect.right + xPosition;

                    // Draw resize preview rectangle.
                    RECT toolbarRect;
                    GetClientRect(toolbarWindow, &toolbarRect);
                    RECT statusRect;
                    GetClientRect(statusBar, &statusRect);
                    RECT resizeBox;
                    HDC hDC = GetDC(parentWindow);
                    SetRect(&resizeBox, 0, toolbarRect.bottom + treeviewBorder,
                        treeviewWidth, parentRect.bottom - statusRect.bottom);
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
                    CActor *selectedActor = NULL;
                    POINT point = { LOWORD(lParam), HIWORD(lParam) };

                    if (scene->Pick(point, &selectedActor))
                    {
                        ClientToScreen(hWnd, &point);
                        MouseMenuCreate(hWnd, point, selectedActor);
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
                    MoveWindow(treeview, 0, toolbarRect.bottom + treeviewBorder, treeviewWidth,
                        HIWORD(lParam) - (toolbarRect.bottom + statusRect.bottom + treeviewBorder), 1);
                    MoveWindow(statusBar, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);

                    RECT parentRect;
                    GetClientRect(parentWindow, &parentRect);
                    RECT treeviewRect;
                    GetClientRect(treeview, &treeviewRect);

                    MoveWindow(tabsWindow, treeviewRect.right,
                        parentRect.bottom - statusRect.bottom - UltraEd::tabsWindowHeight,
                        parentRect.right - treeviewRect.right + tabsBorder, parentRect.bottom, 1);

                    RECT tabsWindowRect;
                    GetClientRect(tabsWindow, &tabsWindowRect);

                    MoveWindow(buildOutput, 8, 28, tabsWindowRect.right - 14, UltraEd::tabsWindowHeight -
                        statusRect.bottom - 8, 1);

                    const int renderWidth = LOWORD(lParam) - treeviewRect.right;
                    const int renderHeight = HIWORD(lParam) - statusRect.bottom - tabsWindowHeight -
                        tabsBorder - toolbarRect.bottom;
                    if (renderWidth > 1 && renderHeight > 1)
                    {
                        MoveWindow(renderWindow, treeviewRect.right + treeviewBorder, toolbarRect.bottom + treeviewBorder,
                            renderWidth, renderHeight, 1);

                        if (scene != NULL) scene->Resize();
                    }
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
            case WM_CLOSE:
                if (scene == NULL || scene->Confirm()) PostQuitMessage(0);
                break;
            default:
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        return 0;
    }
}
