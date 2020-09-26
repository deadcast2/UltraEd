#ifndef _MODEL_PREVIEWER_H_
#define _MODEL_PREVIEWER_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <filesystem>
#include "Model.h"
#include "RenderDevice.h"

namespace UltraEd
{
    class ModelPreviewer
    {
    public:
        ModelPreviewer();
        void Render(LPDIRECT3DDEVICE9 deviceTarget, const std::filesystem::path &path, LPDIRECT3DTEXTURE9 *texture);

    public:
        static const int PreviewWidth = 64;

    private:
        void CenterModel(Model &model);

    private:
        RenderDevice m_renderDevice;
        D3DLIGHT9 m_worldLight;
    };
}

#endif
