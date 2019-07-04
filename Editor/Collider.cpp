#include "Collider.h"
#include "FileIO.h"

namespace UltraEd
{
    CCollider::CCollider()
    {
        m_vertexBuffer = 0;
        m_center = D3DXVECTOR3(0, 0, 0);

        ZeroMemory(&m_material, sizeof(D3DMATERIAL8));
        m_material.Emissive.r = 0;
        m_material.Emissive.g = 1;
        m_material.Emissive.b = 0;
    }

    IDirect3DVertexBuffer8 *CCollider::GetBuffer(IDirect3DDevice8 *device)
    {
        if (m_vertexBuffer == NULL)
        {
            if (FAILED(device->CreateVertexBuffer(
                m_vertices.size() * sizeof(Vertex),
                0,
                D3DFVF_XYZ,
                D3DPOOL_DEFAULT,
                &m_vertexBuffer)))
            {
                return NULL;
            }

            VOID* pVertices;
            if (FAILED(m_vertexBuffer->Lock(0, m_vertices.size() * sizeof(Vertex),
                (BYTE**)&pVertices, 0)))
            {
                return NULL;
            }

            memcpy(pVertices, &m_vertices[0], m_vertices.size() * sizeof(Vertex));
            m_vertexBuffer->Unlock();
        }

        return m_vertexBuffer;
    }

    void CCollider::Render(IDirect3DDevice8 *device)
    {
        IDirect3DVertexBuffer8 *buffer = GetBuffer(device);

        if (buffer != NULL)
        {
            device->SetMaterial(&m_material);
            device->SetStreamSource(0, buffer, sizeof(Vertex));
            device->SetVertexShader(D3DFVF_XYZ);
            device->DrawPrimitive(D3DPT_LINELIST, 0, m_vertices.size() / 2);
        }
    }

    void CCollider::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
            m_vertexBuffer = 0;
        }
    }

    ColliderType::Value CCollider::GetType(cJSON *item)
    {
        int typeValue;
        cJSON *type = cJSON_GetObjectItem(item, "type");
        sscanf(type->valuestring, "%i", &typeValue);
        return (ColliderType::Value)typeValue;
    }

    Savable CCollider::Save()
    {
        char buffer[LINE_FORMAT_LENGTH];
        cJSON *root = cJSON_CreateObject();

        sprintf(buffer, "%i", (int)m_type);
        cJSON_AddStringToObject(root, "type", buffer);

        sprintf(buffer, "%f %f %f", m_center.x, m_center.y, m_center.z);
        cJSON_AddStringToObject(root, "center", buffer);

        return { root, SavableType::Actor };
    }

    bool CCollider::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        m_type = GetType(root);

        float x, y, z;
        cJSON *center = cJSON_GetObjectItem(root, "center");
        sscanf(center->valuestring, "%f %f %f", &x, &y, &z);
        m_center = D3DXVECTOR3(x, y, z);

        return true;
    }
}
