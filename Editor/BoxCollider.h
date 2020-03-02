#pragma once

#include "Collider.h"

using namespace std;

namespace UltraEd
{
    class CBoxCollider : public CCollider
    {
    public:
        CBoxCollider();
        CBoxCollider(const vector<Vertex> &vertices);
        void Build();
        const D3DXVECTOR3 &GetExtents() { return m_extents; }
        cJSON *Save();
        bool Load(cJSON *root);

    private:
        void BuildLine(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end);

    private:
        D3DXVECTOR3 m_extents;
    };
}
