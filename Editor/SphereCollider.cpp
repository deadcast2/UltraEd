#include "SphereCollider.h"
#include "FileIO.h"

namespace UltraEd
{
    CSphereCollider::CSphereCollider()
    {
        m_radius = 1;
        m_type = ColliderType::Sphere;
    }

    CSphereCollider::CSphereCollider(vector<Vertex> &vertices) : CSphereCollider()
    {
        // Compute optimal center and radius (sphere)
        FindCenterWithRadius(m_center, m_radius, vertices);
        Build();
    }

    void CSphereCollider::Build()
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

    void CSphereCollider::DistantAABBPoints(int &min, int &max, vector<Vertex> vertices)
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

        FLOAT distX = D3DXVec3Dot(&(vertices[maxX].position - vertices[minX].position),
            &(vertices[maxX].position - vertices[minX].position));
        FLOAT distY = D3DXVec3Dot(&(vertices[maxY].position - vertices[minY].position),
            &(vertices[maxY].position - vertices[minY].position));
        FLOAT distZ = D3DXVec3Dot(&(vertices[maxZ].position - vertices[minZ].position),
            &(vertices[maxZ].position - vertices[minZ].position));

        min = minX;
        max = maxX;

        if (distY > distX && distY > distZ)
        {
            max = maxY;
            min = minY;
        }

        if (distZ > distX && distZ > distY)
        {
            max = maxZ;
            min = minZ;
        }
    }

    void CSphereCollider::SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices)
    {
        int min, max;
        DistantAABBPoints(min, max, vertices);
        center = (vertices[min].position + vertices[max].position) * 0.5f;
        radius = D3DXVec3Dot(&(vertices[max].position - center), &(vertices[max].position - center));
        radius = sqrtf(radius);
    }

    void CSphereCollider::AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, Vertex vertex)
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

    void CSphereCollider::FindCenterWithRadius(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices)
    {
        SphereFromDistPoints(center, radius, vertices);
        for (size_t i = 0; i < vertices.size(); i++)
        {
            AdjustSphere(center, radius, vertices[i]);
        }
    }

    Savable CSphereCollider::Save()
    {
        Savable savable = CCollider::Save();
        char buffer[LINE_FORMAT_LENGTH];
        sprintf(buffer, "%f", m_radius);
        cJSON_AddStringToObject(savable.object, "radius", buffer);
        return savable;
    }

    bool CSphereCollider::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        CCollider::Load(device, root);
        cJSON *radius = cJSON_GetObjectItem(root, "radius");
        sscanf(radius->valuestring, "%f", &m_radius);
        Build();
        return true;
    }
}
