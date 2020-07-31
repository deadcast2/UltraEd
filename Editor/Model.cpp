#include "Model.h"
#include "FileIO.h"

namespace UltraEd
{
    Model::Model() : m_texture(0) { }

    Model::Model(const Model &model) : Model()
    {
        *this = model;
        ResetId();
    }

    Model::Model(const char *filePath) : Model()
    {
        Import(filePath);
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
            { 32, 32 },  { 32, 64 }, { 64, 32 }
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

    bool Model::LoadTexture(IDirect3DDevice9 *device, const char *filePath)
    {
        FileInfo info = FileIO::Import(filePath);

        if (FAILED(D3DXCreateTextureFromFile(device, info.path.c_str(), &m_texture)))
        {
            return false;
        }

        if (!IsTextureValid())
        {
            DeleteTexture();
            return false;
        }

        // Save location of texture for scene saving.
        if (info.type == FileType::User)
        {
            AddResource("textureDataPath", info.path);
        }

        return true;
    }

    bool Model::SetTexture(IDirect3DDevice9 *device, const char *filePath)
    {
        bool result = false;
        Dirty([&] { result = LoadTexture(device, filePath); }, &result);
        return result;
    }

    void Model::DeleteTexture()
    {
        if (HasTexture())
        {
            m_texture->Release();
            m_texture = 0;
            DeleteResource("textureDataPath");
        }
    }

    cJSON *Model::Save()
    {
        return Actor::Save();
    }

    bool Model::Load(cJSON *root, IDirect3DDevice9 *device)
    {
        Actor::Load(root);

        DeleteTexture();

        cJSON *resource = NULL;
        cJSON *resources = cJSON_GetObjectItem(root, "resources");
        cJSON_ArrayForEach(resource, resources)
        {
            const char *path = resource->child->valuestring;
            if (strcmp(resource->child->string, "textureDataPath") == 0)
            {
                LoadTexture(device, path);
            }
        }

        return true;
    }
}
