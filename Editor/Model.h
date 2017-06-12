#ifndef _MODEL_H_
#define _MODEL_H_

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "Mesh.h"

class CModel  
{
public:
  CModel();
  CModel(const char* filePath);
  ~CModel();
  BOOL LoadTexture(IDirect3DDevice8 *device, const char* filePath);
  D3DXMATRIX GetMatrix();
  D3DXMATRIX GetRotationMatrix();
  void Move(D3DXVECTOR3 position, D3DXVECTOR3 along);
  void Scale(D3DXVECTOR3 position, D3DXVECTOR3 along);
  void Rotate(D3DXVECTOR3 position, D3DXVECTOR3 along);
  GUID GetId();
  D3DXVECTOR3 GetPosition();
  void SetPosition(D3DXVECTOR3 position);
  D3DXVECTOR3 GetRotation();
  void SetRotation(D3DXVECTOR3 rotation);
  D3DXVECTOR3 GetScale();
  void SetScale(D3DXVECTOR3 scale);
  D3DXVECTOR3 GetRight();
  void Release();
  void Render(IDirect3DDevice8*, ID3DXMatrixStack*);
  std::vector<MeshVertex> GetVertices();
  BOOL Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir);
  
private:
  GUID m_id;
  IDirect3DVertexBuffer8* m_vertexBuffer;
  void Process(aiNode*, const aiScene*);
  IDirect3DVertexBuffer8* GetBuffer(IDirect3DDevice8*);
  BOOL IntersectTriangle(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir,
    D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2);
  std::vector<MeshVertex> m_vertices;
  D3DXVECTOR3 m_position;
  D3DXVECTOR3 m_scale;
  D3DXVECTOR3 m_rotation;
  LPDIRECT3DTEXTURE8 m_texture;
};

#endif