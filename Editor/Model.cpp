#include "Model.h"
#include "FileIO.h"
#include "Util.h"

CModel::CModel()
{
  Init();
}

CModel::CModel(const char *filePath)
{
  Init();
  Import(filePath);
}

CModel::~CModel()
{
}

void CModel::Init()
{
  ResetId();

  m_vertexBuffer = 0;
  m_texture = 0;
  m_position = D3DXVECTOR3(0, 0, 0);
  m_scale = D3DXVECTOR3(1, 1, 1);

  D3DXMatrixIdentity(&m_localRot);
  D3DXMatrixIdentity(&m_worldRot);
}

void CModel::Import(const char *filePath)
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

void CModel::Process(aiNode *node, const aiScene *scene)
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

IDirect3DVertexBuffer8 *CModel::GetBuffer(IDirect3DDevice8 *device)
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

vector<MeshVertex> CModel::GetVertices()
{
  return m_vertices;
}

void CModel::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack)
{
  IDirect3DVertexBuffer8 *buffer = GetBuffer(device);
  
  if(buffer != NULL)
  {
    stack->Push();
    stack->MultMatrixLocal(&GetMatrix());
    
    if(m_texture != NULL) device->SetTexture(0, m_texture);

    device->SetTransform(D3DTS_WORLD, stack->GetTop());
    device->SetStreamSource(0, buffer, sizeof(MeshVertex));
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
    device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_vertices.size() / 3);

    device->SetTexture(0, NULL);
    
    stack->Pop();
  }
}

void CModel::Release(ModelRelease::Value type)
{
  if(m_vertexBuffer != NULL)
  {
    m_vertexBuffer->Release();
    m_vertexBuffer = 0;
  }

  if(type == ModelRelease::VertexBufferOnly) return;

  if(m_texture != NULL)
  {
    m_texture->Release();
    m_texture = 0;
  }
}

GUID CModel::GetId()
{
  return m_id;
}

void CModel::ResetId()
{
  m_id = CUtil::NewGuid();
}

D3DXVECTOR3 CModel::GetPosition()
{
  return m_position;
}

void CModel::SetPosition(D3DXVECTOR3 position)
{
  m_position = position;
}

void CModel::SetRotation(D3DXVECTOR3 rotation)
{
  D3DXMatrixRotationYawPitchRoll(&m_worldRot, rotation.y, rotation.x, rotation.z);
}

D3DXVECTOR3 CModel::GetScale()
{
  return m_scale;
}

void CModel::SetScale(D3DXVECTOR3 scale)
{
  m_scale = scale;
}

D3DXVECTOR3 CModel::GetRight()
{
  D3DXVECTOR3 right = D3DXVECTOR3(1, 0, 0);
  D3DXVec3TransformCoord(&right, &right, &GetRotationMatrix());
  return right;
}

D3DXVECTOR3 CModel::GetForward()
{
  D3DXVECTOR3 forward = D3DXVECTOR3(0, 0, 1);
  D3DXVec3TransformCoord(&forward, &forward, &GetRotationMatrix());
  return forward;
}

D3DXVECTOR3 CModel::GetUp()
{
  D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
  D3DXVec3TransformCoord(&up, &up, &GetRotationMatrix());
  return up;
}

void CModel::Move(D3DXVECTOR3 position)
{
  m_position += position;
}

void CModel::Scale(D3DXVECTOR3 position)
{
  m_scale += position;
}

void CModel::Rotate(FLOAT angle, D3DXVECTOR3 dir)
{
  D3DXMATRIX newWorld;
  D3DXMatrixRotationAxis(&newWorld, &dir, angle);
  m_worldRot *= newWorld;
}

D3DXMATRIX CModel::GetMatrix()
{
  D3DXMATRIX translation;
  D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);
  
  D3DXMATRIX scale;
  D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);
  
  return scale * m_worldRot * m_localRot * translation;
}

D3DXMATRIX CModel::GetRotationMatrix()
{  
  return m_worldRot;
}

void CModel::SetLocalRotationMatrix(D3DXMATRIX mat)
{
  m_localRot = mat;
}

bool CModel::Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist)
{
  vector<MeshVertex> vertices = GetVertices();
  
  // Test all faces in this model.
  for(int j = 0; j < vertices.size() / 3; j++)
  {
    D3DXVECTOR3 v0 = vertices[3 * j + 0].position;
    D3DXVECTOR3 v1 = vertices[3 * j + 1].position;
    D3DXVECTOR3 v2 = vertices[3 * j + 2].position;
    
    // Transform the local vert positions based of the models
    // local matrix so when the model is moved around we can still click it.
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

bool CModel::IntersectTriangle(const D3DXVECTOR3 &orig,
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

bool CModel::LoadTexture(IDirect3DDevice8 *device, const char *filePath)
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

Savable CModel::Save()
{
  char buffer[LINE_FORMAT_LENGTH];
  cJSON *root = cJSON_CreateObject();
  cJSON *model = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "model", model);

  cJSON_AddStringToObject(model, "id", CUtil::GuidToString(m_id).c_str());

  sprintf(buffer, "%f %f %f", m_position.x, m_position.y, m_position.z);
  cJSON_AddStringToObject(model, "position", buffer);

  sprintf(buffer, "%f %f %f", m_scale.x, m_scale.y, m_scale.z);
  cJSON_AddStringToObject(model, "scale", buffer);

  D3DXQUATERNION quat;
  D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
  sprintf(buffer, "%f %f %f %f", quat.x, quat.y, quat.z, quat.w);
  cJSON_AddStringToObject(model, "rotation", buffer);

  Savable savable = { root, SavableType::Model };

  return savable;
}

bool CModel::Load(IDirect3DDevice8 *device, cJSON *root)
{
  float x, y, z, w;
  cJSON *id = cJSON_GetObjectItem(root, "id");
  cJSON *resources = cJSON_GetObjectItem(root, "resources");
  cJSON *resource = NULL;

  m_id = CUtil::StringToGuid(id->valuestring);

  cJSON *position = cJSON_GetObjectItem(root, "position");
  sscanf(position->valuestring, "%f %f %f", &x, &y, &z);
  m_position = D3DXVECTOR3(x, y, z);

  cJSON *scale = cJSON_GetObjectItem(root, "scale");
  sscanf(scale->valuestring, "%f %f %f", &x, &y, &z);
  m_scale = D3DXVECTOR3(x, y, z);

  cJSON *rotation = cJSON_GetObjectItem(root, "rotation");
  sscanf(rotation->valuestring, "%f %f %f %f", &x, &y, &z, &w);
  D3DXMatrixRotationQuaternion(&m_worldRot, &D3DXQUATERNION(x, y, z, w));
  
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
