#ifndef _MODEL_H_
#define _MODEL_H_

#include <filesystem>
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
        Model(const boost::uuids::uuid &assetId);
        Model(const char *filePath);
        Model(const Model &model);
        nlohmann::json Save();
        void Load(const nlohmann::json &root, IDirect3DDevice9 *device);
        const boost::uuids::uuid &GetTextureId() { return m_textureId; }
        const boost::uuids::uuid &GetModelId() { return m_modelId; }
        bool SetTexture(IDirect3DDevice9 *device, const boost::uuids::uuid &assetId);
        void SetMesh(const boost::uuids::uuid &assetId);
        bool HasTexture() { return m_texture != NULL; }
        void DeleteTexture();
        void Release(ModelRelease type);
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        std::array<int, 2> TextureDimensions();

    private:
        bool LoadTexture(IDirect3DDevice9 *device);
        bool IsTextureValid();
        LPDIRECT3DTEXTURE9 m_texture;
        boost::uuids::uuid m_textureId;
        boost::uuids::uuid m_modelId;
    };
}

#endif
