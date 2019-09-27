#include "Actor.h"
#include "FileIO.h"
#include "Util.h"
#include "Mesh.h"
#include "BoxCollider.h"
#include "SphereCollider.h"

namespace UltraEd
{
    CActor::CActor()
    {
        ResetId();
        m_type = ActorType::Model;
        m_vertexBuffer = 0;
        m_collider = 0;
        m_position = D3DXVECTOR3(0, 0, 0);
        m_scale = D3DXVECTOR3(1, 1, 1);
        m_script = string("void @start()\n{\n\n}\n\nvoid @update()\n{\n\n}\n\nvoid @input(NUContData gamepads[4])\n{\n\n}");
        m_script.append("\n\nvoid @collide(actor *other)\n{\n\n}");
        
        D3DXMatrixIdentity(&m_localRot);
        D3DXMatrixIdentity(&m_worldRot);

        ZeroMemory(&m_material, sizeof(D3DMATERIAL8));
        m_material.Diffuse.r = 1;
        m_material.Diffuse.g = 1;
        m_material.Diffuse.b = 1;
        m_material.Diffuse.a = 1;
    }

    void CActor::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
            m_vertexBuffer = 0;
        }

        if (m_collider != NULL)
        {
            m_collider->Release();
            m_collider = 0;
        }
    }

    void CActor::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack)
    {
        if (m_collider) m_collider->Render(device);
        device->SetMaterial(&m_material);
    }

    void CActor::Import(const char *filePath)
    {
        CMesh mesh(filePath);
        m_vertices = mesh.GetVertices();
        if (mesh.GetFileInfo().type == FileType::User)
        {
            resources["vertexDataPath"] = mesh.GetFileInfo().path;
        }
    }

    IDirect3DVertexBuffer8 *CActor::GetBuffer(IDirect3DDevice8 *device)
    {
        if (m_vertexBuffer == NULL)
        {
            if (FAILED(device->CreateVertexBuffer(
                m_vertices.size() * sizeof(Vertex),
                0,
                D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                D3DPOOL_DEFAULT,
                &m_vertexBuffer)))
            {
                return NULL;
            }

            VOID *pVertices;
            if (FAILED(m_vertexBuffer->Lock(0, m_vertices.size() * sizeof(Vertex),
                (BYTE**)&pVertices, 0)))
            {
                return NULL;
            }

            memcpy(pVertices, &m_vertices[0], m_vertices.size() * sizeof(Vertex));
            m_vertexBuffer->Unlock();
        }

        return m_vertexBuffer;
    }

    ActorType::Value CActor::GetType(cJSON *item)
    {
        int typeValue;
        cJSON *type = cJSON_GetObjectItem(item, "type");
        sscanf(type->valuestring, "%i", &typeValue);
        return (ActorType::Value)typeValue;
    }

    void CActor::SetRotation(D3DXVECTOR3 rotation)
    {
        D3DXMatrixRotationYawPitchRoll(&m_worldRot, rotation.y, rotation.x, rotation.z);
    }

    D3DXVECTOR3 CActor::GetRight()
    {
        D3DXVECTOR3 right = D3DXVECTOR3(1, 0, 0);
        D3DXVec3TransformCoord(&right, &right, &GetRotationMatrix());
        return right;
    }

    D3DXVECTOR3 CActor::GetForward()
    {
        D3DXVECTOR3 forward = D3DXVECTOR3(0, 0, 1);
        D3DXVec3TransformCoord(&forward, &forward, &GetRotationMatrix());
        return forward;
    }

    D3DXVECTOR3 CActor::GetUp()
    {
        D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
        D3DXVec3TransformCoord(&up, &up, &GetRotationMatrix());
        return up;
    }

    void CActor::GetAxisAngle(D3DXVECTOR3 *axis, float *angle)
    {
        D3DXQUATERNION quat;
        D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
        D3DXQuaternionToAxisAngle(&quat, axis, angle);
    }

    void CActor::Rotate(FLOAT angle, D3DXVECTOR3 dir)
    {
        D3DXMATRIX newWorld;
        D3DXMatrixRotationAxis(&newWorld, &dir, angle);
        m_worldRot *= newWorld;
    }

    D3DXMATRIX CActor::GetMatrix()
    {
        D3DXMATRIX translation;
        D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);

        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);

        return scale * m_worldRot * m_localRot * translation;
    }

    bool CActor::Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist)
    {
        // Test all faces in this actor.
        for (unsigned int j = 0; j < m_vertices.size() / 3; j++)
        {
            D3DXVECTOR3 v0 = m_vertices[3 * j + 0].position;
            D3DXVECTOR3 v1 = m_vertices[3 * j + 1].position;
            D3DXVECTOR3 v2 = m_vertices[3 * j + 2].position;

            // Transform the local vert positions based of the actor's
            // local matrix so when the actor is moved around we can still click it.
            D3DXVec3TransformCoord(&v0, &v0, &GetMatrix());
            D3DXVec3TransformCoord(&v1, &v1, &GetMatrix());
            D3DXVec3TransformCoord(&v2, &v2, &GetMatrix());

            // Check if the pick ray passes through this point.
            if (IntersectTriangle(orig, dir, v0, v1, v2, dist))
            {
                return true;
            }
        }

        return false;
    }

    bool CActor::IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, D3DXVECTOR3 &v0,
        D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, float *dist)
    {
        // Find vectors for two edges sharing vert0
        D3DXVECTOR3 edge1 = v1 - v0;
        D3DXVECTOR3 edge2 = v2 - v0;

        // Begin calculating determinant - also used to calculate U parameter.
        D3DXVECTOR3 pvec;
        D3DXVec3Cross(&pvec, &dir, &edge2);

        // If determinant is near zero, ray lies in plane of triangle.
        float det = D3DXVec3Dot(&edge1, &pvec);

        if (det < 0.0001f) return false;

        // Calculate U parameter and test bounds.
        D3DXVECTOR3 tvec = orig - v0;
        float u = D3DXVec3Dot(&tvec, &pvec);
        if (u < 0.0f || u > det) return false;

        // Prepare to test V parameter.
        D3DXVECTOR3 qvec;
        D3DXVec3Cross(&qvec, &tvec, &edge1);

        // Calculate V parameter and test bounds.
        float v = D3DXVec3Dot(&dir, &qvec);
        if (v < 0.0f || u + v > det) return false;

        *dist = D3DXVec3Dot(&edge2, &qvec) * (1.0f / det);

        return true;
    }

    Savable CActor::Save()
    {
        char buffer[LINE_FORMAT_LENGTH];
        cJSON *root = cJSON_CreateObject();
        cJSON *actor = cJSON_CreateObject();

        cJSON_AddItemToObject(root, "actor", actor);

        cJSON_AddStringToObject(actor, "id", CUtil::GuidToString(m_id).c_str());

        cJSON_AddStringToObject(actor, "name", m_name.c_str());

        sprintf(buffer, "%i", (int)m_type);
        cJSON_AddStringToObject(actor, "type", buffer);

        sprintf(buffer, "%f %f %f", m_position.x, m_position.y, m_position.z);
        cJSON_AddStringToObject(actor, "position", buffer);

        sprintf(buffer, "%f %f %f", m_scale.x, m_scale.y, m_scale.z);
        cJSON_AddStringToObject(actor, "scale", buffer);

        D3DXQUATERNION quat;
        D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
        sprintf(buffer, "%f %f %f %f", quat.x, quat.y, quat.z, quat.w);
        cJSON_AddStringToObject(actor, "rotation", buffer);

        cJSON_AddStringToObject(actor, "script", m_script.c_str());

        if (m_collider)
        {
            cJSON_AddItemToObject(actor, "collider", m_collider->Save().object);
        }

        Savable savable = { root, SavableType::Actor };
        return savable;
    }

    bool CActor::Load(IDirect3DDevice8 *device, cJSON *root)
    {
        float x, y, z, w;
        cJSON *id = cJSON_GetObjectItem(root, "id");
        cJSON *name = cJSON_GetObjectItem(root, "name");
        cJSON *resources = cJSON_GetObjectItem(root, "resources");
        cJSON *resource = NULL;

        m_id = CUtil::StringToGuid(id->valuestring);
        m_name = name->valuestring;
        m_type = GetType(root);

        cJSON *position = cJSON_GetObjectItem(root, "position");
        sscanf(position->valuestring, "%f %f %f", &x, &y, &z);
        m_position = D3DXVECTOR3(x, y, z);

        cJSON *scale = cJSON_GetObjectItem(root, "scale");
        sscanf(scale->valuestring, "%f %f %f", &x, &y, &z);
        m_scale = D3DXVECTOR3(x, y, z);

        cJSON *rotation = cJSON_GetObjectItem(root, "rotation");
        sscanf(rotation->valuestring, "%f %f %f %f", &x, &y, &z, &w);
        D3DXMatrixRotationQuaternion(&m_worldRot, &D3DXQUATERNION(x, y, z, w));

        cJSON *script = cJSON_GetObjectItem(root, "script");
        m_script = script->valuestring;

        cJSON *collider = cJSON_GetObjectItem(root, "collider");
        if (collider)
        {
            switch (CCollider::GetType(collider))
            {
                case ColliderType::Box:
                    SetCollider(new CBoxCollider());
                    m_collider->Load(device, collider);
                    break;
                case ColliderType::Sphere:
                    SetCollider(new CSphereCollider());
                    m_collider->Load(device, collider);
                    break;
            }
        }

        cJSON_ArrayForEach(resource, resources)
        {
            const char *path = resource->child->valuestring;
            if (strcmp(resource->child->string, "vertexDataPath") == 0)
            {
                Import(path);
            }
        }

        return true;
    }
}
