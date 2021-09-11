#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Actor.h"

namespace UltraEd
{
    class Camera : public Actor
    {
    public:
        Camera();
        Camera(const Camera &camera);
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        float GetFOV() { return m_fov; }
        void SetFOV(float fov) { Dirty([&] { m_fov = fov; }, &m_fov); }
        nlohmann::json Save();
        void Load(const nlohmann::json &root);

    private:
        float m_fov;
    };
}

#endif
