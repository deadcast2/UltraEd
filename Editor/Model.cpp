#include "Model.h"
#include "FileIO.h"

namespace UltraEd
{
    CModel::CModel()
    {
        m_texture = 0;
        ResetId();
    }

    CModel::CModel(const CModel &model)
    {
        *this = model;
        m_vertexBuffer = 0;
        m_texture = 0;
        ResetId();
    }

    CModel::CModel(const char *filePath)
    {
        Import(filePath);
        m_texture = 0;
        m_type = ActorType::Model;
    }

    void CModel::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack)
    {
        IDirect3DVertexBuffer8 *buffer = GetBuffer(device);

        if (buffer != NULL)
        {
            stack->Push();
            stack->MultMatrixLocal(&GetMatrix());

            if (m_texture != NULL) device->SetTexture(0, m_texture);

            device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            device->SetTransform(D3DTS_WORLD, stack->GetTop());
            device->SetStreamSource(0, buffer, sizeof(Vertex));
            device->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, GetVertices().size() / 3);
            device->SetTexture(0, NULL);

            stack->Pop();
        }

        CActor::Render(device, stack);
    }

    void CModel::Release(ModelRelease::Value type)
    {
        CActor::Release();

        if (type == ModelRelease::VertexBufferOnly) return;

        if (m_texture != NULL)
        {
            m_texture->Release();
            m_texture = 0;
        }
    }

    bool CModel::LoadTexture(IDirect3DDevice8 *device, const char *filePath)
    {
        FileInfo info = CFileIO::Import(filePath);

        if (FAILED(D3DXCreateTextureFromFile(device, info.path.c_str(), &m_texture)))
        {
            return false;
        }

        // Save location of texture for scene saving.
        if (info.type == FileType::User) resources["textureDataPath"] = info.path;

        return true;
    }

    Savable CModel::Save()
    {
        return CActor::Save();
    }

    bool CModel::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        CActor::Load(device, root);

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
