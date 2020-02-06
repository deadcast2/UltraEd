#include "Collider.h"
#include "FileIO.h"

namespace UltraEd
{
    CCollider::CCollider() : 
        m_type(ColliderType::Box),
        m_vertices(),
        m_center(0, 0, 0), 
        m_material(),
        m_vertexBuffer(make_shared<CVertexBuffer>())
    {
        m_material.Emissive.r = 0;
        m_material.Emissive.g = 1;
        m_material.Emissive.b = 0;
    }

    void CCollider::Render(IDirect3DDevice8 *device)
    {
        IDirect3DVertexBuffer8 *buffer = m_vertexBuffer->GetBuffer(device, m_vertices);

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
        }
    }

    ColliderType::Value CCollider::GetType(cJSON *item)
    {
        int typeValue;
        cJSON *type = cJSON_GetObjectItem(item, "type");
        sscanf(type->valuestring, "%i", &typeValue);
        return (ColliderType::Value)typeValue;
    }

    cJSON *CCollider::Save()
    {
        char buffer[LINE_FORMAT_LENGTH];
        cJSON *root = cJSON_CreateObject();

        sprintf(buffer, "%i", (int)m_type);
        cJSON_AddStringToObject(root, "type", buffer);

        sprintf(buffer, "%f %f %f", m_center.x, m_center.y, m_center.z);
        cJSON_AddStringToObject(root, "center", buffer);

        return root;
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
