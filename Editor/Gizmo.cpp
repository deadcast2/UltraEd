#include "Gizmo.h"

CGizmo::CGizmo()
{
  // Default state.
  m_modifierState = Translate;

  m_xAxisRot = D3DXVECTOR3(0, -D3DX_PI / 2, 0);
  m_yAxisRot = D3DXVECTOR3(D3DX_PI / 2, 0, 0);
  m_zAxisRot = D3DXVECTOR3(0, D3DX_PI, 0);
  
  SetupMaterials();
  
  SetupTransHandles();
  
  SetupScaleHandles();

  SetupRotateHandles();
  
  Reset();
}

CGizmo::~CGizmo()
{
}

void CGizmo::SetModifier(enum GizmoModifierState state)
{
  m_modifierState = state;
}

D3DXVECTOR3 CGizmo::GetModifyVector()
{
  switch(m_state)
  {
  case XAxis:
    return D3DXVECTOR3(1, 0, 0);
  case YAxis:
    return D3DXVECTOR3(0, 1, 0);
  case ZAxis:
    return D3DXVECTOR3(0, 0, 1);
  }
  
  return D3DXVECTOR3(0, 0, 0);
}

void CGizmo::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *matrixStack)
{
  UpdateScale();

  device->SetMaterial(&m_redMaterial);
  m_models[m_modifierState * 3 + 0].Render(device, matrixStack);

  device->SetMaterial(&m_greenMaterial);
  m_models[m_modifierState * 3 + 1].Render(device, matrixStack);

  device->SetMaterial(&m_blueMaterial);
  m_models[m_modifierState * 3 + 2].Render(device, matrixStack);
}

void CGizmo::Release()
{
  for(int i = 0; i < 9; i++)
  {
    m_models[i].Release();
  }
}

BOOL CGizmo::Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir)
{
  if(m_models[m_modifierState * 3 + 0].Pick(orig, dir))
  {
    m_state = XAxis;
    return TRUE;
  }
  
  if(m_models[m_modifierState * 3 + 1].Pick(orig, dir))
  {
    m_state = YAxis;
    return TRUE;
  }
  
  if(m_models[m_modifierState * 3 + 2].Pick(orig, dir))
  {
    m_state = ZAxis;
    return TRUE;
  }

  return FALSE;
}

void CGizmo::SetPosition(D3DXVECTOR3 position)
{
  for(int i = 0; i < 9; i++)
  {
    m_models[i].SetPosition(position);
  }
}

void CGizmo::SetScale(D3DXVECTOR3 scale)
{
  for(int i = 0; i < 9; i++)
  {
    m_models[i].SetScale(scale);
  }
}

void CGizmo::SetupMaterials()
{
  // Create the red material.
  ZeroMemory(&m_redMaterial, sizeof(D3DMATERIAL8));
  m_redMaterial.Emissive.r = 0.8f;
  m_redMaterial.Emissive.g = 0;
  m_redMaterial.Emissive.b = 0;
  
  // Create the green material.
  ZeroMemory(&m_greenMaterial, sizeof(D3DMATERIAL8));
  m_greenMaterial.Emissive.r = 0;
  m_greenMaterial.Emissive.g = 0.8f;
  m_greenMaterial.Emissive.b = 0;
  
  // Create the blue material.
  ZeroMemory(&m_blueMaterial, sizeof(D3DMATERIAL8));
  m_blueMaterial.Emissive.r = 0;
  m_blueMaterial.Emissive.g = 0;
  m_blueMaterial.Emissive.b = 0.8f;
}

void CGizmo::SetupTransHandles()
{
  m_models[0] = CModel("assets/trans-gizmo.dae");
  m_models[0].Rotate(m_xAxisRot, D3DXVECTOR3(0, 1, 0));

  m_models[1] = CModel("assets/trans-gizmo.dae");
  m_models[1].Rotate(m_yAxisRot, D3DXVECTOR3(1, 0, 0));

  m_models[2] = CModel("assets/trans-gizmo.dae");
  m_models[2].Rotate(m_zAxisRot, D3DXVECTOR3(0, 1, 0));
}

void CGizmo::SetupScaleHandles()
{
  m_models[3] = CModel("assets/scale-gizmo.dae");
  m_models[3].Rotate(m_xAxisRot, D3DXVECTOR3(0, 1, 0));

  m_models[4] = CModel("assets/scale-gizmo.dae");
  m_models[4].Rotate(m_yAxisRot, D3DXVECTOR3(1, 0, 0));

  m_models[5] = CModel("assets/scale-gizmo.dae");
  m_models[5].Rotate(m_zAxisRot, D3DXVECTOR3(0, 1, 0));
}

void CGizmo::SetupRotateHandles()
{
  m_models[6] = CModel("assets/rot-gizmo.dae");
  m_models[6].Rotate(m_xAxisRot, D3DXVECTOR3(0, 1, 0));

  m_models[7] = CModel("assets/rot-gizmo.dae");
  m_models[7].Rotate(m_yAxisRot, D3DXVECTOR3(1, 0, 0));

  m_models[8] = CModel("assets/rot-gizmo.dae");
  m_models[8].Rotate(m_zAxisRot, D3DXVECTOR3(0, 1, 0));
}

void CGizmo::Update(D3DXVECTOR3 orig, D3DXVECTOR3 dir, CModel *model)
{
  D3DXVECTOR3 modelPos = model->GetPosition();
  D3DXVECTOR3 v0, v1, v2, intersectPoint;
  
  // Detect which plane that needs to be computed.
  if(m_state == YAxis)
  {
    v0 = D3DXVECTOR3(modelPos.x, modelPos.y, modelPos.z);
    v1 = D3DXVECTOR3(modelPos.x + 1, modelPos.y, modelPos.z);
    v2 = D3DXVECTOR3(modelPos.x + 1, modelPos.y + 1, modelPos.z);
  }
  else
  {
    v0 = D3DXVECTOR3(modelPos.x, modelPos.y, modelPos.z);
    v1 = D3DXVECTOR3(modelPos.x + 1, modelPos.y, modelPos.z);
    v2 = D3DXVECTOR3(modelPos.x + 1, modelPos.y, modelPos.z + 1);
  }
  
  D3DXPLANE testPlane;
  D3DXPlaneFromPoints(&testPlane, &v0, &v1, &v2); 
  D3DXVECTOR3 rayEnd = orig + (dir * 1000);
  
  if(D3DXPlaneIntersectLine(&intersectPoint, &testPlane,
    &orig, &rayEnd) != NULL)
  {
    if(m_updateStartPoint == D3DXVECTOR3(-999, -999, -999))
    {
      m_updateStartPoint = intersectPoint;
    }
    
    if(m_modifierState == Translate)
    {
      model->Move(intersectPoint - m_updateStartPoint, GetModifyVector());
    }
    else if(m_modifierState == Scale)
    {
      model->Scale(intersectPoint - m_updateStartPoint, GetModifyVector());
    }
    else
    {
      model->Rotate(m_updateStartPoint - intersectPoint, GetModifyVector());

      // Keep gizmo in-sync with the model's rotation.
      D3DXVECTOR3 modelRot = model->GetRotation();

      for(int i = 0; i < 3; i++)
      {
        m_models[i * 3 + 0].SetRotation(D3DXVECTOR3(0, modelRot.y, modelRot.z) + m_xAxisRot);
        m_models[i * 3 + 1].SetRotation(D3DXVECTOR3(modelRot.x, modelRot.y, modelRot.z) + m_yAxisRot);
        m_models[i * 3 + 2].SetRotation(D3DXVECTOR3(-modelRot.x, modelRot.y, modelRot.z) + m_zAxisRot);
      }
    }
    
    m_updateStartPoint = intersectPoint;
  }
  
  SetPosition(model->GetPosition());
}

void CGizmo::Reset()
{
  m_updateStartPoint = D3DXVECTOR3(-999, -999, -999);
}

void CGizmo::SetCamera(CCamera *camera)
{
  m_camera = camera;
}

void CGizmo::UpdateScale()
{
  // Scale the size of the gizmo based on the camera distance.
  if(m_camera != NULL)
  {
    D3DXVECTOR3 distance = m_models[0].GetPosition() - m_camera->GetPosition();
    FLOAT length = D3DXVec3Length(&distance) * 0.2;
    SetScale(D3DXVECTOR3(length, length, length));
  }
}