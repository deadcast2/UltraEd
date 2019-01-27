#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#pragma warning(disable: 4786)

#include "deps/Assimp/include/assimp/Importer.hpp"
#include "deps/Assimp/include/assimp/scene.h"
#include "deps/Assimp/include/assimp/postprocess.h"
#include <vector>
#include "Mesh.h"
#include "Savable.h"

using namespace std;

struct GameObjectType
{
  enum Value { Model, EditorCamera };
};

struct GameObjectRelease
{
  enum Value { AllResources, VertexBufferOnly };
};

class CGameObject : public CSavable
{
public:
  CGameObject();
  CGameObject(const char *filePath, GameObjectType::Value type);
  CGameObject(const CGameObject &gameObject);
  ~CGameObject();
  Savable Save();
  bool Load(IDirect3DDevice8 *device, cJSON *root);
  bool LoadTexture(IDirect3DDevice8 *device, const char *filePath);
  D3DXMATRIX GetMatrix();
  D3DXMATRIX GetRotationMatrix();
  void SetLocalRotationMatrix(D3DXMATRIX mat);
  void Move(D3DXVECTOR3 position);
  void Scale(D3DXVECTOR3 position);
  void Rotate(FLOAT angle, D3DXVECTOR3 dir);
  GUID GetId();
  string GetName();
  void SetName(string name);
  GameObjectType::Value GetType();
  void ResetId();
  D3DXVECTOR3 GetPosition();
  void SetPosition(D3DXVECTOR3 position);
  void SetRotation(D3DXVECTOR3 rotation);
  D3DXVECTOR3 GetScale();
  void SetScale(D3DXVECTOR3 scale);
  D3DXVECTOR3 GetRight();
  D3DXVECTOR3 GetForward();
  D3DXVECTOR3 GetUp();
  void GetAxisAngle(D3DXVECTOR3 *axis, float *angle);
  void Release(GameObjectRelease::Value type);
  void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);
  vector<MeshVertex> GetVertices();
  bool Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist);
  void SetScript(string script);
  string GetScript();

private:
  void Init();
  void Import(const char *filePath);
  
private:
  GUID m_id;
  string m_name;
  GameObjectType::Value m_type;
  IDirect3DVertexBuffer8 *m_vertexBuffer;
  void Process(aiNode *node, const aiScene *scene);
  IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
  bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
    D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, float *dist);
  vector<MeshVertex> m_vertices;
  D3DXVECTOR3 m_position;
  D3DXVECTOR3 m_scale;
  D3DXMATRIX m_localRot;
  D3DXMATRIX m_worldRot;
  LPDIRECT3DTEXTURE8 m_texture;
  string m_script;
  float m_collisionRadius;
};

#endif