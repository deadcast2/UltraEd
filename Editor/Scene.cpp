#include "Scene.h"
#include "Settings.h"
#include "FileIO.h"
#include "Dialog.h"
#include "Util.h"

CScene::CScene()
{
  ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
  m_selectedModelId = GUID_NULL;
  m_d3d8 = 0;
  m_device = 0;
  m_fillMode = D3DFILL_SOLID;
  
  ZeroMemory(&m_defaultMaterial, sizeof(D3DMATERIAL8));
  m_defaultMaterial.Diffuse.r = m_defaultMaterial.Ambient.r = 1.0f;
  m_defaultMaterial.Diffuse.g = m_defaultMaterial.Ambient.g = 1.0f;
  m_defaultMaterial.Diffuse.b = m_defaultMaterial.Ambient.b = 1.0f;
  m_defaultMaterial.Diffuse.a = m_defaultMaterial.Ambient.a = 1.0f;

  ZeroMemory(&m_selectedMaterial, sizeof(D3DMATERIAL8));
  m_selectedMaterial.Ambient.r = m_selectedMaterial.Emissive.r = 0.0f;
  m_selectedMaterial.Ambient.g = m_selectedMaterial.Emissive.g = 1.0f;
  m_selectedMaterial.Ambient.b = m_selectedMaterial.Emissive.b = 0.0f;
  m_selectedMaterial.Ambient.a = m_selectedMaterial.Emissive.a = 1.0f;
  
  ZeroMemory(&m_worldLight, sizeof(D3DLIGHT8));
  m_worldLight.Type = D3DLIGHT_DIRECTIONAL;
  m_worldLight.Diffuse.r = 1.0f;
  m_worldLight.Diffuse.g = 1.0f;
  m_worldLight.Diffuse.b = 1.0f;
  m_worldLight.Direction = D3DXVECTOR3(0, 0, 1);
}

CScene::~CScene()
{
  ReleaseResources(ModelRelease::AllResources);
  if(m_device) m_device->Release();
  if(m_d3d8) m_d3d8->Release();
}

bool CScene::Create(HWND windowHandle) 
{
  if((m_d3d8 = Direct3DCreate8(D3D_SDK_VERSION)) == NULL)
  {
    return false;
  }
  
  D3DDISPLAYMODE d3ddm;
  if(FAILED(m_d3d8->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
  {
    return false;
  }
  
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
  m_d3dpp.BackBufferFormat = d3ddm.Format;
  m_d3dpp.EnableAutoDepthStencil = TRUE;
  m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
  
  if(FAILED(m_d3d8->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_device)))
  {
    return false;
  }
  
  // Setup the new scene.
  OnNew();
  Resize();
  
  return true;
}

void CScene::OnNew()
{
  // Update the window title.
  SetTitle("New");
  m_selectedModelId = GUID_NULL;
  ReleaseResources(ModelRelease::AllResources);
  m_models.clear();
  ResetCameras();
}

void CScene::OnSave()
{
  vector<CSavable*> savables;

  // Save all editor cameras.
  for(int i = 0; i < 4; i++) savables.push_back(&m_cameras[i]);

  // Save all of the models in the scene.
  for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
  {
    savables.push_back(&it->second);
  }

  string savedName;
  if(CFileIO::Save(savables, savedName))
  {
    SetTitle(savedName);
  }
}

void CScene::OnLoad()
{
  cJSON *root = NULL;
  string loadedName;
  if(CFileIO::Load(&root, loadedName))
  {
    OnNew();
    SetTitle(loadedName);

    // Restore editor cameras.
    int count = 0;
    cJSON *cameras = cJSON_GetObjectItem(root, "cameras");
    cJSON *cameraItem = NULL;
    cJSON_ArrayForEach(cameraItem, cameras)
    {
      m_cameras[count++].Load(m_device, cameraItem);
    }

    // Create saved models.
    cJSON *models = cJSON_GetObjectItem(root, "models");
    cJSON *modelItem = NULL;
    cJSON_ArrayForEach(modelItem, models)
    {
      CModel model;
      if(model.Load(m_device, modelItem))
      {
        m_models[model.GetId()] = model;
      }
    }

    cJSON_Delete(root);
  }
}

void CScene::OnImportModel() 
{
  string file;
  if(CDialog::Open("Import Model", 
    "3D Studio (*.3ds)\0*.3ds\0Autodesk (*.fbx)\0*.fbx\0"
    "Collada (*.dae)\0*.dae\0DirectX (*.x)\0*.x\0Stl (*.stl)\0*.stl\0"
    "VRML (*.wrl)\0*.wrl\0Wavefront (*.obj)\0*.obj", file))
  {
    CModel model = CModel(file.c_str());
    m_models[model.GetId()] = model;
  }
}

void CScene::OnBuildROM()
{
  string sdkPath;
  if(CSettings::Get("N64 SDK Path", sdkPath))
  {
    SetEnvironmentVariable("ROOT", sdkPath.c_str());
    system("cd ..\\Engine && build.bat");
  }
}

void CScene::OnApplyTexture()
{ 
  string file;

  if(m_selectedModelId == GUID_NULL)
  {
    MessageBox(NULL, "An object must be selected first.", "Error", MB_OK);
    return;
  }
  
  if(CDialog::Open("Select a texture",
    "BMP (*.bmp)\0*.bmp\0JPEG (*.jpg)\0"
    "*.jpg\0PNG (*.png)\0*.png\0TGA (*.tga)\0*.tga", file))
  {
    if(!m_models[m_selectedModelId].LoadTexture(m_device, file.c_str()))
    {
      MessageBox(NULL, "Texture could not be loaded.", "Error", MB_OK);
    }
  }
}

bool CScene::Pick(POINT mousePoint)
{
  D3DXVECTOR3 orig, dir;
  ScreenRaycast(mousePoint, &orig, &dir);
  bool gizmoSelected = m_gizmo.Select(orig, dir);
  float closestDist = FLT_MAX;

  // When just selecting the gizmo don't check any models.
  if(gizmoSelected) return true;
  
  // Check all models to see which poly might have been picked.
  for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
  {
    // Only choose the closest model to the camera.
    float pickDist = 0;
    if(it->second.Pick(orig, dir, &pickDist) && pickDist < closestDist)
    {
      closestDist = pickDist;
      m_selectedModelId = it->first;
    }
  }

  if(closestDist != FLT_MAX) return true;
  if(!gizmoSelected) m_selectedModelId = GUID_NULL;
  return false;
}

void CScene::Resize() 
{
  if(m_device)
  {
    ReleaseResources(ModelRelease::VertexBufferOnly);
    m_device->Reset(&m_d3dpp);
    UpdateViewMatrix();
  }
}

void CScene::UpdateViewMatrix()
{
  D3DXMATRIX viewMat;
  if(m_activeCameraView == CameraView::Perspective)
  {
    RECT rect;
    GetClientRect(GetWndHandle(), &rect);

    float aspect = (float)rect.right / (float)rect.bottom;
    float fov = 3.14f / 2.0f;
  
    D3DXMatrixPerspectiveFovLH(&viewMat, fov, aspect, 0.1f, 1000.0f);
  }
  else
  {
    float size = D3DXVec3Length(&GetActiveCamera()->GetPosition());
    D3DXMatrixOrthoLH(&viewMat, size, size, 0.1f, 1000.0f);
  }

  m_device->SetTransform(D3DTS_PROJECTION, &viewMat);
}

void CScene::Render() 
{
  // Calculate the frame rendering speed.
  static float lastTime = (float)timeGetTime();
  float currentTime = (float)timeGetTime();
  float deltaTime = (currentTime - lastTime) * 0.001f;
  
  CheckInput(deltaTime);
  
  if(m_device)
  {
    ID3DXMatrixStack *stack;
    if(!SUCCEEDED(D3DXCreateMatrixStack(0, &stack))) return;
    stack->LoadMatrix(&GetActiveCamera()->GetViewMatrix());
    
    m_device->SetTransform(D3DTS_WORLD, stack->GetTop());
    m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(90, 90, 90), 1.0f, 0);
    
    if(!SUCCEEDED(m_device->BeginScene())) return;
    
    m_device->SetLight(0, &m_worldLight);
    m_device->LightEnable(0, TRUE);
    m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
    
    // Draw the grid.
    m_grid.Render(m_device);
    
    // Draw any debug lines.
    CDebug::Instance().Render(m_device);
    
    // Render all models with selected fill mode.
    m_device->SetMaterial(&m_defaultMaterial);
    m_device->SetRenderState(D3DRS_FILLMODE, m_fillMode);
    for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
    {
      it->second.Render(m_device, stack);
    }
    
    if(m_selectedModelId != GUID_NULL)
    {
      // Highlight the selected model.
      m_device->SetMaterial(&m_selectedMaterial);
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
      m_models[m_selectedModelId].Render(m_device, stack);

      // Draw the gizmo on "top" of all objects in scene.
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
      m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
      m_gizmo.Render(m_device, stack, GetActiveCamera());
    }
    
    m_device->EndScene();
    m_device->Present(NULL, NULL, NULL, NULL);
  }
  
  lastTime = currentTime;
}

void CScene::CheckInput(float deltaTime)
{
  POINT mousePoint;
  GetCursorPos(&mousePoint);
  ScreenToClient(GetWndHandle(), &mousePoint);
  CCamera *camera = GetActiveCamera();
  
  static POINT prevMousePoint = mousePoint;
  const float smoothingModifier = 16.0f;
  const float mouseSpeedModifier = 0.55f;
  
  if(GetAsyncKeyState('1')) m_gizmo.SetModifier(Translate);
  if(GetAsyncKeyState('2')) m_gizmo.SetModifier(Scale);
  if(GetAsyncKeyState('3')) m_gizmo.SetModifier(Rotate);
  
  if(GetAsyncKeyState(VK_LBUTTON) && m_selectedModelId != GUID_NULL)
  {
    D3DXVECTOR3 rayOrigin, rayDir;
    ScreenRaycast(mousePoint, &rayOrigin, &rayDir);
    m_gizmo.Update(rayOrigin, rayDir, &m_models[m_selectedModelId], GetActiveCamera());
  }
  else if(GetAsyncKeyState(VK_RBUTTON) && m_activeCameraView == CameraView::Perspective)
  {
    if(GetAsyncKeyState('W')) camera->Walk(4.0f * deltaTime);
    if(GetAsyncKeyState('S')) camera->Walk(-4.0f * deltaTime);
    
    if(GetAsyncKeyState('A')) camera->Strafe(-4.0f * deltaTime);
    if(GetAsyncKeyState('D')) camera->Strafe(4.0f * deltaTime);
    
    if(GetAsyncKeyState('Q')) camera->Roll(4.0f * deltaTime);
    if(GetAsyncKeyState('E')) camera->Roll(-4.0f * deltaTime);
    
    mouseSmoothX = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothX, mousePoint.x - prevMousePoint.x);
    
    mouseSmoothY = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothY, mousePoint.y - prevMousePoint.y);
    
    camera->Yaw(mouseSmoothX * mouseSpeedModifier * deltaTime);
    camera->Pitch(mouseSmoothY * mouseSpeedModifier * deltaTime);
  }
  else if(GetAsyncKeyState(VK_MBUTTON))
  {
    mouseSmoothX = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothX, prevMousePoint.x - mousePoint.x);
    
    mouseSmoothY = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothY, mousePoint.y - prevMousePoint.y);
    
    camera->Strafe(mouseSmoothX * deltaTime);
    camera->Fly(mouseSmoothY * deltaTime);
  }
  else
  {
    m_gizmo.Reset();
    
    // Reset smoothing values for new mouse camera movement.
    mouseSmoothX = mouseSmoothX = 0;
  }
  
  // Remeber the last position so we know how much to move the camera.
  prevMousePoint = mousePoint;
}

void CScene::OnMouseWheel(short zDelta) 
{
  GetActiveCamera()->Walk(zDelta * 0.005f);
  UpdateViewMatrix();
}

void CScene::ScreenRaycast(POINT screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir)
{
  CCamera *camera = GetActiveCamera();

  D3DVIEWPORT8 viewport;
  m_device->GetViewport(&viewport);

  D3DXMATRIX matProj;
  m_device->GetTransform(D3DTS_PROJECTION, &matProj);

  D3DXMATRIX matWorld;
  D3DXMatrixIdentity(&matWorld);

  D3DXVECTOR3 v1;
  D3DXVECTOR3 start = D3DXVECTOR3(screenPoint.x, screenPoint.y, 0);
  D3DXVec3Unproject(&v1, &start, &viewport, &matProj, &camera->GetViewMatrix(), &matWorld);

  D3DXVECTOR3 v2;
  D3DXVECTOR3 end = D3DXVECTOR3(screenPoint.x, screenPoint.y, 1);
  D3DXVec3Unproject(&v2, &end, &viewport, &matProj, &camera->GetViewMatrix(), &matWorld);

  *origin = v1;
  D3DXVec3Normalize(dir, &(v2 - v1));
}

void CScene::SetCameraView(CameraView::Value view)
{
  m_activeCameraView = view;
  UpdateViewMatrix();
}

void CScene::SetGizmoModifier(GizmoModifierState state)
{
  m_gizmo.SetModifier(state);
}

CCamera *CScene::GetActiveCamera()
{
  return &m_cameras[m_activeCameraView];
}

bool CScene::ToggleMovementSpace()
{
  return m_gizmo.ToggleSpace();
}

bool CScene::ToggleFillMode()
{
  if(m_fillMode == D3DFILL_SOLID)
  {
    m_fillMode = D3DFILL_WIREFRAME;
    return false;
  }

  m_fillMode = D3DFILL_SOLID;
  return true;
}

void CScene::ReleaseResources(ModelRelease::Value type)
{
  m_grid.Release();
  m_gizmo.Release();
  CDebug::Instance().Release();
  
  for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
  {
    it->second.Release(type);
  }
}

void CScene::Delete()
{
  if(m_selectedModelId != GUID_NULL)
  {
    m_models[m_selectedModelId].Release(ModelRelease::AllResources);
    m_models.erase(m_selectedModelId);
    m_selectedModelId = GUID_NULL;
  }
}

void CScene::Duplicate()
{
  if(m_selectedModelId != GUID_NULL)
  {
    CModel model = m_models[m_selectedModelId];

    // Copy texture if present on model.
    string tex = model.GetResources()["textureDataPath"];
    model.LoadTexture(m_device, tex.c_str());

    m_models[model.GetId()] = model;
  }
}

HWND CScene::GetWndHandle()
{
  D3DDEVICE_CREATION_PARAMETERS params;
  if(m_device && SUCCEEDED(m_device->GetCreationParameters(&params)))
  {
    return params.hFocusWindow;
  }
  return NULL;
}

void CScene::ResetCameras()
{
  for(int i = 0; i < 4; i++) 
  {
    m_cameras[i].Reset();
    switch(i)
    {
    case CameraView::Perspective:
      m_cameras[i].Fly(2);
      m_cameras[i].Walk(-5);
      break;
    case CameraView::Top:
      m_cameras[i].Fly(12);
      m_cameras[i].Pitch(D3DX_PI / 2);
      break;
    case CameraView::Left:
      m_cameras[i].Yaw(D3DX_PI / 2);
      m_cameras[i].Walk(-12);
      break;
    case CameraView::Front:
      m_cameras[i].Yaw(D3DX_PI);
      m_cameras[i].Walk(-12);
      break;
    }
  }
}

void CScene::SetTitle(string title)
{
  HWND parentWnd = GetParent(GetWndHandle());
  if(parentWnd != NULL)
  {
    title.append(" - ").append(APP_NAME);
    SetWindowText(parentWnd, title.c_str());
  }
}
