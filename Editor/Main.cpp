#define WIN32_LEAN_AND_MEAN

// Enable mouse wheel scrolling.
#define _WIN32_WINDOWS 0x0501

#pragma comment(lib, "deps/DXSDK/lib/d3dx8.lib")
#pragma comment(lib, "deps/DXSDK/lib/d3d8.lib")
#pragma comment(lib, "deps/Assimp/lib/assimp-vc140-mt.lib")

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "resource.h"
#include "Scene.h"
#include "Settings.h"
#include "vendor/Scintilla.h"
#include "vendor/SciLexer.h"
#include "FileIO.h"
#include "handlers/Menu.h"
#include "handlers/MouseMenu.h"
#include "handlers/Toolbar.h"
#include "handlers/Treeview.h"

#define IDM_STATUS_BAR 9998

const int treeviewBorder = 2;
const int windowWidth = 800;
const int windowHeight = 600;
const int mouseWaitPeriod = 250; // milliseconds
const TCHAR szWindowClass[] = APP_NAME;
const TCHAR szTitle[] = _T("Loading");

HWND parentWindow, toolbarWindow, statusBar, treeview, renderWindow, scriptEditorWindow;
UltraEd::CScene scene;
DWORD mouseClickTick = 0;
HCURSOR hcSizeCursor;
BOOL resizingTreeView = false;
int treeviewWidth = 160; // Starting width

BOOL CALLBACK ScriptEditorProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            RECT rc;
            GetClientRect(hWndDlg, &rc);
            scriptEditorWindow = CreateWindow(
                "Scintilla",
                "Source",
                WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
                0, 0,
                rc.right, rc.bottom - 50,
                hWndDlg,
                0,
                0,
                0);
            SendMessage(scriptEditorWindow, SCI_SETLEXER, SCLEX_CPP, 0);
            SendMessage(scriptEditorWindow, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
            SendMessage(scriptEditorWindow, SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>("Verdana"));
            SendMessage(scriptEditorWindow, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(scene.GetScript().c_str()));
            ShowWindow(scriptEditorWindow, SW_SHOW);
            SetFocus(scriptEditorWindow);
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_SCRIPT_EDITOR_SAVE_CHANGES:
                {
                    HRESULT length = SendMessage(scriptEditorWindow, SCI_GETLENGTH, 0, 0) + 1;
                    char *buffer = (char*)malloc(sizeof(char) * length);
                    SendMessage(scriptEditorWindow, SCI_GETTEXT, length, reinterpret_cast<LPARAM>(buffer));
                    scene.SetScript(string(buffer));
                    free(buffer);
                    SendMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0);
                    return TRUE;
                }
                case IDCANCEL:
                    EndDialog(hWndDlg, wParam);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_KEYDOWN:
        {
            switch (LOWORD(wParam))
            {
                case VK_DELETE:
                    scene.Delete();
                    break;
                case 'D':
                    if (GetKeyState(VK_CONTROL) & 0x8000) scene.Duplicate();
                    break;
            }
        }
        case WM_COMMAND:
        {
            UltraEd::MenuHandler(statusBar, hWnd, wParam, scene);
            UltraEd::ToolbarHandler(wParam, scene);
            UltraEd::TreeviewHandler(treeview, wParam, lParam);
            UltraEd::MouseMenuHandler(ScriptEditorProc, hWnd, wParam, scene);
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (UltraEd::IsMouseOverSplitter(treeview, wParam, lParam)) SetCursor(hcSizeCursor);
            if (resizingTreeView && wParam == MK_LBUTTON)
            {
                // Track new width of treeview.
                RECT parentRect;
                GetClientRect(renderWindow, &parentRect);
                RECT treeviewRect;
                GetClientRect(treeview, &treeviewRect);
                int xPosition = LOWORD(lParam);
                if (xPosition > parentRect.right) xPosition -= USHRT_MAX;
                treeviewWidth = treeviewRect.right + xPosition;

                // Draw resize preview rectangle.
                RECT renderRect;
                GetClientRect(renderWindow, &renderRect);
                RECT toolbarRect;
                GetClientRect(toolbarWindow, &toolbarRect);
                RECT resizeBox;
                HDC hDC = GetDC(parentWindow);
                SetRect(&resizeBox, 0, toolbarRect.bottom + treeviewBorder, treeviewWidth,
                    renderRect.bottom - treeviewBorder);
                DrawFocusRect(hDC, &resizeBox);
                ReleaseDC(hWnd, hDC);
            }
            break;
        }
        case WM_MOUSEWHEEL:
        {
            scene.OnMouseWheel(HIWORD(wParam));
            break;
        }
        case WM_LBUTTONDOWN:
        {
            if (UltraEd::IsMouseOverSplitter(treeview, wParam, lParam))
            {
                SetCursor(hcSizeCursor);
                SetCapture(hWnd);
                resizingTreeView = true;
                break;
            }
            POINT point = { LOWORD(lParam), HIWORD(lParam) };
            scene.Pick(point);
            break;
        }
        case WM_LBUTTONUP:
        {
            if (resizingTreeView)
            {
                RECT parentRect;
                ReleaseCapture();
                GetClientRect(parentWindow, &parentRect);
                PostMessage(parentWindow, WM_SIZE, wParam, MAKELPARAM(parentRect.right, parentRect.bottom));
                resizingTreeView = false;
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
                if (scene.Pick(point))
                {
                    ClientToScreen(hWnd, &point);
                    HMENU menu = CreatePopupMenu();
                    AppendMenu(menu, MF_STRING, IDM_MENU_ADD_TEXTURE, _T("Add Texture"));
                    AppendMenu(menu, MF_STRING, IDM_MENU_MODIFY_SCRIPT_OBJECT, _T("Modify Script"));
                    AppendMenu(menu, MF_STRING, IDM_MENU_DELETE_OBJECT, _T("Delete"));
                    AppendMenu(menu, MF_STRING, IDM_MENU_DUPLICATE_OBJECT, _T("Duplicate"));
                    TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
                    DestroyMenu(menu);
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

                RECT treeviewRect;
                GetClientRect(treeview, &treeviewRect);
                MoveWindow(renderWindow, treeviewRect.right + treeviewBorder, 0,
                    LOWORD(lParam) - treeviewRect.right, HIWORD(lParam) - statusRect.bottom, 1);
                scene.Resize();
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

HWND CreateToolbar(HWND hWnd, HINSTANCE hInst)
{
    TBBUTTON tbrButtons[8];
    tbrButtons[0].idCommand = IDM_TOOLBAR_TRANSLATE;
    tbrButtons[0].fsState = TBSTATE_ENABLED;
    tbrButtons[0].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[0].iBitmap = 0;
    tbrButtons[0].iString = (INT_PTR)L"Move";

    tbrButtons[1].idCommand = IDM_TOOLBAR_ROTATE;
    tbrButtons[1].fsState = TBSTATE_ENABLED;
    tbrButtons[1].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[1].iBitmap = 1;
    tbrButtons[1].iString = (INT_PTR)L"Rotate";

    tbrButtons[2].idCommand = IDM_TOOLBAR_SCALE;
    tbrButtons[2].fsState = TBSTATE_ENABLED;
    tbrButtons[2].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[2].iBitmap = 2;
    tbrButtons[2].iString = (INT_PTR)L"Scale";

    tbrButtons[3].fsState = TBSTATE_ENABLED;
    tbrButtons[3].fsStyle = TBSTYLE_SEP;
    tbrButtons[3].iBitmap = 0;

    tbrButtons[4].idCommand = IDM_TOOLBAR_VIEW_PERSPECTIVE;
    tbrButtons[4].fsState = TBSTATE_ENABLED;
    tbrButtons[4].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[4].iBitmap = 3;
    tbrButtons[4].iString = (INT_PTR)L"Persp.";

    tbrButtons[5].idCommand = IDM_TOOLBAR_VIEW_TOP;
    tbrButtons[5].fsState = TBSTATE_ENABLED;
    tbrButtons[5].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[5].iBitmap = 4;
    tbrButtons[5].iString = (INT_PTR)L"Top";

    tbrButtons[6].idCommand = IDM_TOOLBAR_VIEW_LEFT;
    tbrButtons[6].fsState = TBSTATE_ENABLED;
    tbrButtons[6].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[6].iBitmap = 5;
    tbrButtons[6].iString = (INT_PTR)L"Left";

    tbrButtons[7].idCommand = IDM_TOOLBAR_VIEW_FRONT;
    tbrButtons[7].fsState = TBSTATE_ENABLED;
    tbrButtons[7].fsStyle = TBSTYLE_BUTTON;
    tbrButtons[7].iBitmap = 6;
    tbrButtons[7].iString = (INT_PTR)L"Front";

    return CreateToolbarEx(hWnd,
        WS_VISIBLE | WS_CHILD | WS_BORDER | TBSTYLE_TOOLTIPS,
        IDB_TOOLBAR,
        8,
        hInst,
        IDB_TOOLBAR,
        tbrButtons,
        8,
        16, 16, 16, 16,
        sizeof(TBBUTTON));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (LoadLibrary("SciLexer.dll") == NULL)
    {
        MessageBox(NULL, "Could not load SciLexer.dll", "Error", NULL);
        return 1;
    }

    hcSizeCursor = LoadCursor(NULL, IDC_SIZEWE);

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, "Call to RegisterClassEx failed!", "Error", NULL);
        return 1;
    }

    // Create the main window which we'll add the toolbar and renderer to.
    parentWindow = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowWidth / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowHeight / 2),
        windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

    if (!parentWindow)
    {
        MessageBox(NULL, "Could not create parent window.", "Error", NULL);
        return 1;
    }

    toolbarWindow = CreateToolbar(parentWindow, hInstance);

    if (!toolbarWindow)
    {
        MessageBox(NULL, "Could not create toolbar.", "Error", NULL);
        return 1;
    }

    statusBar = CreateStatusWindow(WS_VISIBLE | WS_CHILD, "Welcome to UltraEd", parentWindow, IDM_STATUS_BAR);

    if (!statusBar)
    {
        MessageBox(NULL, "Could not create status bar.", "Error", NULL);
        return 1;
    }

    ShowWindow(parentWindow, nCmdShow);

    // Create treeview that shows objects in scene.   
    RECT parentRect;
    GetClientRect(parentWindow, &parentRect);
    RECT toolbarRect;
    GetClientRect(toolbarWindow, &toolbarRect);
    RECT statusRect;
    GetClientRect(statusBar, &statusRect);
    treeview = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, TEXT("Tree View"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
        0, toolbarRect.bottom + treeviewBorder,
        treeviewWidth, parentRect.bottom - (toolbarRect.bottom + statusRect.bottom + treeviewBorder),
        parentWindow, (HMENU)IDM_TREEVIEW, hInstance, NULL);

    if (!treeview)
    {
        MessageBox(NULL, "Could not create treeview.", "Error", NULL);
        return 1;
    }

    // Create the window for rendering the scene.
    RECT treeviewRect;
    GetClientRect(treeview, &treeviewRect);
    renderWindow = CreateWindow(szWindowClass, szTitle, WS_CLIPSIBLINGS | WS_CHILD,
        treeviewRect.right + treeviewBorder, 0,
        parentRect.right - treeviewRect.right, parentRect.bottom - statusRect.bottom,
        parentWindow, NULL, hInstance, NULL);

    if (!renderWindow)
    {
        MessageBox(NULL, "Could not create render child window.", "Error", NULL);
        return 1;
    }

    ShowWindow(renderWindow, nCmdShow);

    if (!scene.Create(renderWindow))
    {
        MessageBox(NULL, "Could not create Direct3D device.", "Error", NULL);
        return 1;
    }

    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            scene.Render();
        }
    }

    return msg.wParam;
}
