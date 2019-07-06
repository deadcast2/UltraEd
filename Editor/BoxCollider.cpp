#include "BoxCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    CBoxCollider::CBoxCollider()
    {
        m_extents[0] = D3DXVECTOR3(1, 0, 0);
        m_extents[1] = D3DXVECTOR3(0, 1, 0);
        m_extents[2] = D3DXVECTOR3(0, 0, 1);
        m_type = ColliderType::Box;
    }

    CBoxCollider::CBoxCollider(vector<Vertex> &vertices) : CBoxCollider()
    {
        Build();
    }

    void CBoxCollider::Build()
    {
        // Top square
        BuildLine(-m_extents[0] + m_extents[1] + m_extents[2], m_extents[0] + m_extents[1] + m_extents[2]);
        BuildLine(-m_extents[0] + m_extents[1] - m_extents[2], m_extents[0] + m_extents[1] - m_extents[2]);
        BuildLine(-m_extents[0] + m_extents[1] - m_extents[2], -m_extents[0] + m_extents[1] + m_extents[2]);
        BuildLine(m_extents[0] + m_extents[1] - m_extents[2], m_extents[0] + m_extents[1] + m_extents[2]);

        // Bottom square
        BuildLine(-m_extents[0] - m_extents[1] + m_extents[2], m_extents[0] - m_extents[1] + m_extents[2]);
        BuildLine(-m_extents[0] - m_extents[1] - m_extents[2], m_extents[0] - m_extents[1] - m_extents[2]);
        BuildLine(-m_extents[0] - m_extents[1] - m_extents[2], -m_extents[0] - m_extents[1] + m_extents[2]);
        BuildLine(m_extents[0] - m_extents[1] - m_extents[2], m_extents[0] - m_extents[1] + m_extents[2]);

        // Left square
        BuildLine(-m_extents[0] - m_extents[1] + m_extents[2], -m_extents[0] + m_extents[1] + m_extents[2]);
        BuildLine(-m_extents[0] - m_extents[1] - m_extents[2], -m_extents[0] + m_extents[1] - m_extents[2]);

        // Right square
        BuildLine(m_extents[0] - m_extents[1] + m_extents[2], m_extents[0] + m_extents[1] + m_extents[2]);
        BuildLine(m_extents[0] - m_extents[1] - m_extents[2], m_extents[0] + m_extents[1] - m_extents[2]);
    }

    void CBoxCollider::BuildLine(D3DXVECTOR3 &start, D3DXVECTOR3 &end)
    {
        Vertex v1;
        v1.position = start + m_center;
        m_vertices.push_back(v1);

        Vertex v2;
        v2.position = end + m_center;
        m_vertices.push_back(v2);    
    }

    Savable CBoxCollider::Save()
    {
        Savable savable = CCollider::Save();
        char buffer[LINE_FORMAT_LENGTH];
        cJSON *array = cJSON_CreateArray();

        for (int i = 0; i < 3; i++)
        {
            sprintf(buffer, "%f %f %f", m_extents[i].x, m_extents[i].y, m_extents[i].z);
            cJSON_AddItemToArray(array, cJSON_CreateString(buffer));
        }

        cJSON_AddItemToObject(savable.object, "extents", array);
        return savable;
    }

    bool CBoxCollider::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        CCollider::Load(device, root);
        cJSON *array = cJSON_GetObjectItem(root, "extents");
        for (int i = 0; i < 3; i++)
        {
            cJSON *extent = cJSON_GetArrayItem(array, i);
            sscanf(extent->valuestring, "%f %f %f", &m_extents[i].x, &m_extents[i].y, &m_extents[i].z);
        }
        Build();
        return true;
    }
}
