#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
    class VertexBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer();
        IDirect3DVertexBuffer9 *GetBuffer(IDirect3DDevice9 *device, vector<Vertex> vertices);
        void Release();

    private:
        IDirect3DVertexBuffer9 *m_vertexBuffer;
    };
}

#endif
