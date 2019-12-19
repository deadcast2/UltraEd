#pragma once

#include <memory>
#include <vector>
#include "Savable.h"
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    struct ColliderType
    {
        enum Value { Box, Sphere };
    };

    static const char* ColliderTypeNames[] { "Box", "Sphere" };

    class CCollider : public CSavable
    {
    public:
        CCollider();
        virtual ~CCollider() { }
        void Release();
        void Render(IDirect3DDevice8 *device);
        virtual void Build() = 0;
        D3DXVECTOR3 GetCenter() { return m_center; }
        Savable Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);
        ColliderType::Value GetType() { return m_type; };
        static ColliderType::Value GetType(cJSON *item);
        const char *GetName() { return ColliderTypeNames[m_type]; }
        
    protected:
        ColliderType::Value m_type;
        vector<Vertex> m_vertices;
        D3DXVECTOR3 m_center;

    private:
        D3DMATERIAL8 m_material;
        shared_ptr<CVertexBuffer> m_vertexBuffer;
    };
}
