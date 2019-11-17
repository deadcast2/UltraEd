#pragma once

#include <memory>
#include <vector>
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    class CDebug
    {
    public:
        CDebug();
        void Release();
        void Render(IDirect3DDevice8 *device);
        static shared_ptr<CDebug> Instance();
        static void DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to);
        static void Log(const char *format, ...);

    private:
        static shared_ptr<CDebug> m_instance;
        D3DMATERIAL8 m_material;
        shared_ptr<CVertexBuffer> m_vertexBuffer;
        vector<Vertex> m_vertices;
        void _DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to);
    };
}
