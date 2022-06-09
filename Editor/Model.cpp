#include "Model.h"
#include "FileIO.h"

namespace UltraEd
{
    Model::Model() :
        m_texture(std::make_shared<Texture>()),
        m_modelId()
    { }

    Model::Model(const Model &model) : Model()
    {
        *this = model;
        m_texture = std::make_shared<Texture>();
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
            {
                stack->MultMatrixLocal(&GetMatrix());
                
                if (m_texture->IsLoaded()) 
                {
                    device->SetTexture(0, m_texture->Get());
                    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                }
                
                device->SetTransform(D3DTS_WORLD, stack->GetTop());
                device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
                device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
                device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, static_cast<UINT>(GetVertices().size() / 3));
                device->SetTexture(0, NULL);
            }
            stack->Pop();
        }

        Actor::Render(device, stack);
    }

    void Model::SetMesh(const boost::uuids::uuid &assetId)
    {
        const auto modelPath = Project::GetAssetPath(assetId);

        if (!modelPath.empty())
        {
            m_modelId = assetId;
            Import(modelPath.string().c_str());
            Release();
        }
    }

    bool Model::SetTexture(IDirect3DDevice9 *device, const boost::uuids::uuid &assetId)
    {
        bool result = false;
        Dirty([&] { result = m_texture->Load(device, assetId); }, &result);
        return result;
    }

    nlohmann::json Model::Save()
    {
        auto actor = Actor::Save();
        actor.update({
            { "texture_id", m_texture->GetId() },
            { "model_id", m_modelId }
        });
        return actor;
    }

    void Model::Load(const nlohmann::json &root, IDirect3DDevice9 *device)
    {
        Actor::Load(root);

        SetMesh(root["model_id"]);
        m_texture->Load(device, root["texture_id"]);
    }
}
