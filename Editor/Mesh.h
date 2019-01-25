#ifndef _MESH_H_
#define _MESH_H_

#include "deps/DXSDK/include/d3dx8.h"
#include <vector>
#include "deps/Assimp/include/assimp/scene.h"
#include "deps/Assimp/include/assimp/cimport.h"

using namespace std;

struct MeshVertex
{
  D3DXVECTOR3 position;
  D3DXVECTOR3 normal;
  FLOAT tu, tv;
};

class CMesh  
{
public:
  CMesh(aiMatrix4x4 transform, aiMesh *mesh);
  ~CMesh();
  vector<MeshVertex> Vertices();
  
private:
  vector<MeshVertex> m_vertices;
};

#endif