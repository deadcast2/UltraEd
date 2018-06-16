#include "build.h"
#include "Scene.h"
#include "Settings.h"
#include "FileIO.h"
#include "Dialog.h"
#include "Util.h"

CScene::CScene()
{
  ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
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
  ReleaseResources(GameObjectRelease::AllResources);
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
  SetTitle("New");
  selectedGameObjectIds.clear();
  ReleaseResources(GameObjectRelease::AllResources);
  m_gameObjects.clear();
  ResetCameras();
}

void CScene::OnSave()
{
  vector<CSavable*> savables;

  // Save all editor cameras.
  for(int i = 0; i < 4; i++) savables.push_back(&m_cameras[i]);

  // Save all of the game objects in the scene.
  for(map<GUID, CGameObject>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
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

    // Create saved game objects.
    cJSON *gameObjects = cJSON_GetObjectItem(root, "gameObjects");
    cJSON *gameObjectItem = NULL;
    cJSON_ArrayForEach(gameObjectItem, gameObjects)
    {
      CGameObject gameObject;
      if(gameObject.Load(m_device, gameObjectItem))
      {
        m_gameObjects[gameObject.GetId()] = gameObject;
      }
    }

    cJSON_Delete(root);
  }
}

void CScene::OnImportModel() 
{
  string file;
  if(CDialog::Open("Add Model", 
    "3D Studio (*.3ds)\0*.3ds\0Autodesk (*.fbx)\0*.fbx\0"
    "Collada (*.dae)\0*.dae\0DirectX (*.x)\0*.x\0Stl (*.stl)\0*.stl\0"
    "VRML (*.wrl)\0*.wrl\0Wavefront (*.obj)\0*.obj", file))
  {
    CGameObject gameObject = CGameObject(file.c_str(), GameObjectType::Model);
    m_gameObjects[gameObject.GetId()] = gameObject;
    char buffer[1024];
    sprintf(buffer, "GameObject %d", m_gameObjects.size());
    m_gameObjects[gameObject.GetId()].SetName(string(buffer));
  }
}

void CScene::OnBuildROM(BuildFlag::Value flag)
{
  vector<CGameObject*> gameObjects;

  // Gather all of the game objects in the scene.
  for(map<GUID, CGameObject>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
  {
    gameObjects.push_back(&it->second);
  }

  if(CBuild::Start(gameObjects))
  {
    if(flag & BuildFlag::Run)
    {
      CBuild::Run();
    }
    else if(flag & BuildFlag::Load)
    {
      if(CBuild::Load())
      {
        MessageBox(NULL, "The ROM has been successfully loaded to the cart!", "Success", MB_OK);
      }
      else
      {
        MessageBox(NULL, "Could not load ROM onto cart. Make sure your cart is connected via USB.", "Error", MB_OK);
      }
    }
    else
    {
      MessageBox(NULL, "The ROM has been successfully built!", "Success", MB_OK);
    }
  }
  else
  {
    MessageBox(NULL, "The ROM build has failed. Make sure the correct N64 SDK path was specified.", "Error", MB_OK);
  }
}

void CScene::OnApplyTexture()
{ 
  string file;

  if(selectedGameObjectIds.empty())
  {
    MessageBox(NULL, "An object must be selected first.", "Error", MB_OK);
    return;
  }
  
  for(vector<GUID>::iterator it = selectedGameObjectIds.begin(); it != selectedGameObjectIds.end(); it++)
  {
    if(CDialog::Open("Select a texture",
      "PNG (*.png)\0*.png\0JPEG (*.jpg)\0"
      "*.jpg\0BMP (*.bmp)\0*.bmp\0TGA (*.tga)\0*.tga", file))
    {
      if(m_gameObjects[*it].GetType() != GameObjectType::Model) continue;
      if(!m_gameObjects[*it].LoadTexture(m_device, file.c_str()))
      {
        MessageBox(NULL, "Texture could not be loaded.", "Error", MB_OK);
      }
    }
  }
}

bool CScene::Pick(POINT mousePoint)
{
  D3DXVECTOR3 orig, dir;
  ScreenRaycast(mousePoint, &orig, &dir);
  bool gizmoSelected = m_gizmo.Select(orig, dir);
  float closestDist = FLT_MAX;

  // When just selecting the gizmo don't check any game objects.
  if(gizmoSelected) return true;
  
  // Check all game objects to see which poly might have been picked.
  for(map<GUID, CGameObject>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
  {
    // Only choose the closest game object to the camera.
    float pickDist = 0;
    if(it->second.Pick(orig, dir, &pickDist) && pickDist < closestDist)
    {
      closestDist = pickDist;
      vector<GUID>::iterator found = find(selectedGameObjectIds.begin(), selectedGameObjectIds.end(), it->first);
      if(found == selectedGameObjectIds.end())
      {
        if(!GetAsyncKeyState(VK_SHIFT)) selectedGameObjectIds.clear();
        selectedGameObjectIds.push_back(it->first);
      }
      else
      {
        // Shift clicking an already selected game object so unselect it.
        if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
        {
          selectedGameObjectIds.erase(found);
        }
        else
        {
          // Unselected everything and only select what was clicked.
          selectedGameObjectIds.clear();
          selectedGameObjectIds.push_back(it->first);
        }
      }
    }
  }

  if(closestDist != FLT_MAX) return true;
  if(!gizmoSelected) selectedGameObjectIds.clear();
  return false;
}

void CScene::Resize() 
{
  if(m_device)
  {
    ReleaseResources(GameObjectRelease::VertexBufferOnly);
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
    float fov = D3DX_PI / 2.0f;
  
    D3DXMatrixPerspectiveFovLH(&viewMat, fov, aspect, 0.1f, 1000.0f);
  }
  else
  {
    float size = D3DXVec3Length(&GetActiveCamera()->GetPosition());
    D3DXMatrixOrthoLH(&viewMat, size, size, -1000.0f, 1000.0f);
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
    m_device->SetLight(0, &m_worldLight);
    m_device->LightEnable(0, TRUE);
    
    if(!SUCCEEDED(m_device->BeginScene())) return;
    
    m_grid.Render(m_device);
    CDebug::Instance().Render(m_device);
    
    // Render all game objects with selected fill mode.
    m_device->SetMaterial(&m_defaultMaterial);
    m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_device->SetRenderState(D3DRS_FILLMODE, m_fillMode);
    m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    for(map<GUID, CGameObject>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
    {
      // When not a model then billboard the object.
      if(it->second.GetType() != GameObjectType::Model)
      {
        D3DXMATRIX mat;
        D3DXMatrixIdentity(&mat);
        D3DXMATRIX cameraViewMat = GetActiveCamera()->GetViewMatrix();     
        mat(0, 0) = cameraViewMat(0, 0);
        mat(0, 1) = cameraViewMat(1, 0);
        mat(0, 2) = cameraViewMat(2, 0);
        mat(1, 0) = cameraViewMat(0, 1);
        mat(1, 1) = cameraViewMat(1, 1);
        mat(1, 2) = cameraViewMat(2, 1);
        mat(2, 0) = cameraViewMat(0, 2);
        mat(2, 1) = cameraViewMat(1, 2);
        mat(2, 2) = cameraViewMat(2, 2);
        it->second.SetLocalRotationMatrix(mat);
      }

      it->second.Render(m_device, stack);
    }

    // Need to disable here to not mess with the wireframe rendering.
    m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    if(!selectedGameObjectIds.empty())
    {
      // Highlight the selected game object.
      m_device->SetMaterial(&m_selectedMaterial);
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
      for(vector<GUID>::iterator it = selectedGameObjectIds.begin(); it != selectedGameObjectIds.end(); ++it)
      {
        m_gameObjects[*it].Render(m_device, stack);
      }

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
  if(GetAsyncKeyState('2')) m_gizmo.SetModifier(Rotate);
  if(GetAsyncKeyState('3')) m_gizmo.SetModifier(Scale);
  
  if(GetAsyncKeyState(VK_LBUTTON) && !selectedGameObjectIds.empty())
  {
    D3DXVECTOR3 rayOrigin, rayDir;
    ScreenRaycast(mousePoint, &rayOrigin, &rayDir);
    GUID lastSelectedGameObjectId = selectedGameObjectIds.back();
    for(vector<GUID>::iterator it = selectedGameObjectIds.begin(); it != selectedGameObjectIds.end(); ++it)
    {
      m_gizmo.Update(GetActiveCamera(), rayOrigin, rayDir, &m_gameObjects[*it], &m_gameObjects[lastSelectedGameObjectId]);
    }
  }
  else if(GetAsyncKeyState(VK_RBUTTON) && m_activeCameraView == CameraView::Perspective)
  {
    if(GetAsyncKeyState('W')) camera->Walk(4.0f * deltaTime);
    if(GetAsyncKeyState('S')) camera->Walk(-4.0f * deltaTime);
    
    if(GetAsyncKeyState('A')) camera->Strafe(-4.0f * deltaTime);
    if(GetAsyncKeyState('D')) camera->Strafe(4.0f * deltaTime);
    
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
  if(!selectedGameObjectIds.empty())
  {
    return m_gizmo.ToggleSpace(&m_gameObjects[selectedGameObjectIds.back()]);
  }
  return false;
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

void CScene::ReleaseResources(GameObjectRelease::Value type)
{
  m_grid.Release();
  m_gizmo.Release();
  CDebug::Instance().Release();
  
  for(map<GUID, CGameObject>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
  {
    it->second.Release(type);
  }
}

void CScene::Delete()
{
  if(!selectedGameObjectIds.empty())
  {
    for(vector<GUID>::iterator it = selectedGameObjectIds.begin(); it != selectedGameObjectIds.end(); ++it)
    {
      m_gameObjects[*it].Release(GameObjectRelease::AllResources);
      m_gameObjects.erase(*it);
    }
    selectedGameObjectIds.clear();
  }
}

void CScene::Duplicate()
{
  if(!selectedGameObjectIds.empty())
  {
    for(vector<GUID>::iterator it = selectedGameObjectIds.begin(); it != selectedGameObjectIds.end(); ++it)
    {
      CGameObject gameObject = m_gameObjects[*it];
      string tex = gameObject.GetResources()["textureDataPath"];
      gameObject.LoadTexture(m_device, tex.c_str());
      m_gameObjects[gameObject.GetId()] = gameObject;
    }
  }
}

void CScene::SetScript(string script)
{
  if(!selectedGameObjectIds.empty())
  {
    m_gameObjects[selectedGameObjectIds[0]].SetScript(script);
  }
}

string CScene::GetScript()
{
  if(!selectedGameObjectIds.empty())
  {
    return m_gameObjects[selectedGameObjectIds[0]].GetScript();
  }
  return string("");
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
      m_cameras[i].SetView(CameraView::Perspective);
      break;
    case CameraView::Top:
      m_cameras[i].Fly(12);
      m_cameras[i].Pitch(D3DX_PI / 2);
      m_cameras[i].SetView(CameraView::Top);
      break;
    case CameraView::Left:
      m_cameras[i].Yaw(D3DX_PI / 2);
      m_cameras[i].Walk(-12);
      m_cameras[i].SetView(CameraView::Left);
      break;
    case CameraView::Front:
      m_cameras[i].Yaw(D3DX_PI);
      m_cameras[i].Walk(-12);
      m_cameras[i].SetView(CameraView::Front);
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

bool CScene::ToggleSnapToGrid()
{
  return m_gizmo.ToggleSnapping();
}

void CScene::OnAddCamera()
{
  char buffer[1024];
  CGameObject newCamera = CGameObject(GameObjectType::Camera);
  newCamera.LoadTexture(m_device, "Assets/camera.png");
  m_gameObjects[newCamera.GetId()] = newCamera;
  m_gameObjects[newCamera.GetId()].SetName(string(buffer));
  sprintf(buffer, "Camera %d", m_gameObjects.size());
}
