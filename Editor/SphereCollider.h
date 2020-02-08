#pragma once

#include "Collider.h"

using namespace std;

namespace UltraEd
{
    class CSphereCollider : public CCollider
    {
    public:
        CSphereCollider();
        CSphereCollider(const vector<Vertex> &vertices);
        void Build();
        FLOAT GetRadius() { return m_radius; }
        cJSON *Save();
        bool Load(cJSON *root);

    private:
        void DistantAABBPoints(int &min, int &max, vector<Vertex> vertices);
        void SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices);
        void AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, Vertex vertex);
        void FindCenterWithRadius(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices);

    private:
        FLOAT m_radius;
    };
}
