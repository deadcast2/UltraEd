#include "GameObject.h"
#include "FileIO.h"
#include "Util.h"

CGameObject::CGameObject()
{
  Init();
}

CGameObject::CGameObject(const CGameObject &gameObject)
{
  *this = gameObject;
  m_vertexBuffer = 0;
  m_texture = 0;
  ResetId();
}

CGameObject::CGameObject(GameObjectType::Value type)
{
  Init();
  m_type = type;
  
  MeshVertex v1;
  v1.position = D3DXVECTOR3(m_position.x - 0.5, m_position.y - 0.5, m_position.z);
  v1.tu = 1;
  v1.tv = 1;
  v1.normal = D3DXVECTOR3(0, 0, -1);
    
  MeshVertex v2;
  v2.position = D3DXVECTOR3(m_position.x - 0.5, m_position.y + 0.5, m_position.z);
  v2.tu = 1;
  v2.tv = 0;
  v2.normal = D3DXVECTOR3(0, 0, -1);

  MeshVertex v3;
  v3.position = D3DXVECTOR3(m_position.x + 0.5, m_position.y + 0.5, m_position.z);
  v3.tu = 0;
  v3.tv = 0;
  v3.normal = D3DXVECTOR3(0, 0, -1);
  
  MeshVertex v4;
  v4.position = D3DXVECTOR3(m_position.x + 0.5, m_position.y - 0.5, m_position.z);
  v4.tu = 0;
  v4.tv = 1;
  v4.normal = D3DXVECTOR3(0, 0, -1);

  m_vertices.push_back(v1);
  m_vertices.push_back(v2);
  m_vertices.push_back(v3);

  m_vertices.push_back(v1);
  m_vertices.push_back(v3);
  m_vertices.push_back(v4);
}

CGameObject::CGameObject(const char *filePath, GameObjectType::Value type)
{
  Init();
  Import(filePath);
  m_type = type;
}

CGameObject::~CGameObject()
{
}

void CGameObject::Init()
{
  ResetId();
  
  m_vertexBuffer = 0;
  m_texture = 0;
  m_position = D3DXVECTOR3(0, 0, 0);
  m_scale = D3DXVECTOR3(1, 1, 1);
  m_script = string("void @start()\n{\n\n}\n\nvoid @update()\n{\n\n}\n\nvoid @input(NUContData gamepads[4])\n{\n\n}");
  
  D3DXMatrixIdentity(&m_localRot);
  D3DXMatrixIdentity(&m_worldRot);
}

void CGameObject::Import(const char *filePath)
{
  Assimp::Importer importer;
  FileInfo info = CFileIO::Import(filePath);
  const aiScene *scene = importer.ReadFile(info.path,
    aiProcess_Triangulate | aiProcess_ConvertToLeftHanded |
    aiProcess_OptimizeMeshes);
  
  // Save path to user imported file for saving later.
  if(info.type == User)
  {
    resources["vertexDataPath"] = info.path;
  }
  
  if(scene)
  {
    Process(scene->mRootNode, scene);
  }
}

void CGameObject::Process(aiNode *node, const aiScene *scene)
{
  int i;
  
  for(i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    vector<MeshVertex> verts = CMesh(node->mTransformation, mesh).Vertices();
    m_vertices.insert(m_vertices.end(), verts.begin(), verts.end()); 
  }
  
  for(i = 0; i < node->mNumChildren; i++)
  {
    Process(node->mChildren[i], scene);
  }
}

IDirect3DVertexBuffer8 *CGameObject::GetBuffer(IDirect3DDevice8 *device)
{
  if(m_vertexBuffer == NULL)
  {
    if(FAILED(device->CreateVertexBuffer(
      m_vertices.size() * sizeof(MeshVertex),
      0, 
      D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,
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

vector<MeshVertex> CGameObject::GetVertices()
{
  return m_vertices;
}

void CGameObject::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack)
{
  IDirect3DVertexBuffer8 *buffer = GetBuffer(device);
  
  if(buffer != NULL)
  {
    stack->Push();
    stack->MultMatrixLocal(&GetMatrix());
    
    if(m_texture != NULL) device->SetTexture(0, m_texture);
    
    device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    device->SetTransform(D3DTS_WORLD, stack->GetTop());
    device->SetStreamSource(0, buffer, sizeof(MeshVertex));
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
    device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_vertices.size() / 3);
    device->SetTexture(0, NULL);
    
    stack->Pop();
  }
}

void CGameObject::Release(GameObjectRelease::Value type)
{
  if(m_vertexBuffer != NULL)
  {
    m_vertexBuffer->Release();
    m_vertexBuffer = 0;
  }
  
  if(type == GameObjectRelease::VertexBufferOnly) return;
  
  if(m_texture != NULL)
  {
    m_texture->Release();
    m_texture = 0;
  }
}

GUID CGameObject::GetId()
{
  return m_id;
}

void CGameObject::ResetId()
{
  m_id = CUtil::NewGuid();
}

string CGameObject::GetName()
{
  return m_name;
}

void CGameObject::SetName(string name)
{
  m_name = name;
}

GameObjectType::Value CGameObject::GetType()
{
  return m_type;
}

D3DXVECTOR3 CGameObject::GetPosition()
{
  return m_position;
}

void CGameObject::SetPosition(D3DXVECTOR3 position)
{
  m_position = position;
}

void CGameObject::SetRotation(D3DXVECTOR3 rotation)
{
  D3DXMatrixRotationYawPitchRoll(&m_worldRot, rotation.y, rotation.x, rotation.z);
}

D3DXVECTOR3 CGameObject::GetScale()
{
  return m_scale;
}

void CGameObject::SetScale(D3DXVECTOR3 scale)
{
  m_scale = scale;
}

D3DXVECTOR3 CGameObject::GetRight()
{
  D3DXVECTOR3 right = D3DXVECTOR3(1, 0, 0);
  D3DXVec3TransformCoord(&right, &right, &GetRotationMatrix());
  return right;
}

D3DXVECTOR3 CGameObject::GetForward()
{
  D3DXVECTOR3 forward = D3DXVECTOR3(0, 0, 1);
  D3DXVec3TransformCoord(&forward, &forward, &GetRotationMatrix());
  return forward;
}

D3DXVECTOR3 CGameObject::GetUp()
{
  D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
  D3DXVec3TransformCoord(&up, &up, &GetRotationMatrix());
  return up;
}

void CGameObject::GetAxisAngle(D3DXVECTOR3 *axis, float *angle)
{
  D3DXQUATERNION quat;
  D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
  D3DXQuaternionToAxisAngle(&quat, axis, angle);
}

void CGameObject::Move(D3DXVECTOR3 position)
{
  m_position += position;
}

void CGameObject::Scale(D3DXVECTOR3 position)
{
  m_scale += position;
}

void CGameObject::Rotate(FLOAT angle, D3DXVECTOR3 dir)
{
  D3DXMATRIX newWorld;
  D3DXMatrixRotationAxis(&newWorld, &dir, angle);
  m_worldRot *= newWorld;
}

D3DXMATRIX CGameObject::GetMatrix()
{
  D3DXMATRIX translation;
  D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);
  
  D3DXMATRIX scale;
  D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);
  
  return scale * m_worldRot * m_localRot * translation;
}

D3DXMATRIX CGameObject::GetRotationMatrix()
{  
  return m_worldRot;
}

void CGameObject::SetLocalRotationMatrix(D3DXMATRIX mat)
{
  m_localRot = mat;
}

bool CGameObject::Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist)
{
  vector<MeshVertex> vertices = GetVertices();
  
  // Test all faces in this game object.
  for(int j = 0; j < vertices.size() / 3; j++)
  {
    D3DXVECTOR3 v0 = vertices[3 * j + 0].position;
    D3DXVECTOR3 v1 = vertices[3 * j + 1].position;
    D3DXVECTOR3 v2 = vertices[3 * j + 2].position;
    
    // Transform the local vert positions based of the game object's
    // local matrix so when the game object is moved around we can still click it.
    D3DXVec3TransformCoord(&v0, &v0, &GetMatrix());
    D3DXVec3TransformCoord(&v1, &v1, &GetMatrix());
    D3DXVec3TransformCoord(&v2, &v2, &GetMatrix());
    
    // Check if the pick ray passes through this point.
    if(IntersectTriangle(orig, dir, v0, v1, v2, dist))
    {
      return true;
    }
  }
  
  return false;
}

bool CGameObject::IntersectTriangle(const D3DXVECTOR3 &orig,
                               const D3DXVECTOR3 &dir, D3DXVECTOR3 &v0,
                               D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, float *dist)
{
  // Find vectors for two edges sharing vert0
  D3DXVECTOR3 edge1 = v1 - v0;
  D3DXVECTOR3 edge2 = v2 - v0;
  
  // Begin calculating determinant - also used to calculate U parameter.
  D3DXVECTOR3 pvec;
  D3DXVec3Cross(&pvec, &dir, &edge2);
  
  // If determinant is near zero, ray lies in plane of triangle.
  float det = D3DXVec3Dot(&edge1, &pvec);
  
  if(det < 0.0001f) return false;
  
  // Calculate U parameter and test bounds.
  D3DXVECTOR3 tvec = orig - v0;
  float u = D3DXVec3Dot(&tvec, &pvec);
  if(u < 0.0f || u > det) return false;
  
  // Prepare to test V parameter.
  D3DXVECTOR3 qvec;
  D3DXVec3Cross(&qvec, &tvec, &edge1);
  
  // Calculate V parameter and test bounds.
  float v = D3DXVec3Dot(&dir, &qvec);
  if(v < 0.0f || u + v > det) return false;
  
  *dist = D3DXVec3Dot(&edge2, &qvec) * (1.0f / det);
  
  return true;
}

bool CGameObject::LoadTexture(IDirect3DDevice8 *device, const char *filePath)
{
  FileInfo info = CFileIO::Import(filePath);
  
  if(FAILED(D3DXCreateTextureFromFile(device, info.path.c_str(), &m_texture)))
  {
    return false;
  }
  
  // Save location of texture for scene saving.
  if(info.type == User) resources["textureDataPath"] = info.path;
  
  return true;
}

Savable CGameObject::Save()
{
  char buffer[LINE_FORMAT_LENGTH];
  cJSON *root = cJSON_CreateObject();
  cJSON *gameObject = cJSON_CreateObject();
  
  cJSON_AddItemToObject(root, "gameObject", gameObject);
  
  cJSON_AddStringToObject(gameObject, "id", CUtil::GuidToString(m_id).c_str());

  cJSON_AddStringToObject(gameObject, "name", m_name.c_str());

  sprintf(buffer, "%i", (int)m_type);
  cJSON_AddStringToObject(gameObject, "type", buffer);
  
  sprintf(buffer, "%f %f %f", m_position.x, m_position.y, m_position.z);
  cJSON_AddStringToObject(gameObject, "position", buffer);
  
  sprintf(buffer, "%f %f %f", m_scale.x, m_scale.y, m_scale.z);
  cJSON_AddStringToObject(gameObject, "scale", buffer);
  
  D3DXQUATERNION quat;
  D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
  sprintf(buffer, "%f %f %f %f", quat.x, quat.y, quat.z, quat.w);
  cJSON_AddStringToObject(gameObject, "rotation", buffer);

  cJSON_AddStringToObject(gameObject, "script", m_script.c_str());
  
  Savable savable = { root, SavableType::GameObject };
  return savable;
}

bool CGameObject::Load(IDirect3DDevice8 *device, cJSON *root)
{
  int typeValue;
  float x, y, z, w;
  cJSON *id = cJSON_GetObjectItem(root, "id");
  cJSON *name = cJSON_GetObjectItem(root, "name");
  cJSON *type = cJSON_GetObjectItem(root, "type");
  cJSON *resources = cJSON_GetObjectItem(root, "resources");
  cJSON *resource = NULL;
  
  m_id = CUtil::StringToGuid(id->valuestring);
  m_name = name->valuestring;

  sscanf(type->valuestring, "%i", &typeValue);
  m_type = (GameObjectType::Value)typeValue;
  
  cJSON *position = cJSON_GetObjectItem(root, "position");
  sscanf(position->valuestring, "%f %f %f", &x, &y, &z);
  m_position = D3DXVECTOR3(x, y, z);
  
  cJSON *scale = cJSON_GetObjectItem(root, "scale");
  sscanf(scale->valuestring, "%f %f %f", &x, &y, &z);
  m_scale = D3DXVECTOR3(x, y, z);
  
  cJSON *rotation = cJSON_GetObjectItem(root, "rotation");
  sscanf(rotation->valuestring, "%f %f %f %f", &x, &y, &z, &w);
  D3DXMatrixRotationQuaternion(&m_worldRot, &D3DXQUATERNION(x, y, z, w));

  cJSON *script = cJSON_GetObjectItem(root, "script");
  m_script = script->valuestring;
  
  // Load any vertex or texture data.
  cJSON_ArrayForEach(resource, resources)
  {
    const char *path = resource->child->valuestring;
    
    if(strcmp(resource->child->string, "vertexDataPath") == 0)
    {  
      Import(path);
    }
    else if(strcmp(resource->child->string, "textureDataPath") == 0)
    {
      LoadTexture(device, path);
    }
  }
  
  return true;
}

void CGameObject::SetScript(string script)
{
  m_script = script;
}

string CGameObject::GetScript()
{
  return m_script;
}
