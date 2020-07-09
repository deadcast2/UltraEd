#ifndef _COLLIDER_H_
#define _COLLIDER_H_

#include <cJSON/cJSON.h>
#include <memory>
#include <vector>
#include "Savable.h"
#include "VertexBuffer.h"

using namespace std;

namespace UltraEd
{
    enum class ColliderType
    {
        Box, Sphere
    };

    static const char* ColliderTypeNames[] { "Box", "Sphere" };

    class Collider : public Savable
    {
    public:
        Collider();
        virtual ~Collider() { }
        void Release();
        void Render(IDirect3DDevice9 *device);
        virtual void Build() = 0;
        D3DXVECTOR3 GetCenter() { return m_center; }
        cJSON *Save();
        bool Load(cJSON *root);
        ColliderType GetType() { return m_type; };
        static ColliderType GetType(cJSON *item);
        const char *GetName() { return ColliderTypeNames[static_cast<int>(m_type)]; }
        
    protected:
        void DistantAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const vector<Vertex> &vertices);
        ColliderType m_type;
        vector<Vertex> m_vertices;
        D3DXVECTOR3 m_center;

    private:
        D3DMATERIAL9 m_material;
        shared_ptr<VertexBuffer> m_vertexBuffer;
    };
}

#endif
