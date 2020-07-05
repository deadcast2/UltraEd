#ifndef _GUI_H_
#define _GUI_H_

#include <windows.h>
#include <functional>
#include <memory>
#include "vendor/ImGui/imgui.h"
#include "vendor/ImGui/imgui_impl_dx9.h"
#include "vendor/ImGui/imgui_impl_win32.h"

using namespace std;

namespace UltraEd
{
    class Gui
    {
    public:
        Gui(HWND hWnd, IDirect3DDevice9 *device);
        ~Gui();
        void PrepareFrame();
        void RenderFrame();
        bool WantsMouse();
        void RebuildWith(function<void()> inner);
    };
}

#endif
