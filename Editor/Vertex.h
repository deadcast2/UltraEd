#pragma once

#include "deps/DXSDK/include/d3dx8.h"

namespace UltraEd
{
    struct Vertex
    {
        D3DXVECTOR3 position;
        D3DXVECTOR3 normal;
        FLOAT tu, tv;
    };
}
