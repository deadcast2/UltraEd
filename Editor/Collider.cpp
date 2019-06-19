#include "Collider.h"

namespace UltraEd
{
    CCollider::CCollider(vector<Vertex> &vertices)
    {
        m_vertexBuffer = 0;
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
        // Todo: compute optimal radius

        const int segments = 16;
        const float sample = (2 * D3DX_PI) / segments;

        // Horizontal
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(sin(sample * i) * m_radius, 0, cos(sample * i) * m_radius);
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(sin(sample * (i + 1)) * m_radius, 0, cos(sample * (i + 1)) * m_radius);
            m_vertices.push_back(v2);
        }

        // Vertical
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(0, sin(sample * i) * m_radius, cos(sample * i) * m_radius);
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(0, sin(sample * (i + 1)) * m_radius, cos(sample * (i + 1)) * m_radius);
            m_vertices.push_back(v2);
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
