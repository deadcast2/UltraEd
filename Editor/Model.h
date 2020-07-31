#ifndef _MODEL_H_
#define _MODEL_H_

#include "Actor.h"

namespace UltraEd
{
    enum class ModelPreset
    {
        Custom, Pumpkin
    };

    enum class ModelRelease
    {
        AllResources, VertexBufferOnly
    };

    class Model : public Actor
    {
    public:
        Model();
        Model(const char *filePath);
        Model(const Model &model);
        cJSON *Save();
        bool Load(cJSON *root, IDirect3DDevice9 *device);
        bool SetTexture(IDirect3DDevice9 *device, const char *filePath);
        bool HasTexture() { return m_texture != NULL; }
        void DeleteTexture();
        void Release(ModelRelease type);
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);

    private:
        bool LoadTexture(IDirect3DDevice9 *device, const char *filePath);
        LPDIRECT3DTEXTURE9 m_texture;
    };
}

#endif
