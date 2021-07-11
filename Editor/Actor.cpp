#include "Actor.h"
#include "Converters.h"
#include "FileIO.h"
#include "Util.h"
#include "Mesh.h"
#include "BoxCollider.h"
#include "SphereCollider.h"

namespace UltraEd
{
    Actor::Actor() :
        m_vertexBuffer(std::make_shared<VertexBuffer>()),
        m_type(ActorType::Model),
        m_id(),
        m_name(),
        m_vertices(),
        m_material(),
        m_position(0, 0, 0),
        m_scale(1, 1, 1),
        m_localRot(),
        m_worldRot(),
        m_eulerAngles(0, 0, 0),
        m_script(),
        m_collider(),
        m_parent(),
        m_children()
    {
        ResetId();
        m_script = std::string("void $start()\n{\n\n}\n\nvoid $update()\n{\n\n}\n\nvoid $input(NUContData gamepads[4])\n{\n\n}");
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
        if (m_collider)
        {
            stack->Push();
            {
                const auto worldPosition = GetPosition();
                stack->TranslateLocal(worldPosition.x, worldPosition.y, worldPosition.z);
                m_collider->Render(device, stack);
            }
            stack->Pop();
        }

        device->SetMaterial(&m_material);
    }

    void Actor::Import(const char *filePath)
    {
        Mesh mesh(filePath);
        m_vertices = mesh.GetVertices();
    }

    void Actor::OnChanged()
    {
        if (HasCollider())
        {
            GetCollider()->Update(GetMatrix());
        }

        for (const auto &child : m_children)
        {
            if (child.second != nullptr)
            {
                child.second->OnChanged();
            }
        }
    }

    D3DXMATRIX Actor::GetMatrix()
    {
        D3DXMATRIX translation;
        D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);

        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);

        D3DXMATRIX mat = scale * m_worldRot * m_localRot * translation;

        if (GetParent() != nullptr)
        {
            return mat * GetParent()->GetMatrix();
        }

        return mat;
    }

    const D3DXMATRIX Actor::GetRotationMatrix(bool worldSpace)
    {
        if (worldSpace && GetParent() != nullptr)
        {
            return m_worldRot * GetParent()->GetRotationMatrix();
        }

        return m_worldRot;
    }

    void Actor::SetRotationMatrix(const D3DXMATRIX &mat, bool worldSpace)
    {
        if (worldSpace)
        {
            m_worldRot = mat;
            m_eulerAngles = Util::ToEuler(mat);
        }
        else
        {
            m_localRot = mat;
        }
    }

    bool Actor::SetRotation(const D3DXVECTOR3 &eulerAngles)
    {
        m_eulerAngles = eulerAngles;

        return Dirty([&]() {
            D3DXMatrixRotationYawPitchRoll(&m_worldRot, D3DXToRadian(eulerAngles.y),
                D3DXToRadian(eulerAngles.x), D3DXToRadian(eulerAngles.z));
        }, &m_worldRot);
    }

    const D3DXVECTOR3 &Actor::GetEulerAngles()
    {
        return m_eulerAngles;
    }

    bool Actor::Rotate(const float &angle, const D3DXVECTOR3 &dir)
    {
        D3DXVECTOR3 scaled;
        D3DXVec3Scale(&scaled, &dir, D3DXToDegree(angle));
        m_eulerAngles += scaled;

        D3DXMATRIX newWorld;
        D3DXMatrixRotationAxis(&newWorld, &dir, angle);

        return Dirty([&] { m_worldRot *= newWorld; }, &m_worldRot);
    }

    const D3DXVECTOR3 Actor::GetPosition(bool worldSpace)
    {
        if (worldSpace && GetParent() != nullptr)
        {
            D3DXVECTOR3 newPosition;
            D3DXVec3TransformCoord(&newPosition, &m_position, &GetParent()->GetMatrix());
            return newPosition;
        }

        return m_position;
    }

    const D3DXMATRIX Actor::GetPositionMatrix(bool worldSpace)
    {
        D3DXMATRIX translation;
        D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);

        if (worldSpace && GetParent() != nullptr)
        {
            return translation * GetParent()->GetPositionMatrix();
        }

        return translation;
    }

    const D3DXVECTOR3 Actor::GetScale()
    {
        return m_scale;
    }

    const D3DXMATRIX Actor::GetScaleMatrix(bool worldSpace)
    {
        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);

        if (worldSpace && GetParent() != nullptr)
        {
            return scale * GetParent()->GetScaleMatrix();
        }

        return scale;
    }

    bool Actor::SetScale(const D3DXVECTOR3 &scale) 
    {
        if (m_type != ActorType::Model) 
            return false;
        
        return Dirty([&] { m_scale = scale; }, &m_scale);
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
        D3DXQuaternionRotationMatrix(&quat, &GetRotationMatrix(false));
        D3DXQuaternionToAxisAngle(&quat, axis, angle);
    }

    std::vector<Vertex> Actor::GetVertices(bool worldSpace) 
    {
        if (worldSpace)
        {
            std::vector<Vertex> transformedVertices(m_vertices);
            auto matrix = GetMatrix();

            for (unsigned int j = 0; j < m_vertices.size() / 3; j++)
            {
                D3DXVECTOR3 v0 = m_vertices[3 * j + 0].position;
                D3DXVECTOR3 v1 = m_vertices[3 * j + 1].position;
                D3DXVECTOR3 v2 = m_vertices[3 * j + 2].position;

                transformedVertices[3 * j + 0].position = *D3DXVec3TransformCoord(&v0, &v0, &matrix);
                transformedVertices[3 * j + 1].position = *D3DXVec3TransformCoord(&v1, &v1, &matrix);
                transformedVertices[3 * j + 2].position = *D3DXVec3TransformCoord(&v2, &v2, &matrix);
            }

            return transformedVertices;
        }

        return m_vertices;
    }

    bool Actor::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *dist)
    {
        auto transformedVertices = GetVertices(true);

        // Test all faces in this actor.
        for (unsigned int j = 0; j < transformedVertices.size() / 3; j++)
        {
            D3DXVECTOR3 v0 = transformedVertices[3 * j + 0].position;
            D3DXVECTOR3 v1 = transformedVertices[3 * j + 1].position;
            D3DXVECTOR3 v2 = transformedVertices[3 * j + 2].position;

            // Check if the pick ray passes through this point.
            if (Util::IntersectTriangle(orig, dir, v0, v1, v2, dist))
            {
                return true;
            }
        }

        return false;
    }

    void Actor::SetCollider(Collider *collider)
    {
        Dirty([&] { m_collider = std::shared_ptr<Collider>(collider); }, &m_collider);
    }

    void Actor::SetParent(Actor *actor, bool applyTransformations)
    {
        Unparent();

        if (actor == nullptr) return;

        if (applyTransformations)
        {
            D3DXMATRIX scaleInverse;
            D3DXMatrixInverse(&scaleInverse, NULL, &actor->GetScaleMatrix());
            D3DXMATRIX rotationInverse;
            D3DXMatrixInverse(&rotationInverse, NULL, &actor->GetRotationMatrix());
            D3DXMATRIX matrixInverse;
            D3DXMatrixInverse(&matrixInverse, NULL, &actor->GetMatrix());

            D3DXVec3TransformCoord(&m_scale, &m_scale, &scaleInverse);
            SetRotationMatrix(m_worldRot * rotationInverse);
            D3DXVec3TransformCoord(&m_position, &m_position, &matrixInverse);
        }

        actor->m_children[GetId()] = this;

        Dirty([&] { m_parent = actor; }, &m_parent);
    }

    void Actor::Unparent()
    {
        if (m_parent == nullptr) return;

        SetRotationMatrix(m_worldRot * m_parent->GetRotationMatrix());
        SetPosition(GetPosition());
        D3DXVec3TransformCoord(&m_scale, &m_scale, &m_parent->GetScaleMatrix());

        m_parent->m_children.erase(GetId());

        Dirty([&] { m_parent = nullptr; }, &m_parent);
    }

    void Actor::LinkChildren(Scene *scene, bool link, bool applyTransformations)
    {
        if (scene == nullptr) return;

        // Iterate using a copy since the underlying collection changes.
        std::map<boost::uuids::uuid, Actor *> childrenCopy(m_children);

        for (const auto &child : childrenCopy)
        {
            auto actor = scene->GetActor(child.first);
            if (actor != nullptr)
            {
                if (link)
                    actor->SetParent(this, applyTransformations);
                else
                    actor->Unparent();

                actor->SetDirty(false);
            }
        }
    }

    nlohmann::json Actor::Save()
    {
        json actor = {
            { "id", m_id },
            { "name", m_name },
            { "type", m_type },
            { "position", m_position },
            { "scale", m_scale },
            { "script", m_script },
            { "rotation", m_worldRot },
            { "euler_angles", m_eulerAngles },
        };

        for (const auto &pair : m_children)
        {
            actor["children"].push_back(pair.first);
        }

        if (m_collider)
        {
            actor["collider"] = m_collider->Save();
        }

        SetDirty(false);

        return actor;
    }

    void Actor::Load(const nlohmann::json &root)
    {
        m_id = root["id"];
        m_name = root["name"];
        m_type = root["type"];
        m_position = root["position"];
        m_scale = root["scale"];
        m_script = root["script"];
        m_worldRot = root["rotation"];
        m_eulerAngles = root["euler_angles"];

        if (root.contains("children"))
        {
            for (const auto &child : root["children"])
            {
                // Don't overwrite once set in case of another reload.
                if (m_children.find(child) != m_children.cend())
                    continue;

                m_children[child] = nullptr;
            }
        }

        SetCollider(nullptr);

        if (root.contains("collider"))
        {
            switch (root["collider"]["type"].get<ColliderType>())
            {
                case ColliderType::Box:
                    SetCollider(new BoxCollider());
                    m_collider->Load(root["collider"]);
                    break;
                case ColliderType::Sphere:
                    SetCollider(new SphereCollider());
                    m_collider->Load(root["collider"]);
                    break;
            }
        }

        SetDirty(false);
    }
}
