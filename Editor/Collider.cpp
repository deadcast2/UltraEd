#include "Collider.h"
#include "FileIO.h"

namespace UltraEd
{
    Collider::Collider() : 
        m_type(ColliderType::Box),
        m_vertices(),
        m_center(), 
        m_originalCenter(),
        m_material(),
        m_vertexBuffer(std::make_shared<VertexBuffer>())
    {
        m_material.Emissive.r = 0;
        m_material.Emissive.g = 1;
        m_material.Emissive.b = 0;
    }

    void Collider::Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack)
    {
        auto *buffer = m_vertexBuffer->GetBuffer(device, m_vertices);

        if (buffer != NULL)
        {
            device->SetTransform(D3DTS_WORLD, stack->GetTop());
            device->SetMaterial(&m_material);
            device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
            device->SetFVF(D3DFVF_XYZ);
            device->DrawPrimitive(D3DPT_LINELIST, 0, static_cast<UINT>(m_vertices.size() / 2));
        }
    }

    void Collider::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
        }
    }

    void Collider::MinMaxAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const std::vector<Vertex> &vertices)
    {
        int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
        for (int i = 0; i < vertices.size(); i++)
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

    void Collider::MostDistantAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const std::vector<Vertex> &vertices)
    {
        int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
        for (int i = 0; i < vertices.size(); i++)
        {
            if (vertices[i].position.x < vertices[minX].position.x) minX = i;
            if (vertices[i].position.x > vertices[maxX].position.x) maxX = i;
            if (vertices[i].position.y < vertices[minY].position.y) minY = i;
            if (vertices[i].position.y > vertices[maxY].position.y) maxY = i;
            if (vertices[i].position.z < vertices[minZ].position.z) minZ = i;
            if (vertices[i].position.z > vertices[maxZ].position.z) maxZ = i;
        }

        const float dist2x = D3DXVec3Dot(&(vertices[maxX].position - vertices[minX].position), &(vertices[maxX].position - vertices[minX].position));
        const float dist2y = D3DXVec3Dot(&(vertices[maxY].position - vertices[minY].position), &(vertices[maxY].position - vertices[minY].position));
        const float dist2z = D3DXVec3Dot(&(vertices[maxZ].position - vertices[minZ].position), &(vertices[maxZ].position - vertices[minZ].position));

        min = vertices[minX].position;
        max = vertices[maxX].position;

        if (dist2y > dist2x && dist2y > dist2z)
        {
            min = vertices[minY].position;
            max = vertices[maxY].position;
        }

        if (dist2z > dist2x && dist2z > dist2y)
        {
            min = vertices[minZ].position;
            max = vertices[maxZ].position;
        }
    }

    nlohmann::json Collider::Save()
    {
        return {
            { "type", m_type },
            { "center", m_center }
        };
    }

    void Collider::Load(const nlohmann::json &root)
    {
        m_type = root["type"];
        m_center = root["center"];
    }
}
