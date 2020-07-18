#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <tuple>
#include "resource.h"
#include "PubSub.h"
#include "Scene.h"

using namespace UltraEd;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int, char **)
{
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL),
        LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN_ICON)),
        NULL, NULL, NULL, APP_CLASS, NULL
    };

    RegisterClassEx(&wc);

    const int windowWidth = 800;
    const int windowHeight = 600;
    HWND hWnd = CreateWindow(wc.lpszClassName, APP_NAME, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowWidth / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowHeight / 2),
        windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);

    Scene scene;
    if (!scene.Create(hWnd))
    {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    PubSub::Subscribe({ "Exit", [&](void *data) {
        if (scene.Confirm())
        {
            PostQuitMessage(0);
        }
    } });

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        scene.Render();
    }

    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
        {
            auto rect = make_tuple<int, int>(LOWORD(lParam), HIWORD(lParam));
            PubSub::Publish("Resize", static_cast<void *>(&rect));
            return 0;
        }
        case WM_SYSCOMMAND:
        {
            // Disable ALT application menu
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        }
        case WM_CLOSE:
            PubSub::Publish("Exit");
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
