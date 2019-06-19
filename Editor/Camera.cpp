#include "Camera.h"
#include "FileIO.h"

namespace UltraEd
{
    CCamera::CCamera()
    {
        Import("Assets/camera.dae");
        m_type = ActorType::Camera;
        m_fov = 60.0f;
    }

    CCamera::CCamera(const CCamera &camera)
    {
        *this = camera;
        m_vertexBuffer = 0;
        ResetId();
    }

    void CCamera::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack)
    {
        IDirect3DVertexBuffer8 *buffer = GetBuffer(device);

        if (buffer != NULL)
        {
            stack->Push();
            stack->MultMatrixLocal(&GetMatrix());

            device->SetTransform(D3DTS_WORLD, stack->GetTop());
            device->SetStreamSource(0, buffer, sizeof(Vertex));
            device->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, GetVertices().size() / 3);

            stack->Pop();
        }

        CActor::Render(device, stack);
    }

    Savable CCamera::Save()
    {
        Savable savable = CActor::Save();
        char buffer[LINE_FORMAT_LENGTH];
        sprintf(buffer, "%f", m_fov);
        cJSON_AddStringToObject(cJSON_GetObjectItem(savable.object, "actor"), "fov", buffer);
        return savable;
    }

    bool CCamera::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        CActor::Load(device, root);
        if (cJSON *fov = cJSON_GetObjectItem(root, "fov"))
        {
            sscanf(fov->valuestring, "%f", &m_fov);
        }
        return true;
    }
}
