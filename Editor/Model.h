#pragma once

#include "Actor.h"

using namespace std;

namespace UltraEd
{
    struct ModelRelease
    {
        enum Value { AllResources, VertexBufferOnly };
    };

    class CModel : public CActor
    {
    public:
        CModel();
        CModel(const char *filePath);
        CModel(const CModel &model);
        Savable Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);
        bool LoadTexture(IDirect3DDevice8 *device, const char *filePath);
        void Release(ModelRelease::Value type);
        void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);

    private:
        LPDIRECT3DTEXTURE8 m_texture;
        float m_collisionRadius;
    };
}
