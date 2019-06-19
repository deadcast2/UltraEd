#pragma once

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
    struct ColliderType
    {
        enum Value { Sphere };
    };

    class CCollider
    {
    public:
        CCollider(vector<Vertex> &vertices);
        void Release();
        void Render(IDirect3DDevice8 *device);
        void Compute(vector<Vertex> &vertices);
        float GetRadius() { return m_radius; }

    private:
        D3DMATERIAL8 m_material;
        IDirect3DVertexBuffer8 *m_vertexBuffer;
        IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
        vector<Vertex> m_vertices;
        ColliderType::Value m_type;
        float m_radius;
    };
}
