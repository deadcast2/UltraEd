#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

#include <vector>
#include "Vertex.h"

namespace UltraEd
{
    class VertexBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer();
        IDirect3DVertexBuffer9 *GetBuffer(IDirect3DDevice9 *device, std::vector<Vertex> vertices);
        void Release();

    private:
        IDirect3DVertexBuffer9 *m_vertexBuffer;
    };
}

#endif
