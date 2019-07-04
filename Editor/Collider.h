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
        CCollider();
        void Release();
        void Render(IDirect3DDevice8 *device);
        virtual void Compute(vector<Vertex> &vertices) = 0;
        D3DXVECTOR3 GetCenter() { return m_center; }
        
    protected:
        ColliderType::Value m_type;
        vector<Vertex> m_vertices;
        D3DXVECTOR3 m_center;

    private:
        D3DMATERIAL8 m_material;
        IDirect3DVertexBuffer8 *m_vertexBuffer;
        IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
    };
}
