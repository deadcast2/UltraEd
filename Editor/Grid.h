#ifndef _GRID_H_
#define _GRID_H_

#include <memory>
#include <vector>
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    class Grid
    {
    public:
        Grid();
        ~Grid();
        void Release();
        void Render(IDirect3DDevice9 *device);

    private:
        D3DMATERIAL9 m_material;
        shared_ptr<VertexBuffer> m_vertexBuffer;
        vector<Vertex> m_vertices;
    };
}

#endif
