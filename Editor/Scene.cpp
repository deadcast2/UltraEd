#include "Scene.h"

namespace UltraEd
{
    Scene::Scene() :
        m_d3d9(),
        m_device(),
        m_d3dpp(),
        m_activeViewType(ViewType::Perspective),
        m_views(),
        m_backgroundColorRGB(),
        m_grid()
    {
        m_worldLight.Type = D3DLIGHT_DIRECTIONAL;
        m_worldLight.Diffuse.r = 1.0f;
        m_worldLight.Diffuse.g = 1.0f;
        m_worldLight.Diffuse.b = 1.0f;
        m_worldLight.Direction = D3DXVECTOR3(0, 0, 1);
    }

    bool Scene::Create(HWND hWnd)
    {
        if (hWnd == NULL)
            return false;

        m_d3d9 = unique_ptr<IDirect3D9, function<void(IDirect3D9 *)>>(Direct3DCreate9(D3D_SDK_VERSION),
                                                                      [](IDirect3D9 *d3d) { d3d->Release(); });

        if (m_d3d9 == NULL)
            return false;

        D3DDISPLAYMODE d3ddm;
        if (FAILED(m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
            return false;

        m_d3dpp.Windowed = TRUE;
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
        m_d3dpp.BackBufferFormat = d3ddm.Format;
        m_d3dpp.EnableAutoDepthStencil = TRUE;
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

        IDirect3DDevice9 *tempDevice;
        if (FAILED(m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &tempDevice)))
            return false;

        m_device = unique_ptr<IDirect3DDevice9, function<void(IDirect3DDevice9 *)>>(tempDevice,
                                                                                    [](IDirect3DDevice9 *device) { device->Release(); });

        m_gui = make_unique<Gui>(hWnd, m_device.get());

        OnNew();

        return true;
    }

    void Scene::OnNew(bool confirm)
    {
        ResetViews();

        m_backgroundColorRGB[0] = m_backgroundColorRGB[1] = m_backgroundColorRGB[2] = 0;
    }

    void Scene::Resize(int width, int height)
    {
        if (m_device)
        {
            m_d3dpp.BackBufferWidth = width;
            m_d3dpp.BackBufferHeight = height;

            ImGui_ImplDX9_InvalidateDeviceObjects();
            ReleaseResources();
            m_device->Reset(&m_d3dpp);
            UpdateViewMatrix(width, height);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    void Scene::UpdateViewMatrix(int width, int height)
    {
        D3DXMATRIX viewMat;
        const float aspect = static_cast<float>(width) / static_cast<float>(height);

        if (GetActiveView()->GetType() == ViewType::Perspective)
        {
            const float fov = D3DX_PI / 2.0f;
            D3DXMatrixPerspectiveFovLH(&viewMat, fov, aspect, 0.1f, 1000.0f);
        }
        else
        {
            const float zoom = GetActiveView()->GetZoom();
            D3DXMatrixOrthoLH(&viewMat, zoom * aspect, zoom, -1000.0f, 1000.0f);
        }

        m_device->SetTransform(D3DTS_PROJECTION, &viewMat);
    }

    HWND Scene::GetWndHandle()
    {
        D3DDEVICE_CREATION_PARAMETERS params;
        if (m_device && SUCCEEDED(m_device->GetCreationParameters(&params)))
        {
            return params.hFocusWindow;
        }
        return NULL;
    }

    View *Scene::GetActiveView()
    {
        return &m_views[static_cast<int>(m_activeViewType)];
    }

    void Scene::Render()
    {
        // Calculate the frame rendering speed.
        static double lastTime = (double)timeGetTime();
        const double currentTime = (double)timeGetTime();
        const float deltaTime = (float)(currentTime - lastTime) * 0.001f;

        if (m_gui && m_device)
        {
            m_gui->PrepareFrame();

            ID3DXMatrixStack *stack;
            if (!SUCCEEDED(D3DXCreateMatrixStack(0, &stack)))
                return;

            D3DXMATRIX activeViewMatrix = GetActiveView()->GetViewMatrix();
            stack->LoadMatrix(&activeViewMatrix);

            m_device->SetTransform(D3DTS_WORLD, stack->GetTop());
            m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                            D3DCOLOR_XRGB(m_backgroundColorRGB[0], m_backgroundColorRGB[1], m_backgroundColorRGB[2]), 1.0f, 0);
            m_device->SetLight(0, &m_worldLight);
            m_device->LightEnable(0, TRUE);

            if (!SUCCEEDED(m_device->BeginScene()))
                return;

            m_gui->RenderFrame();
            m_grid.Render(m_device.get());

            m_device->EndScene();
            m_device->Present(NULL, NULL, NULL, NULL);
            stack->Release();
        }

        lastTime = currentTime;
    }

    void Scene::ResetViews()
    {
        for (int i = 0; i < 4; i++)
        {
            m_views[i].Reset();
            switch (static_cast<ViewType>(i))
            {
                case ViewType::Perspective:
                {
                    m_views[i].Fly(2);
                    m_views[i].Walk(-5);
                    m_views[i].SetViewType(ViewType::Perspective);
                    break;
                }
                case ViewType::Top:
                {
                    m_views[i].Fly(12);
                    m_views[i].Pitch(D3DX_PI / 2);
                    m_views[i].SetViewType(ViewType::Top);
                    break;
                }
                case ViewType::Left:
                {
                    m_views[i].Yaw(D3DX_PI / 2);
                    m_views[i].Walk(-12);
                    m_views[i].SetViewType(ViewType::Left);
                    break;
                }
                case ViewType::Front:
                {
                    m_views[i].Yaw(D3DX_PI);
                    m_views[i].Walk(-12);
                    m_views[i].SetViewType(ViewType::Front);
                    break;
                }
            }
        }
    }

    void Scene::ReleaseResources()
    {
        m_grid.Release();
    }
}
