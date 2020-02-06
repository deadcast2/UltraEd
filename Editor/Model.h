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
        cJSON *Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);
        bool SetTexture(IDirect3DDevice8 *device, const char *filePath);
        bool HasTexture() { return m_texture != NULL; }
        void DeleteTexture();
        void Release(ModelRelease::Value type);
        void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);

    private:
        bool LoadTexture(IDirect3DDevice8 *device, const char *filePath);
        LPDIRECT3DTEXTURE8 m_texture;
    };
}
