#define WIN32_LEAN_AND_MEAN

// Enable mouse wheel scrolling.
#define _WIN32_WINDOWS 0x0501

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "resource.h"
#include "Scene.h"

#define IDM_TOOLBAR_TRANSLATE 5000
#define IDM_TOOLBAR_ROTATE 5001
#define IDM_TOOLBAR_SCALE 5002
#define IDM_MENU_DELETE_OBJECT 9001

const int windowWidth = 800;
const int windowHeight = 600;
const int mouseWaitPeriod = 500; // milliseconds
const TCHAR szWindowClass[] = _T("UltraEd");
const TCHAR szTitle[] = _T("UltraEd v0.1");

HWND parentWindow, toolbarWindow, renderWindow;
CScene scene;
DWORD mouseClickTick = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_KEYDOWN:
  {
    switch(LOWORD(wParam))
    {
    case VK_DELETE:
      scene.Delete();
      break;
    }
  }
  case WM_COMMAND:
    {
      switch(LOWORD(wParam))
      {
      case ID_FILE_IMPORTMODEL:
        scene.OnImportModel();
        break;
      case ID_TOOL_APPLYTEXTURE:
        scene.OnApplyTexture();
        break;
      case ID_MOVEMENT_WORLDSPACE:
        {
          HMENU menu = GetMenu(hWnd);
          if(menu != NULL)
          {
            bool toggled = scene.ToggleMovementSpace();
            CheckMenuItem(menu, wParam, toggled ? MF_CHECKED : MF_UNCHECKED);
          }
          break;
        }
      case IDM_TOOLBAR_TRANSLATE:
        scene.SetGizmoModifier(Translate);
        break;
      case IDM_TOOLBAR_ROTATE:
        scene.SetGizmoModifier(Rotate);
        break;
      case IDM_TOOLBAR_SCALE:
        scene.SetGizmoModifier(Scale);
        break;
      case IDM_MENU_DELETE_OBJECT:
        scene.Delete();
        break;
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
      POINT point = {LOWORD(lParam), HIWORD(lParam)};
      scene.Pick(point);
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
      if(GetTickCount() - mouseClickTick < mouseWaitPeriod)
      {
        POINT point = {LOWORD(lParam), HIWORD(lParam)};
        ClientToScreen(hWnd, &point);
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, IDM_MENU_DELETE_OBJECT, _T("Delete Object"));
        TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
        DestroyMenu(menu);
      }
      break;
    }
  case WM_SIZE:
    {
      if(wParam != SIZE_MINIMIZED)
      {
        // Resize the child windows and the scene.
        MoveWindow(toolbarWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);
        MoveWindow(renderWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), 1);
        scene.Resize(LOWORD(lParam), HIWORD(lParam));
      }
      break;
    }
  case WM_ERASEBKGND:
    {
      return 1;
      break;
    }
  case WM_DESTROY:
    {
      PostQuitMessage(0);
      break;
    }
  default:
    {
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }
  }
  
  return 0;
}

HWND CreateToolbar(HWND hWnd, HINSTANCE hInst)
{
  TBBUTTON tbrButtons[3];  
  tbrButtons[0].idCommand = IDM_TOOLBAR_TRANSLATE;
  tbrButtons[0].fsState   = TBSTATE_ENABLED;
  tbrButtons[0].fsStyle   = TBSTYLE_BUTTON;
  tbrButtons[0].dwData    = 0L;
  tbrButtons[0].iBitmap   = 0;
  tbrButtons[0].iString   = 0;
  
  tbrButtons[1].idCommand = IDM_TOOLBAR_ROTATE;
  tbrButtons[1].fsState   = TBSTATE_ENABLED;
  tbrButtons[1].fsStyle   = TBSTYLE_BUTTON;
  tbrButtons[1].dwData    = 0L;
  tbrButtons[1].iBitmap   = 1;
  tbrButtons[1].iString   = 0;
  
  tbrButtons[2].idCommand = IDM_TOOLBAR_SCALE;
  tbrButtons[2].fsState   = TBSTATE_ENABLED;
  tbrButtons[2].fsStyle   = TBSTYLE_BUTTON;
  tbrButtons[2].dwData    = 0L;
  tbrButtons[2].iBitmap   = 2;
  tbrButtons[2].iString   = 0;
  
  return CreateToolbarEx(hWnd,
    WS_VISIBLE | WS_CHILD | WS_BORDER,
    IDB_TOOLBAR,
    3,
    hInst,
    IDB_TOOLBAR,
    tbrButtons,
    3,
    16, 16, 16, 16,
    sizeof(TBBUTTON));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  
  if(!RegisterClassEx(&wcex))
  {
    MessageBox(NULL, "Call to RegisterClassEx failed!", "Error", NULL);
    return 1;
  }
  
  // Create the main window which we'll add the toolbar and renderer to.
  parentWindow = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowWidth / 2), 
    (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowHeight / 2), 
    windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
  
  if(!parentWindow)
  {
    MessageBox(NULL, "Could not create parent window.", "Error", NULL);
    return 1;
  }
  
  toolbarWindow = CreateToolbar(parentWindow, hInstance);
  if(!toolbarWindow)
  {
    MessageBox(NULL, "Could not create toolbar.", "Error", NULL);
    return 1;
  }
  
  ShowWindow(parentWindow, nCmdShow);
  UpdateWindow(parentWindow);
  
  // Create the window for rendering the scene.
  renderWindow = CreateWindow(szWindowClass, szTitle, WS_CLIPSIBLINGS | WS_CHILD,
    CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, parentWindow, NULL, hInstance, NULL);
  
  if(!renderWindow)
  {
    MessageBox(NULL, "Could not create render child window.", "Error", NULL);
    return 1;
  }
  
  ShowWindow(renderWindow, nCmdShow);
  UpdateWindow(renderWindow);
  
  if(!scene.Create(renderWindow))
  {
    return 1;
  }
  
  // Trigger the scene resize calculation.
  SendMessage(parentWindow, WM_SIZE, 0, MAKELPARAM(windowWidth, windowHeight));
  
  MSG msg;
  while(WM_QUIT != msg.message)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
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