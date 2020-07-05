#ifndef _SCENE_H_
#define _SCENE_H_

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <array>
#include <functional>
#include <memory>
#include "Grid.h"
#include "Gui.h"
#include "View.h"

using namespace std;

namespace UltraEd
{
    class Scene
    {
    public:
        Scene();
        bool Create(HWND hWnd);
        void Render();

    private:
        void Resize(int width, int height);
        void UpdateViewMatrix(int width, int height);
        HWND GetWndHandle();
        View *GetActiveView();
        void OnNew(bool confirm = true);
        void ResetViews();
        void ReleaseResources();

    private:
        unique_ptr<IDirect3D9, function<void(IDirect3D9 *)>> m_d3d9;
        unique_ptr<IDirect3DDevice9, function<void(IDirect3DDevice9 *)>> m_device;
        unique_ptr<Gui> m_gui;
        D3DPRESENT_PARAMETERS m_d3dpp;
        ViewType m_activeViewType;
        View m_views[4];
        array<int, 3> m_backgroundColorRGB;
        D3DLIGHT9 m_worldLight;
        Grid m_grid;
    };
}

#endif
