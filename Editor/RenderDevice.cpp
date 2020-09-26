#include "RenderDevice.h"
#include "Util.h"

namespace UltraEd 
{
    LRESULT WINAPI RenderDeviceWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    RenderDevice::RenderDevice(UINT width, UINT height) :
        m_id(Util::NewUuid()),
        m_device(),
        m_d3d9(),
        m_d3dpp(),
        m_wc(),
        m_hWnd()
    {
        if (SetupWindow(width, height))
        {
            SetupRenderer();
        }
    }

    RenderDevice::~RenderDevice()
    {
        if (m_device) m_device->Release();
        if (m_d3d9) m_d3d9->Release();

        DestroyWindow(m_hWnd);
        UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
    }

    const LPDIRECT3DDEVICE9 RenderDevice::GetDevice()
    {
        return m_device;
    }

    const D3DPRESENT_PARAMETERS *RenderDevice::GetParameters()
    {
        return &m_d3dpp;
    }

    void RenderDevice::Resize(UINT width, UINT height)
    {
        m_d3dpp.BackBufferWidth = width;
        m_d3dpp.BackBufferHeight = height;
        m_device->Reset(&m_d3dpp);
    }

    bool RenderDevice::SetupWindow(UINT width, UINT height)
    {
        m_wc = {
            sizeof(WNDCLASSEX), CS_CLASSDC, RenderDeviceWndProc, 0L, 0L, GetModuleHandle(NULL),
            NULL, NULL, NULL, NULL, Util::UuidToString(m_id).c_str(), NULL
        };

        RegisterClassEx(&m_wc);

        m_hWnd = CreateWindow(m_wc.lpszClassName, Util::UuidToString(m_id).c_str(), 
            WS_EX_TOOLWINDOW, 0, 0, width, height, NULL, NULL, m_wc.hInstance, NULL);

        return m_hWnd != NULL;
    }

    bool RenderDevice::SetupRenderer()
    {
        if ((m_d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
            return false;

        D3DDISPLAYMODE d3ddm;
        if (FAILED(m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
            return false;

        m_d3dpp.Windowed = TRUE;
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
        m_d3dpp.BackBufferFormat = d3ddm.Format;
        m_d3dpp.EnableAutoDepthStencil = TRUE;
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

        if (FAILED(m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_device)))
            return false;

        return true;
    }

    LRESULT WINAPI RenderDeviceWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}