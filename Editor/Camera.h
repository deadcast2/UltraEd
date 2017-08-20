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
  char* Save();
  bool Load(char* data);
  D3DXVECTOR3 GetPosition();
  D3DXVECTOR3 GetForward();
  D3DXVECTOR3 GetRight();
  D3DXVECTOR3 GetUp();
  void Reset();
  void Fly(float units); // up/down
  void Strafe(float units); // left/right
  void Walk(float units); // forward/backward
  void Pitch(float angle); // rotate on right vector
  void Yaw(float angle); // rotate on up vector
  void Roll(float angle); // rotate on look vector
  D3DXMATRIX GetViewMatrix();
  
private:
  D3DXVECTOR3 m_right;
  D3DXVECTOR3 m_up;
  D3DXVECTOR3 m_forward;
  D3DXVECTOR3 m_pos;
};

#endif