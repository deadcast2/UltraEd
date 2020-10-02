#include "Model.h"
#include "FileIO.h"

namespace UltraEd
{
    Model::Model() :
        m_texture(0),
        m_textureId(),
        m_modelId()
    { }

    Model::Model(const Model &model) : Model()
    {
        *this = model;
        ResetId();
    }

    Model::Model(const char *filePath) : Model()
    {
        Import(filePath);
    }

    Model::Model(const boost::uuids::uuid &assetId) : Model()
    {
        SetMesh(assetId);
    }

    void Model::Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack)
    {
        auto *buffer = m_vertexBuffer->GetBuffer(device, GetVertices());

        if (buffer != NULL)
        {
            stack->Push();
            stack->MultMatrixLocal(&GetMatrix());

            if (m_texture != NULL) device->SetTexture(0, m_texture);

            device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            device->SetTransform(D3DTS_WORLD, stack->GetTop());
            device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
            device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, static_cast<UINT>(GetVertices().size() / 3));
            device->SetTexture(0, NULL);

            stack->Pop();
        }

        Actor::Render(device, stack);
    }

    std::array<int, 2> Model::TextureDimensions()
    {
        D3DSURFACE_DESC desc;
        if (m_texture != NULL && SUCCEEDED(m_texture->GetLevelDesc(0, &desc)))
        {
            int width = static_cast<int>(desc.Width);
            int height = static_cast<int>(desc.Height);
            return { width, height };
        }
        return { 0, 0 };
    }

    bool Model::IsTextureValid()
    {
        auto dimensions = TextureDimensions();

        // Valid sizes for the RDP.
        std::vector<std::tuple<int, int>> validSizes = {
            { 8, 8 }, { 8, 16 }, { 16, 8 },
            { 16, 16 }, { 16, 32 }, { 32, 16 },
            { 32, 32 }, { 32, 64 }, { 64, 32 }
        };

        auto size = std::find_if(validSizes.begin(), validSizes.end(), [&](const auto &t) {
            return std::get<0>(t) == dimensions[0] && std::get<1>(t) == dimensions[1];
        });

        if (size != validSizes.end())
            return true;

        return false;
    }

    void Model::Release(ModelRelease type)
    {
        Actor::Release();

        if (type == ModelRelease::VertexBufferOnly)
            return;

        DeleteTexture();
    }

    void Model::SetMesh(const boost::uuids::uuid &assetId)
    {
        const auto modelPath = Project::GetAssetPath(assetId);

        if (!modelPath.empty())
        {
            m_modelId = assetId;
            Import(modelPath.string().c_str());
            Release(ModelRelease::VertexBufferOnly);
        }
    }

    bool Model::LoadTexture(IDirect3DDevice9 *device, const boost::uuids::uuid &assetId)
    {
        DeleteTexture();
        
        auto assetPath = Project::GetAssetPath(assetId);
        if (!assetPath.empty())
        {
            m_textureId = assetId;
            return SUCCEEDED(D3DXCreateTextureFromFile(device, assetPath.string().c_str(), &m_texture));
        }
        
        return false;
    }

    bool Model::SetTexture(IDirect3DDevice9 *device, const boost::uuids::uuid &assetId)
    {
        bool result = false;
        Dirty([&] { result = LoadTexture(device, assetId); }, &result);
        return result;
    }

    void Model::DeleteTexture()
    {
        if (HasTexture())
        {
            m_texture->Release();
            m_texture = 0;
            m_textureId = boost::uuids::nil_uuid();
        }
    }

    nlohmann::json Model::Save()
    {
        auto actor = Actor::Save();
        actor.update({
            { "texture_id", m_textureId },
            { "model_id", m_modelId }
        });
        return actor;
    }

    void Model::Load(const nlohmann::json &root, IDirect3DDevice9 *device)
    {
        Actor::Load(root);

        SetMesh(root["model_id"]);
        LoadTexture(device, root["texture_id"]);
    }
}
