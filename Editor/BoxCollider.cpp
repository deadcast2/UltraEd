#include "BoxCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    CBoxCollider::CBoxCollider() :
        m_extents(1, 1, 1)
    { 
        m_type = ColliderType::Box;
    }

    CBoxCollider::CBoxCollider(const vector<Vertex> &vertices) : CBoxCollider()
    {
        D3DXVECTOR3 min, max;
        DistantAABBPoints(min, max, vertices);

        FLOAT midX = (min.x + max.x) / 2;
        FLOAT midY = (min.y + max.y) / 2;
        FLOAT midZ = (min.z + max.z) / 2;

        m_center = D3DXVECTOR3(midX / 2, midY / 2, midZ / 2);
        m_extents = D3DXVECTOR3(min.x - midX, min.y - midY, min.z - midZ);

        Build();
    }

    void CBoxCollider::DistantAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const vector<Vertex> &vertices)
    {
        int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
        for (size_t i = 0; i < vertices.size(); i++)
        {
            if (vertices[i].position.x < vertices[minX].position.x) minX = i;
            if (vertices[i].position.x > vertices[maxX].position.x) maxX = i;
            if (vertices[i].position.y < vertices[minY].position.y) minY = i;
            if (vertices[i].position.y > vertices[maxY].position.y) maxY = i;
            if (vertices[i].position.z < vertices[minZ].position.z) minZ = i;
            if (vertices[i].position.z > vertices[maxZ].position.z) maxZ = i;
        }

        min = D3DXVECTOR3(vertices[minX].position.x, vertices[minY].position.y, vertices[minZ].position.z);
        max = D3DXVECTOR3(vertices[maxX].position.x, vertices[maxY].position.y, vertices[maxZ].position.z);
    }

    void CBoxCollider::Build()
    {
        // Top square
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z));

        // Bottom square
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z));

        // Left square
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x - m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z),
            D3DXVECTOR3(m_center.x - m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z));

        // Right square
        BuildLine(D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z + m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z + m_extents.z));
        BuildLine(D3DXVECTOR3(m_center.x + m_extents.x, m_center.y + m_extents.y, m_center.z - m_extents.z),
            D3DXVECTOR3(m_center.x + m_extents.x, m_center.y - m_extents.y, m_center.z - m_extents.z));
    }

    void CBoxCollider::BuildLine(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end)
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
        sprintf(buffer, "%f %f %f", m_extents.x, m_extents.y, m_extents.z);
        cJSON_AddStringToObject(savable.object, "extents", buffer);
        return savable;
    }

    bool CBoxCollider::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        CCollider::Load(device, root);
        cJSON *extents = cJSON_GetObjectItem(root, "extents");
        float x, y, z;
        sscanf(extents->valuestring, "%f %f %f", &x, &y, &z);
        m_extents = D3DXVECTOR3(x, y, z);
        Build();
        return true;
    }
}
