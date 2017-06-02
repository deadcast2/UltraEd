#ifndef _GRID_H_
#define _GRID_H_

#include "Mesh.h"

class CGrid  
{
public:
  CGrid();
  ~CGrid();
  void Release();
  void Render(IDirect3DDevice8*);
  
private:
  D3DMATERIAL8 m_whiteMaterial;
  IDirect3DVertexBuffer8* m_vertexBuffer;
  IDirect3DVertexBuffer8* GetBuffer(IDirect3DDevice8*);
  std::vector<MeshVertex> m_vertices;
};

#endif