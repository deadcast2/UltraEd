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
        Savable Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);

    private:
        void BuildLine(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end);
        void DistantAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const vector<Vertex> &vertices);

    private:
        D3DXVECTOR3 m_extents;
    };
}
