#ifndef _MODEL_PREVIEWER_H_
#define _MODEL_PREVIEWER_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <filesystem>
#include "Model.h"

namespace UltraEd
{
    class ModelPreviewer
    {
    public:
        ModelPreviewer();
        ~ModelPreviewer();
        void Render(LPDIRECT3DDEVICE9 device, const std::filesystem::path &path, LPDIRECT3DTEXTURE9 *texture);

    private:
        bool SetupWindow();
        bool SetupRenderer();
        void CenterModel(Model &model);

    private:
        IDirect3DDevice9 *m_device;
        IDirect3D9 *m_d3d9;
        D3DPRESENT_PARAMETERS m_d3dpp;
        WNDCLASSEX m_wc;
        HWND m_hWnd;
        D3DLIGHT9 m_worldLight;
    };
}

#endif
