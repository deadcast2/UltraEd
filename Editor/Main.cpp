#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINDOWS 0x0501 // Enable mouse wheel scrolling.

#pragma comment(lib, "deps/DXSDK/lib/d3dx8.lib")
#pragma comment(lib, "deps/DXSDK/lib/d3d8.lib")
#pragma comment(lib, "deps/Assimp/lib/assimp-vc140-mt.lib")

#include <tchar.h>
#include "handlers/Window.h"

const int windowWidth = 800;
const int windowHeight = 600;
const TCHAR szWindowClass[] = APP_NAME;
const TCHAR szTitle[] = _T("Loading");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (LoadLibrary("SciLexer.dll") == NULL)
    {
        MessageBox(NULL, "Could not load SciLexer.dll", "Error", NULL);
        return 1;
    }

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = UltraEd::WindowHandler;
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
    UltraEd::parentWindow = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowWidth / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowHeight / 2),
        windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

    if (!UltraEd::parentWindow)
    {
        MessageBox(NULL, "Could not create parent window.", "Error", NULL);
        return 1;
    }

    UltraEd::toolbarWindow = UltraEd::CreateToolbar(UltraEd::parentWindow, hInstance);

    if (!UltraEd::toolbarWindow)
    {
        MessageBox(NULL, "Could not create toolbar.", "Error", NULL);
        return 1;
    }

    UltraEd::statusBar = CreateStatusWindow(WS_VISIBLE | WS_CHILD, "Welcome to UltraEd",
        UltraEd::parentWindow, IDM_STATUS_BAR);

    if (!UltraEd::statusBar)
    {
        MessageBox(NULL, "Could not create status bar.", "Error", NULL);
        return 1;
    }

    ShowWindow(UltraEd::parentWindow, nCmdShow);

    // Create treeview that shows objects in scene.   
    RECT parentRect;
    GetClientRect(UltraEd::parentWindow, &parentRect);
    RECT toolbarRect;
    GetClientRect(UltraEd::toolbarWindow, &toolbarRect);
    RECT statusRect;
    GetClientRect(UltraEd::statusBar, &statusRect);
    UltraEd::treeview = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, TEXT("Tree View"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
        0, toolbarRect.bottom + UltraEd::treeviewBorder,
        UltraEd::treeviewWidth, parentRect.bottom - (toolbarRect.bottom + statusRect.bottom + UltraEd::treeviewBorder),
        UltraEd::parentWindow, (HMENU)IDM_TREEVIEW, hInstance, NULL);

    if (!UltraEd::treeview)
    {
        MessageBox(NULL, "Could not create treeview.", "Error", NULL);
        return 1;
    }

    // Create the tabbed window.
    RECT treeviewRect;
    GetClientRect(UltraEd::treeview, &treeviewRect);
    UltraEd::tabsWindow = CreateWindow(WC_TABCONTROL, "Tabs", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        treeviewRect.right, parentRect.bottom - statusRect.bottom - UltraEd::tabsWindowHeight,
        parentRect.right - treeviewRect.right + UltraEd::tabsBorder, parentRect.bottom,
        UltraEd::parentWindow, NULL, hInstance, NULL);

    if (UltraEd::tabsWindow == NULL)
    {
        MessageBox(NULL, "Could not create tabs window.", "Error", NULL);
        return 1;
    }

    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = "Build Output";
    if (TabCtrl_InsertItem(UltraEd::tabsWindow, 0, &tie) == -1)
    {
        MessageBox(NULL, "Could not add tab to tabs windows.", "Error", NULL);
        return 1;
    }

    RECT tabbedWindowRect;
    GetClientRect(UltraEd::tabsWindow, &tabbedWindowRect);
    UltraEd::buildOutput = CreateWindowEx(0, "EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        8, 28, tabbedWindowRect.right - 14, UltraEd::tabsWindowHeight - statusRect.bottom - 8,
        UltraEd::tabsWindow, NULL, hInstance, NULL);

    SendMessage(UltraEd::buildOutput, EM_SETREADONLY, TRUE, NULL);

    // Create the window for rendering the scene.
    UltraEd::renderWindow = CreateWindow(szWindowClass, szTitle, WS_CLIPSIBLINGS | WS_CHILD,
        treeviewRect.right + UltraEd::treeviewBorder, toolbarRect.bottom + UltraEd::treeviewBorder,
        parentRect.right - treeviewRect.right, parentRect.bottom - statusRect.bottom - UltraEd::tabsWindowHeight - 
        UltraEd::tabsBorder - toolbarRect.bottom,
        UltraEd::parentWindow, NULL, hInstance, NULL);

    if (!UltraEd::renderWindow)
    {
        MessageBox(NULL, "Could not create render child window.", "Error", NULL);
        return 1;
    }

    ShowWindow(UltraEd::renderWindow, nCmdShow);

    UltraEd::CScene scene;
    if (!scene.Create(UltraEd::renderWindow))
    {
        MessageBox(NULL, "Could not create Direct3D device.", "Error", NULL);
        return 1;
    }

    // Pass scene pointers to handles that need it during event handling.
    SetWindowLongPtr(UltraEd::renderWindow, GWLP_USERDATA, (LPARAM)& scene);
    SetWindowLongPtr(UltraEd::parentWindow, GWLP_USERDATA, (LPARAM)& scene);

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
