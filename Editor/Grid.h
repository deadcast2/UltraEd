#ifndef _GRID_H_
#define _GRID_H_

#include "Mesh.h"

using namespace std;

class CGrid  
{
public:
  CGrid();
  ~CGrid();
  void Release();
  void Render(IDirect3DDevice8 *device);
  
private:
  D3DMATERIAL8 m_material;
  IDirect3DVertexBuffer8 *m_vertexBuffer;
  IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
  vector<MeshVertex> m_vertices;
};

#endif