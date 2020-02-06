#pragma once

#include "Actor.h"

namespace UltraEd
{
    class CCamera : public CActor
    {
    public:
        CCamera();
        CCamera(const CCamera &camera);
        void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);
        cJSON *Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);

    private:
        float m_fov;
    };
}
