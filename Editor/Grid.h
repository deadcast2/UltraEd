#ifndef _GRID_H_
#define _GRID_H_

#include <memory>
#include <vector>
#include "VertexBuffer.h"

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
        std::shared_ptr<VertexBuffer> m_vertexBuffer;
        std::vector<Vertex> m_vertices;
    };
}

#endif
