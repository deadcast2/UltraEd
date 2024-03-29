#ifndef _COLLIDER_H_
#define _COLLIDER_H_

#include <memory>
#include <vector>
#include "Converters.h"
#include "Savable.h"
#include "VertexBuffer.h"

namespace UltraEd
{
    static const char* ColliderTypeNames[] { "TBox", "TSphere" };

    class Collider : public Savable
    {
    public:
        Collider();
        virtual ~Collider() { }
        void Release();
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        virtual void Build() = 0;
        virtual void Update(D3DXMATRIX &mat) = 0;
        D3DXVECTOR3 GetCenter() { return m_originalCenter; }
        nlohmann::json Save();
        void Load(const nlohmann::json &root);
        ColliderType GetType() { return m_type; };
        const char *GetName() { return ColliderTypeNames[static_cast<int>(m_type)]; }
        
    protected:
        void MinMaxAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const std::vector<Vertex> &vertices);
        void MostDistantAABBPoints(D3DXVECTOR3 &min, D3DXVECTOR3 &max, const std::vector<Vertex> &vertices);
        ColliderType m_type;
        std::vector<Vertex> m_vertices;
        D3DXVECTOR3 m_center;
        D3DXVECTOR3 m_originalCenter;

    private:
        D3DMATERIAL9 m_material;
        std::shared_ptr<VertexBuffer> m_vertexBuffer;
    };
}

#endif
