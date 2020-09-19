#include "Camera.h"
#include "FileIO.h"

namespace UltraEd
{
    Camera::Camera() :
        m_fov(60.0f)
    {
        Import("assets/camera.dae");
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
            device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
            device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, static_cast<UINT>(GetVertices().size() / 3));

            stack->Pop();
        }

        Actor::Render(device, stack);
    }

    nlohmann::json Camera::Save()
    {
        auto actor = Actor::Save();
        actor.update({
            { "fov", m_fov }
        });
        return actor;
    }

    void Camera::Load(const nlohmann::json &root)
    {
        Actor::Load(root);
        m_fov = root["fov"];
    }
}
