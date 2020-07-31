#include "VertexBuffer.h"

namespace UltraEd
{
    VertexBuffer::VertexBuffer() : m_vertexBuffer() { }

    VertexBuffer::~VertexBuffer()
    {
        Release();
    }

    IDirect3DVertexBuffer9 *VertexBuffer::GetBuffer(IDirect3DDevice9 *device, std::vector<Vertex> vertices)
    {
        if (m_vertexBuffer == NULL)
        {
            if (FAILED(device->CreateVertexBuffer(
                (UINT)vertices.size() * sizeof(Vertex),
                0,
                D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                D3DPOOL_DEFAULT,
                &m_vertexBuffer, 0)))
            {
                return NULL;
            }

            void *pVertices;
            if (FAILED(m_vertexBuffer->Lock(0, (UINT)vertices.size() * sizeof(Vertex), &pVertices, 0)))
            {
                return NULL;
            }

            memcpy(pVertices, &vertices[0], vertices.size() * sizeof(Vertex));
            m_vertexBuffer->Unlock();
        }

        return m_vertexBuffer;
    }

    void VertexBuffer::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
            m_vertexBuffer = 0;
        }
    }
}
