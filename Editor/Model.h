#ifndef _MODEL_H_
#define _MODEL_H_

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "Mesh.h"
#include "Savable.h"

enum ModelRelease { AllResources, VertexBufferOnly };

class CModel : public CSavable
{
public:
  CModel();
  CModel(const char* filePath);
  ~CModel();
  char* Save();
  bool Load(char* data);
  BOOL LoadTexture(IDirect3DDevice8 *device, const char* filePath);
  D3DXMATRIX GetMatrix();
  D3DXMATRIX GetRotationMatrix();
  void SetLocalRotationMatrix(D3DXMATRIX mat);
  void Move(D3DXVECTOR3 position);
  void Scale(D3DXVECTOR3 position);
  void Rotate(FLOAT angle, D3DXVECTOR3 dir);
  void RotateLocal(FLOAT angle, D3DXVECTOR3 dir);
  GUID GetId();
  D3DXVECTOR3 GetPosition();
  void SetPosition(D3DXVECTOR3 position);
  void SetRotation(D3DXVECTOR3 rotation);
  D3DXVECTOR3 GetScale();
  void SetScale(D3DXVECTOR3 scale);
  D3DXVECTOR3 GetRight();
  D3DXVECTOR3 GetForward();
  D3DXVECTOR3 GetUp();
  void Release(ModelRelease type);
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
  D3DXMATRIX m_localRot;
  D3DXMATRIX m_worldRot;
  LPDIRECT3DTEXTURE8 m_texture;
};

#endif