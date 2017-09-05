#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <string>
#include "cJSON.h"
#include "Savable.h"

#include <d3dx8.h>

class CCamera : public CSavable
{
public:
  CCamera();
  ~CCamera();
  Savable Save();
  bool Load(IDirect3DDevice8 *device, cJSON *root);
  D3DXVECTOR3 GetPosition();
  D3DXVECTOR3 GetForward();
  D3DXVECTOR3 GetRight();
  D3DXVECTOR3 GetUp();
  D3DXMATRIX GetViewMatrix();
  void Reset();
  void Fly(float units); // up/down
  void Strafe(float units); // left/right
  void Walk(float units); // forward/backward
  void Pitch(float angle); // rotate on right vector
  void Yaw(float angle); // rotate on up vector
  void Roll(float angle); // rotate on look vector
  
private:
  D3DXVECTOR3 m_right;
  D3DXVECTOR3 m_up;
  D3DXVECTOR3 m_forward;
  D3DXVECTOR3 m_pos;
};

#endif