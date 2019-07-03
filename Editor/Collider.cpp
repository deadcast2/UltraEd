#include "Collider.h"

namespace UltraEd
{
    CCollider::CCollider(vector<Vertex> &vertices)
    {
        m_vertexBuffer = 0;
        m_center = D3DXVECTOR3(0, 0, 0);
        m_radius = 1;
        m_type = ColliderType::Sphere;

        ZeroMemory(&m_material, sizeof(D3DMATERIAL8));
        m_material.Emissive.r = 0;
        m_material.Emissive.g = 1;
        m_material.Emissive.b = 0;

        Compute(vertices);
    }

    void CCollider::Compute(vector<Vertex> &vertices)
    {
        // Compute optimal center and radius (sphere)
        FindCenterWithRadius(m_center, m_radius, vertices);

        const int segments = 16;
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

    void CCollider::DistantAABBPoints(int &min, int &max, vector<Vertex> vertices)
    {
        int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
        for (size_t i = 0; i < vertices.size(); i++)
        {
            if (vertices[i].position.x < vertices[minX].position.x) minX = i;
            if (vertices[i].position.x > vertices[maxX].position.x) minX = i;
            if (vertices[i].position.y < vertices[minY].position.y) minY = i;
            if (vertices[i].position.y > vertices[maxY].position.y) minY = i;
            if (vertices[i].position.z < vertices[minZ].position.z) minZ = i;
            if (vertices[i].position.z > vertices[maxZ].position.z) minZ = i;
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

    void CCollider::SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices)
    {
        int min, max;
        DistantAABBPoints(min, max, vertices);
        center = (vertices[min].position + vertices[max].position) * 0.5f;
        radius = D3DXVec3Dot(&(vertices[max].position - center), &(vertices[max].position - center));
        radius = sqrtf(radius);
    }

    void CCollider::AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, Vertex vertex)
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

    void CCollider::FindCenterWithRadius(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices)
    {
        SphereFromDistPoints(center, radius, vertices);
        for (size_t i = 0; i < vertices.size(); i++)
        {
            AdjustSphere(center, radius, vertices[i]);
        }
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
}
