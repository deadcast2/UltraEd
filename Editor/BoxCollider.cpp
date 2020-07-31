#include "BoxCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    BoxCollider::BoxCollider() :
        m_extents(1, 1, 1)
    { 
        m_type = ColliderType::Box;
    }

    BoxCollider::BoxCollider(const std::vector<Vertex> &vertices) : BoxCollider()
    {
        D3DXVECTOR3 min, max;
        DistantAABBPoints(min, max, vertices);

        FLOAT midX = (min.x + max.x) / 2;
        FLOAT midY = (min.y + max.y) / 2;
        FLOAT midZ = (min.z + max.z) / 2;

        m_center = D3DXVECTOR3(midX / 2, midY / 2, midZ / 2);
        m_extents = D3DXVECTOR3(midX - min.x, midY - min.y, midZ - min.z);

        Build();
    }

    void BoxCollider::Build()
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

    void BoxCollider::BuildLine(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end)
    {
        Vertex v1;
        v1.position = start + m_center;
        m_vertices.push_back(v1);

        Vertex v2;
        v2.position = end + m_center;
        m_vertices.push_back(v2);    
    }

    cJSON *BoxCollider::Save()
    {
        auto state = Collider::Save();
        char buffer[LINE_FORMAT_LENGTH];
        sprintf(buffer, "%f %f %f", m_extents.x, m_extents.y, m_extents.z);
        cJSON_AddStringToObject(state, "extents", buffer);
        return state;
    }

    bool BoxCollider::Load(cJSON *root)
    {
        Collider::Load(root);
        cJSON *extents = cJSON_GetObjectItem(root, "extents");
        float x, y, z;
        sscanf(extents->valuestring, "%f %f %f", &x, &y, &z);
        m_extents = D3DXVECTOR3(x, y, z);
        Build();
        return true;
    }
}
