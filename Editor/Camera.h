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
        cJSON *Save();
        bool Load(cJSON *root);

    private:
        float m_fov;
    };
}

#endif
