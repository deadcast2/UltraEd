#include "BoxCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    BoxCollider::BoxCollider() :
        m_extents(),
        m_originalExtents()
    {
        m_type = ColliderType::Box;
    }

    BoxCollider::BoxCollider(const std::vector<Vertex> &vertices) : BoxCollider()
    {
        D3DXVECTOR3 min, max;
        MinMaxAABBPoints(min, max, vertices);

        m_originalCenter = m_center = D3DXVECTOR3(
            (min.x + max.x) / 2, 
            (min.y + max.y) / 2, 
            (min.z + max.z) / 2);

        m_originalExtents = m_extents = D3DXVECTOR3(
            (fabs(min.x) + fabs(max.x)) / 2, 
            (fabs(min.y) + fabs(max.y)) / 2, 
            (fabs(min.z) + fabs(max.z)) / 2);

        Build();
    }

    void BoxCollider::Build()
    {
        m_vertices.clear();

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
        v1.position = start;
        m_vertices.push_back(v1);

        Vertex v2;
        v2.position = end;
        m_vertices.push_back(v2);
    }

    /// <summary>
    /// Adapted code from Real Time Collision Detection - Section 4.2.6
    /// </summary>
    /// <param name="mat"></param>
    void BoxCollider::Update(D3DXMATRIX &mat)
    {
        float center[3] = { m_center.x, m_center.y, m_center.z };
        const float originalCenter[3] = { m_originalCenter.x, m_originalCenter.y, m_originalCenter.z };

        float extents[3] = { m_extents.x, m_extents.y, m_extents.z };
        const float originalExtents[3] = { m_originalExtents.x, m_originalExtents.y, m_originalExtents.z };
        
        for (int i = 0; i < 3; i++)
        {
            center[i] = 0.0f;
            extents[i] = 0.0f;

            for (int j = 0; j < 3; j++)
            {
                center[i] += mat.m[j][i] * originalCenter[j];
                extents[i] += fabs(mat.m[j][i]) * originalExtents[j];
            }
        }

        m_center = D3DXVECTOR3(center);
        m_extents = D3DXVECTOR3(extents);

        Build();
        Release();
    }

    nlohmann::json BoxCollider::Save()
    {
        auto collider = Collider::Save();
        collider.update({
            { "extents", m_extents },
            { "originalExtents", m_originalExtents },
            { "originalCenter", m_originalCenter }
        });
        return collider;
    }

    void BoxCollider::Load(const nlohmann::json &root)
    {
        Collider::Load(root);

        m_extents = root["extents"];
        m_originalExtents = root["originalExtents"];
        m_originalCenter = root["originalCenter"];

        Build();
    }
}
