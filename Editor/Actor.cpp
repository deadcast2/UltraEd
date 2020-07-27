#include "Actor.h"
#include "FileIO.h"
#include "Util.h"
#include "Mesh.h"
#include "BoxCollider.h"
#include "SphereCollider.h"

namespace UltraEd
{
    Actor::Actor() :
        m_vertexBuffer(make_shared<VertexBuffer>()),
        m_type(ActorType::Model),
        m_id(),
        m_name(),
        m_vertices(),
        m_material(),
        m_position(0, 0, 0),
        m_scale(1, 1, 1),
        m_localRot(),
        m_worldRot(),
        m_script(),
        m_collider()
    {
        ResetId();
        m_script = string("void $start()\n{\n\n}\n\nvoid $update()\n{\n\n}\n\nvoid $input(NUContData gamepads[4])\n{\n\n}");
        m_script.append("\n\nvoid $collide(actor *other)\n{\n\n}");
        SetDirty(true);

        D3DXMatrixIdentity(&m_localRot);
        D3DXMatrixIdentity(&m_worldRot);

        m_material.Diffuse.r = 1;
        m_material.Diffuse.g = 1;
        m_material.Diffuse.b = 1;
        m_material.Diffuse.a = 1;
    }

    void Actor::Release()
    {
        if (m_vertexBuffer != NULL)
        {
            m_vertexBuffer->Release();
        }

        if (m_collider != NULL)
        {
            m_collider->Release();
        }
    }

    void Actor::Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack)
    {
        if (m_collider) m_collider->Render(device);
        device->SetMaterial(&m_material);
    }

    void Actor::Import(const char *filePath)
    {
        Mesh mesh(filePath);
        m_vertices = mesh.GetVertices();
        if (mesh.GetFileInfo().type == FileType::User)
        {
            AddResource("vertexDataPath", mesh.GetFileInfo().path);
        }
    }

    GUID Actor::GetId(cJSON *item)
    {
        cJSON *id = cJSON_GetObjectItem(item, "id");
        return Util::StringToGuid(id->valuestring);
    }

    ActorType Actor::GetType(cJSON *item)
    {
        int typeValue;
        cJSON *type = cJSON_GetObjectItem(item, "type");
        sscanf(type->valuestring, "%i", &typeValue);
        return (ActorType)typeValue;
    }

    D3DXVECTOR3 Actor::GetRotation()
    {
        D3DXVECTOR3 rotation;
        D3DXMATRIX mat = GetRotationMatrix();

        rotation.x = asinf(-mat._32);

        if (cosf(rotation.x) > 0.0001f)
        {
            rotation.y = atan2f(mat._31, mat._33);
            rotation.z = atan2f(mat._12, mat._22);
        }
        else
        {
            rotation.y = 0.0f;
            rotation.z = atan2f(-mat._21, mat._11);
        }

        return D3DXVECTOR3(D3DXToDegree(rotation.x), D3DXToDegree(rotation.y), D3DXToDegree(rotation.z));
    }

    bool Actor::SetRotation(const D3DXVECTOR3 &rotation)
    {
        return Dirty([&]() {
            D3DXMatrixRotationYawPitchRoll(&m_worldRot, D3DXToRadian(rotation.y),
                D3DXToRadian(rotation.x), D3DXToRadian(rotation.z));
        }, &m_worldRot);
    }

    D3DXVECTOR3 Actor::GetRight()
    {
        D3DXVECTOR3 right = D3DXVECTOR3(1, 0, 0);
        D3DXVec3TransformCoord(&right, &right, &GetRotationMatrix());
        return right;
    }

    D3DXVECTOR3 Actor::GetForward()
    {
        D3DXVECTOR3 forward = D3DXVECTOR3(0, 0, 1);
        D3DXVec3TransformCoord(&forward, &forward, &GetRotationMatrix());
        return forward;
    }

    D3DXVECTOR3 Actor::GetUp()
    {
        D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
        D3DXVec3TransformCoord(&up, &up, &GetRotationMatrix());
        return up;
    }

    void Actor::GetAxisAngle(D3DXVECTOR3 *axis, float *angle)
    {
        D3DXQUATERNION quat;
        D3DXQuaternionRotationMatrix(&quat, &m_worldRot);
        D3DXQuaternionToAxisAngle(&quat, axis, angle);
    }

    bool Actor::Rotate(const float &angle, const D3DXVECTOR3 &dir)
    {
        D3DXMATRIX newWorld;
        D3DXMatrixRotationAxis(&newWorld, &dir, angle);
        return Dirty([&] { m_worldRot *= newWorld; }, &m_worldRot);
    }

    D3DXMATRIX Actor::GetMatrix()
    {
        D3DXMATRIX translation;
        D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);

        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);

        return scale * m_worldRot * m_localRot * translation;
    }

    bool Actor::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *dist)
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

    bool Actor::IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &v0,
        const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, float *dist)
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

    cJSON *Actor::Save()
    {
        char buffer[LINE_FORMAT_LENGTH];
        cJSON *actor = cJSON_CreateObject();

        cJSON_AddStringToObject(actor, "id", Util::GuidToString(m_id).c_str());

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
            cJSON_AddItemToObject(actor, "collider", m_collider->Save());
        }

        // Add array to hold all attached resources.
        cJSON *resourceArray = cJSON_CreateArray();
        cJSON_AddItemToObject(actor, "resources", resourceArray);
        for (const auto &resource : GetResources())
        {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, resource.first.c_str(), resource.second.c_str());
            cJSON_AddItemToArray(resourceArray, item);
        }

        SetDirty(false);

        return actor;
    }

    bool Actor::Load(cJSON *root)
    {
        float x, y, z, w;
        cJSON *name = cJSON_GetObjectItem(root, "name");
        cJSON *resources = cJSON_GetObjectItem(root, "resources");
        cJSON *resource = NULL;

        m_id = GetId(root);
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

        SetCollider(NULL);

        cJSON *collider = cJSON_GetObjectItem(root, "collider");
        if (collider)
        {
            switch (Collider::GetType(collider))
            {
                case ColliderType::Box:
                    SetCollider(new BoxCollider());
                    m_collider->Load(collider);
                    break;
                case ColliderType::Sphere:
                    SetCollider(new SphereCollider());
                    m_collider->Load(collider);
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

        SetDirty(false);

        return true;
    }
}
