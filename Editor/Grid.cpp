#include "Grid.h"

namespace UltraEd
{
    Grid::Grid() :
        m_material(),
        m_vertexBuffer(make_shared<VertexBuffer>()),
        m_vertices()
    {
        int i = 0;
        const float size = 30.0f;

        // Create the x-axis lines.
        for (i = 0; i <= size; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(-size / 2 + i, 0, -size / 2);

            Vertex v2;
            v2.position = D3DXVECTOR3(-size / 2 + i, 0, size / 2);

            m_vertices.push_back(v1);
            m_vertices.push_back(v2);
        }

        // Create the z-axis lines.
        for (i = 0; i <= size; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(-size / 2, 0, -size / 2 + i);

            Vertex v2;
            v2.position = D3DXVECTOR3(size / 2, 0, -size / 2 + i);

            m_vertices.push_back(v1);
            m_vertices.push_back(v2);
        }

        m_material.Emissive.r = 0.55f;
        m_material.Emissive.g = 0.55f;
        m_material.Emissive.b = 0.55f;
    }

    Grid::~Grid()
    {
        Release();
    }

    void Grid::Render(IDirect3DDevice9 *device)
    {
        auto *buffer = m_vertexBuffer->GetBuffer(device, m_vertices);

        if (buffer != NULL)
        {
            device->SetMaterial(&m_material);
            device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
            device->SetFVF(D3DFVF_XYZ);
            device->DrawPrimitive(D3DPT_LINELIST, 0, (UINT)m_vertices.size() / 2);
        }
    }

    void Grid::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
        }
    }
}
