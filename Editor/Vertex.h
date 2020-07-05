#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <d3dx9.h>

namespace UltraEd
{
    struct Vertex
    {
        D3DXVECTOR3 position;
        D3DXVECTOR3 normal;
        D3DCOLOR color;
        FLOAT tu, tv;
    };
}

#endif
