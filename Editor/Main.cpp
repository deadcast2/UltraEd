#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <tuple>
#include "resource.h"
#include "Scene.h"

using namespace UltraEd;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::unique_ptr<Gui> gui;

int main(int, char **)
{
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL),
        LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN_ICON)),
        NULL, NULL, NULL, APP_CLASS, NULL
    };

    RegisterClassEx(&wc);

    const int windowWidth = 1024;
    const int windowHeight = 768;
    HWND hWnd = CreateWindow(wc.lpszClassName, APP_NAME, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowWidth / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowHeight / 2),
        windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    MSG msg {};
    gui = std::make_unique<Gui>(hWnd);

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        gui->Render();
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
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_ACTIVE && gui)
            {
                std::vector<boost::uuids::uuid> changedAssetIds;
                Project::Activate(&changedAssetIds);
                gui->RefreshScene(changedAssetIds);
            }
            return 0;
        case WM_SIZE:
        {
            if (gui) gui->Resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        case WM_SYSCOMMAND:
        {
            // Disable ALT application menu
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        }
        case WM_CLOSE:
            if (gui) gui->ConfirmScene([]() { PostQuitMessage(0); });
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
