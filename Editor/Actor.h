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
        virtual void Release();
        virtual void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);
        GUID GetId() { return m_id; }
        void ResetId() { m_id = CUtil::NewGuid(); }
        string GetName() { return m_name; }
        void SetName(string name) { m_name = name; }
        ActorType::Value GetType() { return m_type; }
        static ActorType::Value GetType(cJSON *item);
        D3DXMATRIX GetMatrix();
        D3DXMATRIX GetRotationMatrix() { return m_worldRot; }
        void SetLocalRotationMatrix(D3DXMATRIX mat) { m_localRot = mat; }
        void Move(D3DXVECTOR3 position) { m_position += position; }
        void Scale(D3DXVECTOR3 position) { m_scale += position; }
        void Rotate(FLOAT angle, D3DXVECTOR3 dir);
        D3DXVECTOR3 GetPosition() { return m_position; }
        void SetPosition(D3DXVECTOR3 position) { m_position = position; }
        void SetRotation(D3DXVECTOR3 rotation);
        D3DXVECTOR3 GetScale() { return m_scale; }
        void SetScale(D3DXVECTOR3 scale) { m_scale = scale; }
        D3DXVECTOR3 GetRight();
        D3DXVECTOR3 GetForward();
        D3DXVECTOR3 GetUp();
        void GetAxisAngle(D3DXVECTOR3 *axis, float *angle);
        vector<Vertex> GetVertices() { return m_vertices; }
        bool Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist);
        string GetScript() { return m_script; }
        void SetScript(string script) { m_script = script; }
        CCollider *GetCollider() { return m_collider; }
        void SetCollider(CCollider *collider) { m_collider = collider; }
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
            D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, float *dist);
        CCollider *m_collider;
    };
}
