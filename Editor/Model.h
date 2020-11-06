#ifndef _MODEL_H_
#define _MODEL_H_

#include <filesystem>
#include "Actor.h"
#include "Texture.h"

namespace UltraEd
{
    class Model : public Actor
    {
    public:
        Model();
        Model(const boost::uuids::uuid &assetId);
        Model(const char *filePath);
        Model(const Model &model);
        nlohmann::json Save();
        void Load(const nlohmann::json &root, IDirect3DDevice9 *device);
        Texture *GetTexture() { return m_texture.get(); }
        const boost::uuids::uuid &GetModelId() { return m_modelId; }
        bool SetTexture(IDirect3DDevice9 *device, const boost::uuids::uuid &assetId);
        void SetMesh(const boost::uuids::uuid &assetId);
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);

    private:
        std::shared_ptr<Texture> m_texture;
        boost::uuids::uuid m_modelId;
    };
}

#endif
