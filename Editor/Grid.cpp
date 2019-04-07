#include "Grid.h"

CGrid::CGrid()
{
  m_vertexBuffer = 0;
  
  int i = 0;
  const FLOAT size = 30.0f;
  
  // Create the x-axis lines.
  for(i = 0; i <= size; i++)
  {
    MeshVertex v1;
    v1.position = D3DXVECTOR3(-size / 2 + i, 0, -size / 2);
    
    MeshVertex v2;
    v2.position = D3DXVECTOR3(-size / 2 + i, 0, size / 2);
    
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
  }
  
  // Create the z-axis lines.
  for(i = 0; i <= size; i++)
  {
    MeshVertex v1;
    v1.position = D3DXVECTOR3(-size / 2, 0, -size / 2 + i);
    
    MeshVertex v2;
    v2.position = D3DXVECTOR3(size / 2, 0, -size / 2 + i);
    
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
  }
  
  ZeroMemory(&m_material, sizeof(D3DMATERIAL8));
  m_material.Emissive.r = 0.55f;
  m_material.Emissive.g = 0.55f;
  m_material.Emissive.b = 0.55f;
}

CGrid::~CGrid()
{ 
}

IDirect3DVertexBuffer8 *CGrid::GetBuffer(IDirect3DDevice8 *device)
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
    
    VOID* pVertices;
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

void CGrid::Render(IDirect3DDevice8 *device)
{
  IDirect3DVertexBuffer8* buffer = GetBuffer(device);
  
  if(buffer != NULL)
  {
    device->SetMaterial(&m_material);
    device->SetStreamSource(0, buffer, sizeof(MeshVertex));
    device->SetVertexShader(D3DFVF_XYZ);
    device->DrawPrimitive(D3DPT_LINELIST, 0, m_vertices.size() / 2);
  }
}

void CGrid::Release()
{
  if(m_vertexBuffer != NULL)
  {
    m_vertexBuffer->Release();
    m_vertexBuffer = 0;
  }
}