#ifndef _MODEL_H_
#define _MODEL_H_

#include <filesystem>
#include <rpc.h>
#include "Actor.h"

namespace UltraEd
{
    enum class ModelRelease
    {
        AllResources, VertexBufferOnly
    };

    class Model : public Actor
    {
    public:
        Model();
        Model(const GUID &assetId);
        Model(const char *filePath);
        Model(const Model &model);
        cJSON *Save();
        bool Load(cJSON *root, IDirect3DDevice9 *device);
        const GUID &GetTextureId() { return m_textureId; }
        bool SetTexture(IDirect3DDevice9 *device, const GUID &assetId);
        bool HasTexture() { return m_texture != NULL; }
        void DeleteTexture();
        void Release(ModelRelease type);
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        std::array<int, 2> TextureDimensions();

    private:
        bool LoadTexture(IDirect3DDevice9 *device);
        bool IsTextureValid();
        LPDIRECT3DTEXTURE9 m_texture;
        GUID m_textureId;
    };
}

#endif
