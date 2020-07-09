#ifndef _VIEW_H_
#define _VIEW_H_

#include <cJSON/cJSON.h>
#include <d3dx9.h>
#include "Savable.h"

namespace UltraEd
{
    enum class ViewType { Perspective, Top, Left, Front };

    class View : public Savable
    {
    public:
        View();
        cJSON *Save();
        bool Load(cJSON *root);
        D3DXVECTOR3 GetPosition();
        void SetPosition(D3DXVECTOR3 position);
        D3DXVECTOR3 GetForward();
        D3DXVECTOR3 GetRight();
        D3DXVECTOR3 GetUp();
        float GetZoom();
        D3DXMATRIX GetViewMatrix();
        void SetViewType(ViewType type);
        ViewType GetType();
        void Reset();
        void Fly(float units); // up/down
        void Strafe(float units); // left/right
        void Walk(float units); // forward/backward
        void Pitch(float angle); // rotate on right vector
        void Yaw(float angle); // rotate on up vector
        void SingleStep(short delta);

    private:
        bool CanWalk();
        ViewType m_type;
        D3DXVECTOR3 m_right;
        D3DXVECTOR3 m_up;
        D3DXVECTOR3 m_forward;
        D3DXVECTOR3 m_pos;
        const float m_step;
    };
}

#endif
