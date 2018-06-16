#include "Debug.h"

CDebug* CDebug::m_instance = &Instance();

CDebug::CDebug()
{
  m_vertexBuffer = 0;
  
  ZeroMemory(&m_material, sizeof(D3DMATERIAL8));
  m_material.Emissive.r = 0.55f;
  m_material.Emissive.g = 1.0f;
  m_material.Emissive.b = 0.55f;
}

CDebug::~CDebug()
{
  Release();
}

CDebug& CDebug::Instance()
{
  if(m_instance == NULL)
    m_instance = new CDebug;

  return *m_instance;
}

void CDebug::DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to)
{
  Instance()._DrawLine(from, to);
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
  MeshVertex v1;
  v1.position = from;
    
  MeshVertex v2;
  v2.position = to;
  
  m_vertices.push_back(v1);
  m_vertices.push_back(v2);

  Release();
}

IDirect3DVertexBuffer8 *CDebug::GetBuffer(IDirect3DDevice8 *device)
{
  if(m_vertexBuffer == NULL)
  {
    if(FAILED(device->CreateVertexBuffer(
      m_vertices.size() * sizeof(MeshVertex),
      0, 
      D3DFVF_XYZ,
      D3DPOOL_DEFAULT,
      &m_vertexBuffer)))
    {
      return NULL;
    }
    
    VOID *pVertices;
    if(FAILED(m_vertexBuffer->Lock(0, m_vertices.size() * sizeof(MeshVertex),
      (BYTE**)&pVertices, 0)))
    {
      return NULL;
    }
    
    memcpy(pVertices, &m_vertices[0], m_vertices.size() * sizeof(MeshVertex));
    m_vertexBuffer->Unlock();
  }
  
  return m_vertexBuffer;
}

void CDebug::Render(IDirect3DDevice8 *device)
{
  IDirect3DVertexBuffer8 *buffer = GetBuffer(device);
  
  if(buffer != NULL)
  {
    device->SetMaterial(&m_material);
    device->SetStreamSource(0, buffer, sizeof(MeshVertex));
    device->SetVertexShader(D3DFVF_XYZ);
    device->DrawPrimitive(D3DPT_LINELIST, 0, m_vertices.size() / 2);
  }
}

void CDebug::Release()
{
  if(m_vertexBuffer != NULL)
  {
    m_vertexBuffer->Release();
    m_vertexBuffer = 0;
  }
}
