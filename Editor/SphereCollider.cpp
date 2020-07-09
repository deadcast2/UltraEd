#include "SphereCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    SphereCollider::SphereCollider() :
        m_radius(1)
    {
        m_type = ColliderType::Sphere;
    }

    SphereCollider::SphereCollider(const vector<Vertex> &vertices) : SphereCollider()
    {
        // Compute optimal center and radius (sphere)
        FindCenterWithRadius(m_center, m_radius, vertices);
        Build();
    }

    void SphereCollider::Build()
    {
        const int segments = 32;
        const float sample = (2 * D3DX_PI) / segments;

        // Horizontal
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(sin(sample * i) * m_radius, 0, cos(sample * i) * m_radius) + m_center;
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(sin(sample * (i + 1)) * m_radius, 0, cos(sample * (i + 1)) * m_radius) + m_center;
            m_vertices.push_back(v2);
        }

        // Vertical
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(0, sin(sample * i) * m_radius, cos(sample * i) * m_radius) + m_center;
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(0, sin(sample * (i + 1)) * m_radius, cos(sample * (i + 1)) * m_radius) + m_center;
            m_vertices.push_back(v2);
        }
    }

    void SphereCollider::SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, const vector<Vertex> &vertices)
    {
        D3DXVECTOR3 min, max;
        DistantAABBPoints(min, max, vertices);
        center = (min + max) * 0.5f;
        radius = sqrtf(D3DXVec3Dot(&(max - center), &(max - center)));
    }

    void SphereCollider::AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, const Vertex &vertex)
    {
        D3DXVECTOR3 dist = vertex.position - center;
        FLOAT dist2 = D3DXVec3Dot(&dist, &dist);
        if (dist2 > radius * radius)
        {
            FLOAT dn = sqrtf(dist2);
            FLOAT newRadius = (radius + dn) * 0.5f;
            FLOAT k = (newRadius - radius) / dn;
            radius = newRadius;
            center += dist * k;
        }
    }

    void SphereCollider::FindCenterWithRadius(D3DXVECTOR3 &center, FLOAT &radius, const vector<Vertex> &vertices)
    {
        SphereFromDistPoints(center, radius, vertices);
        for (size_t i = 0; i < vertices.size(); i++)
        {
            AdjustSphere(center, radius, vertices[i]);
        }
    }

    cJSON *SphereCollider::Save()
    {
        auto state = Collider::Save();
        char buffer[LINE_FORMAT_LENGTH];
        sprintf(buffer, "%f", m_radius);
        cJSON_AddStringToObject(state, "radius", buffer);
        return state;
    }

    bool SphereCollider::Load(cJSON *root)
    {
        Collider::Load(root);
        cJSON *radius = cJSON_GetObjectItem(root, "radius");
        sscanf(radius->valuestring, "%f", &m_radius);
        Build();
        return true;
    }
}
