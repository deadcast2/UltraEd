#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <cJSON/cJSON.h>
#include <vector>
#include "VertexBuffer.h"
#include "Savable.h"
#include "Util.h"
#include "Collider.h"

namespace UltraEd
{
    enum class ActorType
    {
        Model, Camera
    };

    class Actor : public Savable
    {
    public:
        Actor();
        virtual ~Actor() { }
        virtual void Release();
        virtual void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        const GUID &GetId() { return m_id; }
        void ResetId() { m_id = Util::NewGuid(); }
        const string &GetName() { return m_name; }
        void SetName(string name) { m_name = name; }
        const ActorType &GetType() { return m_type; }
        static GUID GetId(cJSON *item);
        static ActorType GetType(cJSON *item);
        D3DXMATRIX GetMatrix();
        const D3DXMATRIX &GetRotationMatrix() { return m_worldRot; }
        void SetLocalRotationMatrix(const D3DXMATRIX &mat) { m_localRot = mat; }
        bool Move(const D3DXVECTOR3 &position) { return Dirty([&] { m_position += position; }, &m_position); }
        bool Scale(const D3DXVECTOR3 &position) { return Dirty([&] { m_scale += position; }, &m_scale); }
        bool Rotate(const float &angle, const D3DXVECTOR3 &dir);
        const D3DXVECTOR3 &GetPosition() { return m_position; }
        void SetPosition(const D3DXVECTOR3 &position) { m_position = position; }
        void SetRotation(const D3DXVECTOR3 &rotation);
        const D3DXVECTOR3 &GetScale() { return m_scale; }
        void SetScale(const D3DXVECTOR3 &scale) { m_scale = scale; }
        D3DXVECTOR3 GetRight();
        D3DXVECTOR3 GetForward();
        D3DXVECTOR3 GetUp();
        void GetAxisAngle(D3DXVECTOR3 *axis, float *angle);
        const vector<Vertex> &GetVertices() { return m_vertices; }
        bool Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *dist);
        const string &GetScript() { return m_script; }
        void SetScript(const string &script) { Dirty([&] { m_script = script; }, &m_script); }
        Collider *GetCollider() { return m_collider.get(); }
        void SetCollider(Collider *collider) { Dirty([&] { m_collider = shared_ptr<Collider>(collider); }, &m_collider); }
        bool HasCollider() { return GetCollider() != NULL; }
        cJSON *Save();
        bool Load(cJSON *root);

    protected:
        shared_ptr<VertexBuffer> m_vertexBuffer;
        ActorType m_type;
        void Import(const char *filePath);

    private:
        GUID m_id;
        string m_name;
        vector<Vertex> m_vertices;
        D3DMATERIAL9 m_material;
        D3DXVECTOR3 m_position;
        D3DXVECTOR3 m_scale;
        D3DXMATRIX m_localRot;
        D3DXMATRIX m_worldRot;
        string m_script;
        bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
            const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, float *dist);
        shared_ptr<Collider> m_collider;
    };
}

#endif
