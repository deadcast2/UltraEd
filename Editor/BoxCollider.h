#ifndef _BOXCOLLIDER_H_
#define _BOXCOLLIDER_H_

#include <cJSON/cJSON.h>
#include "Collider.h"

using namespace std;

namespace UltraEd
{
    class BoxCollider : public Collider
    {
    public:
        BoxCollider();
        BoxCollider(const vector<Vertex> &vertices);
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

#endif
