#pragma once

#include <vector>
#include "Vertex.h"
#include "Savable.h"
#include "Util.h"
#include "Collider.h"

namespace UltraEd
{
    struct ActorType
    {
        enum Value { Model, Camera };
    };

    class CActor : public CSavable
    {
    public:
        CActor();
        virtual ~CActor() { }
        virtual void Release();
        virtual void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);
        const GUID &GetId() { return m_id; }
        void ResetId() { m_id = CUtil::NewGuid(); }
        const string &GetName() { return m_name; }
        void SetName(string name) { m_name = name; }
        const ActorType::Value &GetType() { return m_type; }
        static ActorType::Value GetType(cJSON *item);
        D3DXMATRIX GetMatrix();
        const D3DXMATRIX &GetRotationMatrix() { return m_worldRot; }
        void SetLocalRotationMatrix(const D3DXMATRIX &mat) { m_localRot = mat; }
        void Move(const D3DXVECTOR3 &position) { Dirty([&] { m_position += position; }, &m_position); }
        void Scale(const D3DXVECTOR3 &position) { Dirty([&] { m_scale += position; }, &m_scale); }
        void Rotate(const float &angle, const D3DXVECTOR3 &dir);
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
        CCollider *GetCollider() { return m_collider.get(); }
        void SetCollider(CCollider *collider) { Dirty([&] { m_collider = shared_ptr<CCollider>(collider); }, &m_collider); }
        Savable Save();
        bool Load(IDirect3DDevice8 *device, cJSON *root);

    protected:
        IDirect3DVertexBuffer8 *m_vertexBuffer;
        ActorType::Value m_type;
        void Import(const char *filePath);
        IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);

    private:
        GUID m_id;
        string m_name;
        vector<Vertex> m_vertices;
        D3DMATERIAL8 m_material;
        D3DXVECTOR3 m_position;
        D3DXVECTOR3 m_scale;
        D3DXMATRIX m_localRot;
        D3DXMATRIX m_worldRot;
        string m_script;
        bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
            const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, float *dist);
        shared_ptr<CCollider> m_collider;
    };
}
