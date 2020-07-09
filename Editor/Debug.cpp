#include "Debug.h"

namespace UltraEd
{
    shared_ptr<Debug> Debug::m_instance = Instance();

    Debug::Debug() :
        m_material(),
        m_vertexBuffer(make_shared<VertexBuffer>()),
        m_vertices()
    {
        m_material.Emissive.r = 0.55f;
        m_material.Emissive.g = 1.0f;
        m_material.Emissive.b = 0.55f;
    }

    shared_ptr<Debug> Debug::Instance()
    {
        if (m_instance == NULL) m_instance = make_shared<Debug>();
        return m_instance;
    }

    void Debug::DrawLine(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to)
    {
        Instance()->BuildLine(from, to);
    }

    void Debug::Log(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[256];
        vsprintf(buffer, format, args);
        OutputDebugString(buffer);

        va_end(args);
    }

    void Debug::BuildLine(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to)
    {
        Vertex v1;
        v1.position = from;

        Vertex v2;
        v2.position = to;

        m_vertices.push_back(v1);
        m_vertices.push_back(v2);

        Release();
    }

    void Debug::Render(IDirect3DDevice9 *device)
    {
        auto *buffer = m_vertexBuffer->GetBuffer(device, m_vertices);

        if (buffer != NULL)
        {
            device->SetMaterial(&m_material);
            device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
            device->SetFVF(D3DFVF_XYZ);
            device->DrawPrimitive(D3DPT_LINELIST, 0, m_vertices.size() / 2);
        }
    }

    void Debug::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
        }
    }
}
