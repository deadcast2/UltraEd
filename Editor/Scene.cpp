#include "Scene.h"
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
  m_hWnd = windowHandle;
  
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
  
  m_gizmo.SetCamera(&m_camera);
  
  // Setup the new scene.
  OnNew();
  
  return true;
}

void CScene::OnNew()
{
  // Update the window title.
  SetTitle("New");
  
  // Delete any selected objects.
  Delete();
  
  // Release all models.
  for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
  {
    it->second.Release(ModelRelease::AllResources);
  }
  
  m_models.clear();
  m_camera.Reset();
}

void CScene::OnSave()
{
  vector<CSavable*> savables;
  savables.push_back(&m_camera);

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
  // Clear scene.
  OnNew();

  cJSON *root = NULL;
  string loadedName;
  if(CFileIO::Load(&root, loadedName))
  {
    SetTitle(loadedName);
    m_camera.Load(m_device, root);

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

void CScene::OnApplyTexture()
{ 
  string file;

  if(m_selectedModelId == GUID_NULL)
  {
    MessageBox(m_hWnd, "An object must be selected first.", "Error", MB_OK);
    return;
  }
  
  if(CDialog::Open("Select a texture",
    "BMP (*.bmp)\0*.bmp\0JPEG (*.jpg)\0"
    "*.jpg\0PNG (*.png)\0*.png\0TGA (*.tga)\0*.tga", file))
  {
    if(!m_models[m_selectedModelId].LoadTexture(m_device, file.c_str()))
    {
      MessageBox(m_hWnd, "Texture could not be loaded.", "Error", MB_OK);
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
    stack->LoadMatrix(&m_camera.GetViewMatrix());
    
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
      // Draw the gizmo on "top" of all objects in scene.
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
      m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
      m_gizmo.Render(m_device, stack);

      // Highlight the selected model.
      m_device->SetMaterial(&m_selectedMaterial);
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
      m_models[m_selectedModelId].Render(m_device, stack);
    }
    
    m_device->EndScene();
    m_device->Present(NULL, NULL, NULL, NULL);
  }
  
  lastTime = currentTime;
}

void CScene::Resize(int width, int height) 
{
  float aspect = (float)width / (float)height;
  float fov = 3.14f / 2.0f;
  
  D3DXMATRIX m;
  D3DXMatrixPerspectiveFovLH(&m, fov, aspect, 0.1f, 1000.0f);
  
  if(m_device)
  {
    ReleaseResources(ModelRelease::VertexBufferOnly);
    m_device->Reset(&m_d3dpp);
    m_device->SetTransform(D3DTS_PROJECTION, &m);
  }
}

void CScene::CheckInput(float deltaTime)
{
  POINT mousePoint;
  GetCursorPos(&mousePoint);
  ScreenToClient(m_hWnd, &mousePoint);
  
  static POINT prevMousePoint = mousePoint;
  const float smoothingModifier = 18.0f;
  const float mouseSpeedModifier = 0.55f;
  
  if(GetAsyncKeyState('1')) m_gizmo.SetModifier(Translate);
  if(GetAsyncKeyState('2')) m_gizmo.SetModifier(Scale);
  if(GetAsyncKeyState('3')) m_gizmo.SetModifier(Rotate);
  
  if(GetAsyncKeyState(VK_LBUTTON) && m_selectedModelId != GUID_NULL)
  {
    D3DXVECTOR3 rayOrigin, rayDir;
    ScreenRaycast(mousePoint, &rayOrigin, &rayDir);
    m_gizmo.Update(rayOrigin, rayDir, &m_models[m_selectedModelId]);
  }
  else if(GetAsyncKeyState(VK_RBUTTON))
  {
    if(GetAsyncKeyState('W')) m_camera.Walk(4.0f * deltaTime);
    if(GetAsyncKeyState('S')) m_camera.Walk(-4.0f * deltaTime);
    
    if(GetAsyncKeyState('A')) m_camera.Strafe(-4.0f * deltaTime);
    if(GetAsyncKeyState('D')) m_camera.Strafe(4.0f * deltaTime);
    
    if(GetAsyncKeyState('Q')) m_camera.Roll(4.0f * deltaTime);
    if(GetAsyncKeyState('E')) m_camera.Roll(-4.0f * deltaTime);
    
    mouseSmoothX = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothX, mousePoint.x - prevMousePoint.x);
    
    mouseSmoothY = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothY, mousePoint.y - prevMousePoint.y);
    
    m_camera.Yaw(mouseSmoothX * mouseSpeedModifier * deltaTime);
    m_camera.Pitch(mouseSmoothY * mouseSpeedModifier * deltaTime);
  }
  else if(GetAsyncKeyState(VK_MBUTTON))
  {
    mouseSmoothX = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothX, prevMousePoint.x - mousePoint.x);
    
    mouseSmoothY = CUtil::Lerp(deltaTime * smoothingModifier, 
      mouseSmoothY, mousePoint.y - prevMousePoint.y);
    
    m_camera.Strafe(mouseSmoothX * deltaTime);
    m_camera.Fly(mouseSmoothY * deltaTime);
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
  m_camera.Walk(zDelta * 0.005f);
}

void CScene::ScreenRaycast(POINT screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir)
{
  D3DXMATRIX matProj;
  m_device->GetTransform(D3DTS_PROJECTION, &matProj);
  
  RECT clientRect;
  GetClientRect(m_hWnd, &clientRect);
  int width = clientRect.right - clientRect.left;
  int height = clientRect.bottom - clientRect.top;
  
  // Compute the vector of the pick ray in screen space.
  D3DXVECTOR3 v;
  v.x = (((2.0f * screenPoint.x) / width) - 1) / matProj._11;
  v.y = -(((2.0f * screenPoint.y) / height) - 1) / matProj._22;
  v.z = 1.0f;
  
  // Get the camera inverse view matrix.
  D3DXMATRIX m;
  D3DXMatrixInverse(&m, NULL, &m_camera.GetViewMatrix());
  
  // Transform the screen space pick ray into 3D space.
  (*dir).x = v.x * m._11 + v.y * m._21 + v.z * m._31;
  (*dir).y = v.x * m._12 + v.y * m._22 + v.z * m._32;
  (*dir).z = v.x * m._13 + v.y * m._23 + v.z * m._33;
  
  // Center of screen.
  (*origin).x = m._41;
  (*origin).y = m._42;
  (*origin).z = m._43;
}

void CScene::SetGizmoModifier(GizmoModifierState state)
{
  m_gizmo.SetModifier(state);
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
  
  for(map<GUID, CModel>::iterator it = m_models.begin(); it != m_models.end(); ++it)
  {
    it->second.Release(type);
  }
}

void CScene::Delete()
{
  if(m_selectedModelId != GUID_NULL)
  {
    CModel model = m_models[m_selectedModelId];
    model.Release(ModelRelease::AllResources);
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

void CScene::SetTitle(string title)
{
  HWND parentWnd = GetParent(m_hWnd);
  if(parentWnd != NULL)
  {
    title.append(" - ").append(APP_NAME);
    SetWindowText(parentWnd, title.c_str());
  }
}
