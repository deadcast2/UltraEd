#pragma once

#include <string>
#include "vendor/cJSON.h"
#include "Savable.h"
#include "deps/DXSDK/include/d3dx8.h"

namespace UltraEd
{
    struct ViewType
    {
        enum Value { Perspective, Top, Left, Front };
    };

    class CView : public CSavable
    {
    public:
        CView();
        Savable Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);
        D3DXVECTOR3 GetPosition();
        D3DXVECTOR3 GetForward();
        D3DXVECTOR3 GetRight();
        D3DXVECTOR3 GetUp();
        D3DXMATRIX GetViewMatrix();
        void SetViewType(ViewType::Value type);
        ViewType::Value GetType();
        void Reset();
        void Fly(float units); // up/down
        void Strafe(float units); // left/right
        void Walk(float units); // forward/backward
        void Pitch(float angle); // rotate on right vector
        void Yaw(float angle); // rotate on up vector

    private:
        D3DXVECTOR3 m_right;
        D3DXVECTOR3 m_up;
        D3DXVECTOR3 m_forward;
        D3DXVECTOR3 m_pos;
        ViewType::Value m_type;
    };
}
