#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <memory>
#include <vector>
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    class Debug
    {
    public:
        Debug();
        void Release();
        void Render(IDirect3DDevice9 *device);
        static shared_ptr<Debug> Instance();
        static void DrawLine(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to);
        static void Log(const char *format, ...);

    private:
        static shared_ptr<Debug> m_instance;
        D3DMATERIAL9 m_material;
        shared_ptr<VertexBuffer> m_vertexBuffer;
        vector<Vertex> m_vertices;
        void BuildLine(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to);
    };
}

#endif
