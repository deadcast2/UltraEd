#include "Debug.h"

namespace UltraEd
{
    shared_ptr<CDebug> CDebug::m_instance = Instance();

    CDebug::CDebug() :
        m_material(),
        m_vertexBuffer(make_shared<CVertexBuffer>()),
        m_vertices()
    {
        m_material.Emissive.r = 0.55f;
        m_material.Emissive.g = 1.0f;
        m_material.Emissive.b = 0.55f;
    }

    shared_ptr<CDebug> CDebug::Instance()
    {
        if (m_instance == NULL) m_instance = make_shared<CDebug>();
        return m_instance;
    }

    void CDebug::DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to)
    {
        Instance()->_DrawLine(from, to);
    }

    void CDebug::Log(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[256];
        vsprintf(buffer, format, args);
        OutputDebugString(buffer);

        va_end(args);
    }

    void CDebug::_DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to)
    {
        Vertex v1;
        v1.position = from;

        Vertex v2;
        v2.position = to;

        m_vertices.push_back(v1);
        m_vertices.push_back(v2);

        Release();
    }

    void CDebug::Render(IDirect3DDevice8 *device)
    {
        IDirect3DVertexBuffer8 *buffer = m_vertexBuffer->GetBuffer(device, m_vertices);

        if (buffer != NULL)
        {
            device->SetMaterial(&m_material);
            device->SetStreamSource(0, buffer, sizeof(Vertex));
            device->SetVertexShader(D3DFVF_XYZ);
            device->DrawPrimitive(D3DPT_LINELIST, 0, m_vertices.size() / 2);
        }
    }

    void CDebug::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
        }
    }
}
