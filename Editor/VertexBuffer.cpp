#include "VertexBuffer.h"

namespace UltraEd
{
    CVertexBuffer::CVertexBuffer() : m_vertexBuffer()
    { }

    CVertexBuffer::~CVertexBuffer()
    {
        Release();
    }

    IDirect3DVertexBuffer8 *CVertexBuffer::GetBuffer(IDirect3DDevice8 *device, vector<Vertex> vertices)
    {
        if (m_vertexBuffer == NULL)
        {
            if (FAILED(device->CreateVertexBuffer(
                vertices.size() * sizeof(Vertex),
                0,
                D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                D3DPOOL_DEFAULT,
                &m_vertexBuffer)))
            {
                return NULL;
            }

            VOID *pVertices;
            if (FAILED(m_vertexBuffer->Lock(0, vertices.size() * sizeof(Vertex),
                (BYTE **)&pVertices, 0)))
            {
                return NULL;
            }

            memcpy(pVertices, &vertices[0], vertices.size() * sizeof(Vertex));
            m_vertexBuffer->Unlock();
        }

        return m_vertexBuffer;
    }

    void CVertexBuffer::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
            m_vertexBuffer = 0;
        }
    }
}
