#include "View.h"

namespace UltraEd
{
    CView::CView() :
        m_step(0.005f)
    {
        Reset();
    }

    void CView::Reset()
    {
        m_type = ViewType::Perspective;
        m_right = D3DXVECTOR3(1, 0, 0);
        m_up = D3DXVECTOR3(0, 1, 0);
        m_forward = D3DXVECTOR3(0, 0, 1);
        m_pos = D3DXVECTOR3(0, 0, 0);
    }

    D3DXMATRIX CView::GetViewMatrix()
    {
        D3DXMATRIX view;

        // Keep view's axes orthogonal to each other:
        D3DXVec3Normalize(&m_forward, &m_forward);
        D3DXVec3Cross(&m_up, &m_forward, &m_right);
        D3DXVec3Normalize(&m_up, &m_up);
        D3DXVec3Cross(&m_right, &m_up, &m_forward);
        D3DXVec3Normalize(&m_right, &m_right);

        // Build the view matrix:
        float x = -D3DXVec3Dot(&m_right, &m_pos);
        float y = -D3DXVec3Dot(&m_up, &m_pos);
        float z = -D3DXVec3Dot(&m_forward, &m_pos);

        view(0, 0) = m_right.x;
        view(0, 1) = m_up.x;
        view(0, 2) = m_forward.x;
        view(0, 3) = 0.0f;
        view(1, 0) = m_right.y;
        view(1, 1) = m_up.y;
        view(1, 2) = m_forward.y;
        view(1, 3) = 0.0f;
        view(2, 0) = m_right.z;
        view(2, 1) = m_up.z;
        view(2, 2) = m_forward.z;
        view(2, 3) = 0.0f;
        view(3, 0) = x;
        view(3, 1) = y;
        view(3, 2) = z;
        view(3, 3) = 1.0f;

        return view;
    }

    void CView::Pitch(float angle)
    {
        D3DXMATRIX T;
        D3DXMatrixRotationAxis(&T, &m_right, angle);

        // rotate _up and _look around _right vector
        D3DXVec3TransformCoord(&m_up, &m_up, &T);
        D3DXVec3TransformCoord(&m_forward, &m_forward, &T);
    }

    void CView::Yaw(float angle)
    {
        D3DXMATRIX T;
        D3DXMatrixRotationY(&T, angle);

        // rotate _right and _look around _up or y-axis
        D3DXVec3TransformCoord(&m_right, &m_right, &T);
        D3DXVec3TransformCoord(&m_forward, &m_forward, &T);
    }

    void CView::Walk(float units)
    {
        // Limit only forward movement.
        if (CanWalk() || units < 0)
            m_pos += m_forward * units;
    }

    void CView::Strafe(float units)
    {
        m_pos += m_right * units;
    }

    void CView::Fly(float units)
    {
        m_pos += m_up * units;
    }

    D3DXVECTOR3 CView::GetPosition()
    {
        return m_pos;
    }

    void CView::SetPosition(D3DXVECTOR3 position)
    {
        m_pos = position;
    }

    D3DXVECTOR3 CView::GetForward()
    {
        return m_forward;
    }

    D3DXVECTOR3 CView::GetRight()
    {
        return m_right;
    }

    D3DXVECTOR3 CView::GetUp()
    {
        return m_up;
    }

    void CView::SetViewType(ViewType::Value type)
    {
        m_type = type;
    }

    ViewType::Value CView::GetType()
    {
        return m_type;
    }

    float CView::GetZoom()
    {
        switch (m_type)
        {
            case ViewType::Top:
                return m_pos.y;
            case ViewType::Left:
                return -m_pos.x;
            case ViewType::Front:
                return m_pos.z;
            default:
                return 0;
        }
    }

    bool CView::CanWalk()
    {
        // Perspective view does not need to be limited.
        return m_type == ViewType::Perspective || GetZoom() > m_step;
    }

    void CView::SingleStep(short delta)
    {
        Walk(delta * m_step);
    }

    cJSON *CView::Save()
    {
        char buffer[128];
        cJSON *view = cJSON_CreateObject();

        sprintf(buffer, "%f %f %f", m_pos.x, m_pos.y, m_pos.z);
        cJSON_AddStringToObject(view, "position", buffer);

        sprintf(buffer, "%f %f %f", m_forward.x, m_forward.y, m_forward.z);
        cJSON_AddStringToObject(view, "forward", buffer);

        sprintf(buffer, "%f %f %f", m_right.x, m_right.y, m_right.z);
        cJSON_AddStringToObject(view, "right", buffer);

        sprintf(buffer, "%f %f %f", m_up.x, m_up.y, m_up.z);
        cJSON_AddStringToObject(view, "up", buffer);

        return view;
    }

    bool CView::Load(cJSON *root)
    {
        float x, y, z;

        cJSON *position = cJSON_GetObjectItem(root, "position");
        sscanf(position->valuestring, "%f %f %f", &x, &y, &z);
        m_pos = D3DXVECTOR3(x, y, z);

        cJSON *forward = cJSON_GetObjectItem(root, "forward");
        sscanf(forward->valuestring, "%f %f %f", &x, &y, &z);
        m_forward = D3DXVECTOR3(x, y, z);

        cJSON *right = cJSON_GetObjectItem(root, "right");
        sscanf(right->valuestring, "%f %f %f", &x, &y, &z);
        m_right = D3DXVECTOR3(x, y, z);

        cJSON *up = cJSON_GetObjectItem(root, "up");
        sscanf(up->valuestring, "%f %f %f", &x, &y, &z);
        m_up = D3DXVECTOR3(x, y, z);

        return true;
    }
}
