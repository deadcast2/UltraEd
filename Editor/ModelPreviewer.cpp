#include "Debug.h"
#include "ModelPreviewer.h"
#include "BoxCollider.h"

namespace UltraEd
{
    ModelPreviewer::ModelPreviewer() :
        m_renderDevice(PreviewWidth, PreviewWidth),
        m_worldLight()
    {
        m_worldLight.Type = D3DLIGHT_DIRECTIONAL;
        m_worldLight.Diffuse.r = 1.0f;
        m_worldLight.Diffuse.g = 1.0f;
        m_worldLight.Diffuse.b = 1.0f;
        m_worldLight.Direction = D3DXVECTOR3(0, 0, 1);
    }

    void ModelPreviewer::Render(LPDIRECT3DDEVICE9 deviceTarget, const std::filesystem::path &path, LPDIRECT3DTEXTURE9 *texture)
    {
        auto device = m_renderDevice.GetDevice();

        ID3DXMatrixStack *stack;
        if (!SUCCEEDED(D3DXCreateMatrixStack(0, &stack))) return;

        device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);
        device->SetLight(0, &m_worldLight);
        device->LightEnable(0, TRUE);

        if (SUCCEEDED(device->BeginScene()))
        {
            try
            {
                auto model = Model(path.string().c_str());
                CenterModel(model);
                model.Render(device, stack);
            }
            catch (const std::exception &e)
            {
                Debug::Instance().Error(std::string(e.what()));
            }
            
            device->EndScene();
            device->Present(NULL, NULL, NULL, NULL);

            Util::BackBufferToTexture(PreviewWidth, PreviewWidth, device, deviceTarget, texture);
        }

        stack->Release();
    }

    void ModelPreviewer::CenterModel(Model &model)
    {
        auto boxCollider = std::make_unique<BoxCollider>(model.GetVertices());
        auto center = boxCollider->GetCenter();
        auto extents = boxCollider->GetExtents();
        
        std::vector<float> extentsList({ extents.x, extents.y, extents.z });
        auto largestExtent = *std::max_element(extentsList.begin(), extentsList.end());     
     
        D3DXMATRIX viewMat;
        D3DXMatrixLookAtLH(&viewMat, 
            &D3DXVECTOR3(center.x, center.y, center.z - (2 * largestExtent)),
            &D3DXVECTOR3(0, center.y, 0),
            &D3DXVECTOR3(0, 1, 0));
        m_renderDevice.GetDevice()->SetTransform(D3DTS_VIEW, &viewMat);

        D3DXMATRIX projMat;
        const float fov = D3DX_PI / 2.0f;
        D3DXMatrixPerspectiveFovLH(&projMat, fov, 1.0f, 0.1f, 1000.0f);
        m_renderDevice.GetDevice()->SetTransform(D3DTS_PROJECTION, &projMat);
    }
}
