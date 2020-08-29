#include "ModelPreviewer.h"
#include "Model.h"

namespace UltraEd
{
    LRESULT WINAPI ModelPreviewerWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    ModelPreviewer::ModelPreviewer() :
        m_device(),
        m_d3d9(),
        m_d3dpp()
    {
        if (SetupWindow())
        {
            SetupRenderer();
        }
    }

    ModelPreviewer::~ModelPreviewer()
    {
        if (m_device) m_device->Release();
        if (m_d3d9) m_d3d9->Release();

        DestroyWindow(m_hWnd);
        UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
    }

    void ModelPreviewer::Render(LPDIRECT3DDEVICE9 device, const std::filesystem::path &path, LPDIRECT3DTEXTURE9 *texture)
    {
        auto model = Model(path.string().c_str());

        if (!m_device) return;

        ID3DXMatrixStack *stack;
        if (!SUCCEEDED(D3DXCreateMatrixStack(0, &stack))) return;

        m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);
        if (SUCCEEDED(m_device->BeginScene()))
        {
            model.Render(m_device, stack);
            m_device->EndScene();
            m_device->Present(NULL, NULL, NULL, NULL);

            // Create texture with child device and apply the backbuffer.
            LPDIRECT3DSURFACE9 surface;
            m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);

            LPDIRECT3DTEXTURE9 tempTexture;
            m_device->CreateTexture(64, 64, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &tempTexture, 0);
            
            LPDIRECT3DSURFACE9 tempTextureSurface;
            tempTexture->GetSurfaceLevel(0, &tempTextureSurface);
            m_device->StretchRect(surface, NULL, tempTextureSurface, NULL, D3DTEXF_NONE);

            // Create a texture with the parent device and copy the rendered surface to its context.
            device->CreateTexture(64, 64, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, texture, 0);
            PDIRECT3DSURFACE9 newTextureSurface;
            (*texture)->GetSurfaceLevel(0, &newTextureSurface);
            D3DXLoadSurfaceFromSurface(newTextureSurface, 0, 0, tempTextureSurface, 0, 0, D3DX_DEFAULT, 0);

            surface->Release();
            tempTextureSurface->Release();
            newTextureSurface->Release();
            tempTexture->Release();
        }
    }

    bool ModelPreviewer::SetupWindow()
    {
        m_wc = {
            sizeof(WNDCLASSEX), CS_CLASSDC, ModelPreviewerWndProc, 0L, 0L, GetModuleHandle(NULL),
            NULL, NULL, NULL, NULL, "UltraEdModelPreviewer", NULL
        };

        RegisterClassEx(&m_wc);

        const int windowWidth = 320;
        const int windowHeight = 240;
        m_hWnd = CreateWindow(m_wc.lpszClassName, "UltraEdModelPreviewer v0.1", WS_EX_TOOLWINDOW,
            0, 0, windowWidth, windowHeight, NULL, NULL, m_wc.hInstance, NULL);

        return m_hWnd != NULL;
    }

    bool ModelPreviewer::SetupRenderer()
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

        D3DXMATRIX viewMat;
        const float aspect = static_cast<float>(m_d3dpp.BackBufferWidth) / static_cast<float>(m_d3dpp.BackBufferHeight);
        D3DXMatrixOrthoLH(&viewMat, 1 * aspect, 1, -1000.0f, 1000.0f);
        m_device->SetTransform(D3DTS_PROJECTION, &viewMat);

        return true;
    }

    LRESULT WINAPI ModelPreviewerWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
