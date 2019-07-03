#pragma once

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
    struct ColliderType
    {
        enum Value { Sphere };
    };

    class CCollider
    {
    public:
        CCollider(vector<Vertex> &vertices);
        void Release();
        void Render(IDirect3DDevice8 *device);
        void Compute(vector<Vertex> &vertices);
        D3DXVECTOR3 GetCenter() { return m_center; }
        FLOAT GetRadius() { return m_radius; }

    private:
        void DistantAABBPoints(int &min, int &max, vector<Vertex> vertices);
        void SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices);
        void AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, Vertex vertex);
        void FindCenterWithRadius(D3DXVECTOR3 &center, FLOAT &radius, vector<Vertex> vertices);

    private:
        D3DMATERIAL8 m_material;
        IDirect3DVertexBuffer8 *m_vertexBuffer;
        IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
        vector<Vertex> m_vertices;
        ColliderType::Value m_type;
        D3DXVECTOR3 m_center;
        FLOAT m_radius;
    };
}
