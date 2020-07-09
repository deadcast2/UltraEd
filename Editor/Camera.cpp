#include "Camera.h"
#include "FileIO.h"

namespace UltraEd
{
    Camera::Camera() :
        m_fov(60.0f)
    {
        Import("assets/camera.fbx");
        m_type = ActorType::Camera;
    }

    Camera::Camera(const Camera &camera)
    {
        *this = camera;
        ResetId();
    }

    void Camera::Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack)
    {
        auto *buffer = m_vertexBuffer->GetBuffer(device, GetVertices());

        if (buffer != NULL)
        {
            stack->Push();
            stack->MultMatrixLocal(&GetMatrix());

            device->SetTransform(D3DTS_WORLD, stack->GetTop());
            device->SetStreamSource(0, buffer, 0, sizeof(Vertex));
            device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, GetVertices().size() / 3);

            stack->Pop();
        }

        Actor::Render(device, stack);
    }

    cJSON *Camera::Save()
    {
        auto state = Actor::Save();
        char buffer[LINE_FORMAT_LENGTH];
        sprintf(buffer, "%f", m_fov);
        cJSON_AddStringToObject(state, "fov", buffer);
        return state;
    }

    bool Camera::Load(cJSON *root)
    {
        Actor::Load(root);
        if (cJSON *fov = cJSON_GetObjectItem(root, "fov"))
        {
            sscanf(fov->valuestring, "%f", &m_fov);
        }
        return true;
    }
}
