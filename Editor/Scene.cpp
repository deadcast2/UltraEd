#include "Scene.h"
#include "PubSub.h"
#include "Util.h"

namespace UltraEd
{
    Scene::Scene() :
        m_d3d9(),
        m_device(),
        m_gui(),
        m_d3dpp(),
        m_activeViewType(ViewType::Perspective),
        m_views(),
        m_backgroundColorRGB(),
        m_worldLight(),
        m_grid(),
        m_mouseSmoothX(0),
        m_mouseSmoothY(0)
    {
        m_worldLight.Type = D3DLIGHT_DIRECTIONAL;
        m_worldLight.Diffuse.r = 1.0f;
        m_worldLight.Diffuse.g = 1.0f;
        m_worldLight.Diffuse.b = 1.0f;
        m_worldLight.Direction = D3DXVECTOR3(0, 0, 1);

        PubSub::Subscribe({ "Resize", [&](void *data) {
            auto rect = static_cast<tuple<int, int> *>(data);
            if (rect) Resize(get<0>(*rect), get<1>(*rect));
        } });

        PubSub::Subscribe({ "MouseWheel", [&](void *data) {
            auto delta = static_cast<int *>(data);
            if (delta) OnMouseWheel(*delta);
        } });

        PubSub::Subscribe({ "ViewChange", [&](void *data) {
            auto type = static_cast<ViewType *>(data);
            m_activeViewType = *type;
            UpdateViewMatrix();
        } });
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

    void Scene::CheckInput(float deltaTime)
    {
        POINT mousePoint;
        GetCursorPos(&mousePoint);
        ScreenToClient(GetWndHandle(), &mousePoint);
        View *view = GetActiveView();

        static POINT prevMousePoint = mousePoint;
        static DWORD prevTick = GetTickCount();
        static bool prevGizmo = false;
        static GUID groupId = Util::NewGuid();

        const float smoothingModifier = 20.0f;
        const float mouseSpeedModifier = 0.55f;
        const bool mouseReady = GetTickCount() - prevTick < 100;

        // Only accept input when mouse in scene.
        if (m_gui->WantsMouse()) return;

        if (GetAsyncKeyState(VK_RBUTTON) && m_activeViewType == ViewType::Perspective && mouseReady)
        {
            if (GetAsyncKeyState('W')) view->Walk(4.0f * deltaTime);
            if (GetAsyncKeyState('S')) view->Walk(-4.0f * deltaTime);
            if (GetAsyncKeyState('A')) view->Strafe(-4.0f * deltaTime);
            if (GetAsyncKeyState('D')) view->Strafe(4.0f * deltaTime);

            m_mouseSmoothX = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothX, (FLOAT)(mousePoint.x - prevMousePoint.x));
            m_mouseSmoothY = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothY, (FLOAT)(mousePoint.y - prevMousePoint.y));

            view->Yaw(m_mouseSmoothX * mouseSpeedModifier * deltaTime);
            view->Pitch(m_mouseSmoothY * mouseSpeedModifier * deltaTime);
            WrapCursor();
        }
        else if (GetAsyncKeyState(VK_MBUTTON) && mouseReady)
        {
            m_mouseSmoothX = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothX, (FLOAT)(prevMousePoint.x - mousePoint.x));
            m_mouseSmoothY = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothY, (FLOAT)(mousePoint.y - prevMousePoint.y));

            view->Strafe(m_mouseSmoothX * deltaTime);
            view->Fly(m_mouseSmoothY * deltaTime);
            WrapCursor();
        }
        else
        {
            // Reset smoothing values for new mouse view movement.
            m_mouseSmoothX = m_mouseSmoothY = 0;
            prevGizmo = false;
            groupId = Util::NewGuid();
        }

        // Remember the last position so we know how much to move the view.
        prevMousePoint = mousePoint;
        prevTick = GetTickCount();
    }

    void Scene::OnMouseWheel(short delta)
    {
        if (!m_gui->WantsMouse())
        {
            GetActiveView()->SingleStep(delta);
            UpdateViewMatrix();
        }
    }

    void Scene::WrapCursor()
    {
        const int screenX = GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
        const int screenY = GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1;

        POINT mousePoint;
        GetCursorPos(&mousePoint);

        if (mousePoint.x >= screenX)
            SetCursorPos(1, mousePoint.y);
        else if (mousePoint.x < 1)
            SetCursorPos(screenX - 1, mousePoint.y);
        else if (mousePoint.y >= screenY)
            SetCursorPos(mousePoint.x, 1);
        else if (mousePoint.y < 1)
            SetCursorPos(mousePoint.x, screenY - 1);
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

            m_gui->RebuildWith([&]() {
                ReleaseResources();
                m_device->Reset(&m_d3dpp);
                UpdateViewMatrix();
            });
        }
    }

    void Scene::UpdateViewMatrix()
    {
        D3DXMATRIX viewMat;
        const float aspect = static_cast<float>(m_d3dpp.BackBufferWidth) / static_cast<float>(m_d3dpp.BackBufferHeight);

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

        CheckInput(deltaTime);

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
