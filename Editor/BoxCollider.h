#ifndef _BOXCOLLIDER_H_
#define _BOXCOLLIDER_H_

#include "Collider.h"

namespace UltraEd
{
    class BoxCollider : public Collider
    {
    public:
        BoxCollider();
        BoxCollider(const std::vector<Vertex> &vertices);
        void Build();
        const D3DXVECTOR3 &GetExtents() { return m_extents; }
        void Update(D3DXMATRIX &mat);
        nlohmann::json Save();
        void Load(const nlohmann::json &root);

    private:
        void BuildLine(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end);

    private:
        D3DXVECTOR3 m_extents;
        D3DXVECTOR3 m_originalExtents;
    };
}

#endif
