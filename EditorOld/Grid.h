#pragma once

#include <memory>
#include <vector>
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    class CGrid
    {
    public:
        CGrid();
        void Release();
        void Render(IDirect3DDevice8 *device);

    private:
        D3DMATERIAL8 m_material;
        shared_ptr<CVertexBuffer> m_vertexBuffer;
        vector<Vertex> m_vertices;
    };
}
