#ifndef _VIEW_H_
#define _VIEW_H_

#include <d3dx9.h>

namespace UltraEd
{
    enum class ViewType { Perspective, Top, Left, Front };

    class View
    {
    public:
        View();
        ViewType GetType();
        float GetZoom();
        D3DXMATRIX GetViewMatrix();
        void Reset();
        void SetViewType(ViewType type);
        void Fly(float units);
        void Strafe(float units);
        void Walk(float units);
        void Pitch(float angle);
        void Yaw(float angle);
        void SingleStep(short delta);

    private:
        bool CanWalk();

    private:
        ViewType m_type;
        D3DXVECTOR3 m_pos;
        D3DXVECTOR3 m_right;
        D3DXVECTOR3 m_up;
        D3DXVECTOR3 m_forward;
        const float m_step;
    };
}

#endif
