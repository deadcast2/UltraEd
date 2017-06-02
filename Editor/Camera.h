#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <d3dx8.h>

class CCamera  
{
public:
  CCamera();
  ~CCamera();
  D3DXVECTOR3 GetPosition();
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
  D3DXVECTOR3 m_look;
  D3DXVECTOR3 m_pos;
};

#endif