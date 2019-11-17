#pragma once

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
    class CVertexBuffer
    {
    public:
        CVertexBuffer();
        ~CVertexBuffer();
        IDirect3DVertexBuffer8 *CVertexBuffer::GetBuffer(IDirect3DDevice8 *device, vector<Vertex> vertices);
        void Release();

    private:
        IDirect3DVertexBuffer8 *m_vertexBuffer;
    };
}
