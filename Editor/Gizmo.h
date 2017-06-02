#ifndef _GIZMO_H_
#define _GIZMO_H_

#include "Camera.h"
#include "Model.h"

enum GizmoState { TransX, TransY, TransZ };

enum GizmoModifierState { Translate, Scale, Rotate };

class CGizmo  
{
public:
  CGizmo();
  ~CGizmo();
  void SetCamera(CCamera *camera);
  void SetModifier(GizmoModifierState state);
  void Update(D3DXVECTOR3 orig, D3DXVECTOR3 dir, CModel *model);
  void Reset();
  BOOL Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir);
  void Release();
  void Render(IDirect3DDevice8* device, ID3DXMatrixStack *matrixStack);
  
private:
  void UpdateScale();
  void SetPosition(D3DXVECTOR3 position);
  void SetScale(D3DXVECTOR3 scale);
  D3DXVECTOR3 GetModifyVector();
  void SetupMaterials();
  void SetupScaleHandles();
  void SetupTransHandles();
  void SetupRotateHandles();
  
private:
  D3DMATERIAL8 m_redMaterial;
  D3DMATERIAL8 m_greenMaterial;
  D3DMATERIAL8 m_blueMaterial;
  CModel m_models[9];
  GizmoState m_state;
  GizmoModifierState m_modifierState;
  D3DXVECTOR3 m_updateStartPoint;
  CCamera *m_camera;
};

#endif