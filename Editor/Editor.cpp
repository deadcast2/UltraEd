// Enable mouse wheel scrolling.
#define _WIN32_WINDOWS 0x0501

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "resource.h"
#include "Scene.h"

static TCHAR szWindowClass[] = _T("UltraEd");
static TCHAR szTitle[] = _T("UltraEd v0.1");
CScene scene;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
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
  case WM_SIZE:
    {
      if(wParam != SIZE_MINIMIZED) scene.Resize();
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
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  
  if(!RegisterClassEx(&wcex))
  {
    MessageBox(NULL, _T("Call to RegisterClassEx failed!"),
      _T("Error"), NULL);
    
    return 1;
  }
  
  HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
  
  if(!hWnd)
  {
    MessageBox(NULL, _T("Call to CreateWindow failed!"),
      _T("Error"), NULL);
    
    return 1;
  }
  
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  
  if(!scene.Create(hWnd))
  {
    return 1;
  }
  
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