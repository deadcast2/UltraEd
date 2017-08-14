#include "Camera.h"

CCamera::CCamera()
{
  Reset();
}

CCamera::~CCamera()
{
}

void CCamera::Reset()
{
  m_right = D3DXVECTOR3(1, 0, 0);
  m_up = D3DXVECTOR3(0, 1, 0);
  m_forward = D3DXVECTOR3(0, 0, 1);
  m_pos = D3DXVECTOR3(0, 2, -6);
}

D3DXMATRIX CCamera::GetViewMatrix()
{
  D3DXMATRIX view;
  
  // Keep camera's axes orthogonal to each other:
  D3DXVec3Normalize(&m_forward, &m_forward);
  D3DXVec3Cross(&m_up, &m_forward, &m_right);
  D3DXVec3Normalize(&m_up, &m_up);
  D3DXVec3Cross(&m_right, &m_up, &m_forward);
  D3DXVec3Normalize(&m_right, &m_right);
  
  // Build the view matrix:
  float x = -D3DXVec3Dot(&m_right, &m_pos);
  float y = -D3DXVec3Dot(&m_up, &m_pos);
  float z = -D3DXVec3Dot(&m_forward, &m_pos);
  
  view(0, 0) = m_right.x;
  view(0, 1) = m_up.x;
  view(0, 2) = m_forward.x;
  view(0, 3) = 0.0f;
  view(1, 0) = m_right.y;
  view(1, 1) = m_up.y;
  view(1, 2) = m_forward.y;
  view(1, 3) = 0.0f;
  view(2, 0) = m_right.z;
  view(2, 1) = m_up.z;
  view(2, 2) = m_forward.z;
  view(2, 3) = 0.0f;
  view(3, 0) = x;
  view(3, 1) = y;
  view(3, 2) = z;
  view(3, 3) = 1.0f;
  
  return view;
}

void CCamera::Pitch(float angle)
{
  D3DXMATRIX T;
  D3DXMatrixRotationAxis(&T, &m_right, angle);
  
  // rotate _up and _look around _right vector
  D3DXVec3TransformCoord(&m_up, &m_up, &T);
  D3DXVec3TransformCoord(&m_forward, &m_forward, &T);
}

void CCamera::Yaw(float angle)
{
  D3DXMATRIX T;
  D3DXMatrixRotationY(&T, angle);
  
  // rotate _right and _look around _up or y-axis
  D3DXVec3TransformCoord(&m_right, &m_right, &T);
  D3DXVec3TransformCoord(&m_forward, &m_forward, &T);
}

void CCamera::Roll(float angle)
{
  D3DXMATRIX T;
  D3DXMatrixRotationAxis(&T, &m_forward, angle);
  
  // rotate _up and _right around _look vector
  D3DXVec3TransformCoord(&m_right, &m_right, &T);
  D3DXVec3TransformCoord(&m_up, &m_up, &T);
}

void CCamera::Walk(float units)
{
  m_pos += m_forward * units;
}

void CCamera::Strafe(float units)
{
  m_pos += m_right * units;
}

void CCamera::Fly(float units)
{
  m_pos += m_up * units;
}

D3DXVECTOR3 CCamera::GetPosition()
{
  return m_pos;
}

D3DXVECTOR3 CCamera::GetForward()
{
  return m_forward;
}

D3DXVECTOR3 CCamera::GetRight()
{
  return m_right;
}

D3DXVECTOR3 CCamera::GetUp()
{
  return m_up;
}

char* CCamera::Save()
{
  char buffer[128];
  cJSON *root = cJSON_CreateObject();
  cJSON *camera = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "camera", camera);

  // Position.
  D3DXVECTOR3 cameraPos = GetPosition();
  sprintf(buffer, "%f %f %f", cameraPos.x, cameraPos.y, cameraPos.z);
  cJSON_AddStringToObject(camera, "position", buffer);

  // Forward.
  D3DXVECTOR3 cameraForward = GetForward();
  sprintf(buffer, "%f %f %f", cameraForward.x, cameraForward.y, cameraForward.z);
  cJSON_AddStringToObject(camera, "forward", buffer);

  // Right.
  D3DXVECTOR3 cameraRight = GetRight();
  sprintf(buffer, "%f %f %f", cameraRight.x, cameraRight.y, cameraRight.z);
  cJSON_AddStringToObject(camera, "right", buffer);

  // Up.
  D3DXVECTOR3 cameraUp = GetUp();
  sprintf(buffer, "%f %f %f", cameraUp.x, cameraUp.y, cameraUp.z);
  cJSON_AddStringToObject(camera, "up", buffer);

  char *rendered = cJSON_Print(root);
  cJSON_Delete(root);

  return rendered;
}