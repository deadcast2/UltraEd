#include "Common.h"
#include "Gizmo.h"

namespace UltraEd
{
    Gizmo::Gizmo() :
        m_redMaterial(),
        m_greenMaterial(),
        m_blueMaterial(),
        m_models(),
        m_axisState(GizmoAxisState::XAxis),
        m_modifierState(GizmoModifierState::Translate),
        m_updateStartPoint(-999, -999, -999),
        m_worldSpaceToggled(true),
        m_snapToGridToggled(false),
        m_snapSize(0)
    {
        SetupMaterials();
        SetupTransHandles();
        SetupScaleHandles();
        SetupRotateHandles();
        SetupSelectorHandles();
        Reset();
    }

    void Gizmo::SetModifier(GizmoModifierState state)
    {
        m_modifierState = state;
    }

    std::string Gizmo::GetModifierName()
    {
        switch (m_modifierState)
        {
            case GizmoModifierState::Translate: return "Translate";
            case GizmoModifierState::Rotate: return "Rotate";
            case GizmoModifierState::Scale: return "Scale";
            default: return "Unknown";
        }
    }

    D3DXVECTOR3 Gizmo::GetModifyVector()
    {
        switch (m_axisState)
        {
            case GizmoAxisState::XAxis:
                return D3DXVECTOR3(1, 0, 0);
            case GizmoAxisState::YAxis:
                return D3DXVECTOR3(0, 1, 0);
            case GizmoAxisState::ZAxis:
                return D3DXVECTOR3(0, 0, 1);
        }

        return D3DXVECTOR3(0, 0, 0);
    }

    void Gizmo::Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack, View *view)
    {
        // Scale the size of the gizmo based on the view distance.
        D3DXVECTOR3 distance = m_models[0].GetPosition() - view->GetPosition();
        float scaleFactor = view->GetType() == ViewType::Perspective ? 0.2f : 0.1f;
        float length = D3DXVec3Length(&distance) * scaleFactor;
        SetScale(D3DXVECTOR3(length, length, length));

        // Render all gizmo handles.
        device->SetMaterial(&m_redMaterial);
        m_models[static_cast<int>(m_modifierState) * 3 + 0].Render(device, stack);

        device->SetMaterial(&m_greenMaterial);
        m_models[static_cast<int>(m_modifierState) * 3 + 1].Render(device, stack);

        device->SetMaterial(&m_blueMaterial);
        m_models[static_cast<int>(m_modifierState) * 3 + 2].Render(device, stack);
    }

    void Gizmo::Release()
    {
        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].Release();
        }
    }

    bool Gizmo::Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir)
    {
        float dist = 0;

        // Use the hidden selector gizmo handles to make selection easier.
        if (m_models[9].Pick(orig, dir, &dist))
        {
            m_axisState = GizmoAxisState::XAxis;
            return true;
        }
        else if (m_models[10].Pick(orig, dir, &dist))
        {
            m_axisState = GizmoAxisState::YAxis;
            return true;
        }
        else if (m_models[11].Pick(orig, dir, &dist))
        {
            m_axisState = GizmoAxisState::ZAxis;
            return true;
        }

        return false;
    }

    void Gizmo::SetPosition(D3DXVECTOR3 position)
    {
        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].SetPosition(position);
        }
    }

    void Gizmo::SetScale(D3DXVECTOR3 scale)
    {
        // Can't scale lower than one.
        scale.x = scale.x < 1 ? 1 : scale.x;
        scale.y = scale.y < 1 ? 1 : scale.y;
        scale.z = scale.z < 1 ? 1 : scale.z;

        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].SetScale(scale);
        }
    }

    void Gizmo::SetupMaterials()
    {
        // Create the red material.
        m_redMaterial.Emissive.r = 0.8f;
        m_redMaterial.Emissive.g = 0;
        m_redMaterial.Emissive.b = 0;

        // Create the green material.
        m_greenMaterial.Emissive.r = 0;
        m_greenMaterial.Emissive.g = 0.8f;
        m_greenMaterial.Emissive.b = 0;

        // Create the blue material.
        m_blueMaterial.Emissive.r = 0;
        m_blueMaterial.Emissive.g = 0;
        m_blueMaterial.Emissive.b = 0.8f;
    }

    void Gizmo::SetupTransHandles()
    {
        m_models[0] = Model("assets/trans-gizmo.fbx");
        m_models[0].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[1] = Model("assets/trans-gizmo.fbx");
        m_models[2] = Model("assets/trans-gizmo.fbx");
        m_models[2].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void Gizmo::SetupScaleHandles()
    {
        m_models[3] = Model("assets/scale-gizmo.fbx");
        m_models[3].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[4] = Model("assets/scale-gizmo.fbx");
        m_models[5] = Model("assets/scale-gizmo.fbx");
        m_models[5].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void Gizmo::SetupRotateHandles()
    {
        m_models[6] = Model("assets/rot-gizmo.fbx");
        m_models[6].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[7] = Model("assets/rot-gizmo.fbx");
        m_models[8] = Model("assets/rot-gizmo.fbx");
        m_models[8].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void Gizmo::SetupSelectorHandles()
    {
        m_models[9] = Model("assets/selector-gizmo.fbx");
        m_models[9].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[10] = Model("assets/selector-gizmo.fbx");
        m_models[11] = Model("assets/selector-gizmo.fbx");
        m_models[11].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void Gizmo::Update(Actor *currentActor)
    {
        if (currentActor != NULL)
        {
            D3DXMATRIX mat;
            D3DXMatrixIdentity(&mat);

            if (!m_worldSpaceToggled)
            {
                mat = currentActor->GetRotationMatrix();
            }

            // Keep gizmo in-sync with the actor's rotation.
            for (int i = 0; i < GIZMO_COUNT; i++)
            {
                m_models[i].SetRotationMatrix(mat, false);
            }

            SetPosition(currentActor->GetPosition());
        }
    }

    bool Gizmo::Update(View *view, D3DXVECTOR3 orig, D3DXVECTOR3 dir, Actor *currentActor, Actor *selectedActor)
    {
        bool changeDetected = false;
        D3DXVECTOR3 targetDir = D3DXVECTOR3(0, 0, 0);
        D3DXVECTOR3 v0, v1, v2, intersectPoint;
        D3DXVECTOR3 look = selectedActor->GetPosition() - view->GetPosition();
        D3DXVec3Normalize(&look, &look);

        // Determine orientation for plane to produce depending on selected axis.
        if (m_axisState == GizmoAxisState::XAxis)
        {
            const D3DXVECTOR3 right = m_worldSpaceToggled ? D3DXVECTOR3(1, 0, 0) : selectedActor->GetRight();
            D3DXVECTOR3 up;
            D3DXVec3Cross(&up, &right, &look);
            D3DXVec3Cross(&look, &right, &up);

            v0 = selectedActor->GetPosition();
            v1 = v0 + right;
            v2 = v0 + up;

            targetDir = right;
        }
        else if (m_axisState == GizmoAxisState::YAxis)
        {
            const D3DXVECTOR3 up = m_worldSpaceToggled ? D3DXVECTOR3(0, 1, 0) : selectedActor->GetUp();
            D3DXVECTOR3 right;
            D3DXVec3Cross(&right, &up, &look);
            D3DXVec3Cross(&look, &up, &right);

            v0 = selectedActor->GetPosition();
            v1 = v0 + right;
            v2 = v0 + up;

            targetDir = up;
        }
        else if (m_axisState == GizmoAxisState::ZAxis)
        {
            const D3DXVECTOR3 forward = m_worldSpaceToggled ? D3DXVECTOR3(0, 0, 1) : selectedActor->GetForward();
            D3DXVECTOR3 up;
            D3DXVec3Cross(&up, &forward, &look);
            D3DXVec3Cross(&look, &forward, &up);

            v0 = selectedActor->GetPosition();
            v1 = v0 + forward;
            v2 = v0 + up;

            targetDir = forward;
        }

        const D3DXVECTOR3 origTargetDir = targetDir;

        // Adjust the target direction to compensate for any parent rotations.
        if (selectedActor->GetParent() != nullptr)
        {
            D3DXMATRIX inverse;
            D3DXMatrixInverse(&inverse, NULL, &(selectedActor->GetParent()->GetScaleMatrix() * selectedActor->GetParent()->GetRotationMatrix()));
            D3DXVec3TransformCoord(&targetDir, &targetDir, &inverse);
        }

        D3DXPLANE testPlane;
        D3DXPlaneFromPoints(&testPlane, &v0, &v1, &v2);
        const D3DXVECTOR3 rayEnd = orig + (dir * 1000);

        if (D3DXPlaneIntersectLine(&intersectPoint, &testPlane, &orig, &rayEnd) != NULL)
        {
            if (m_updateStartPoint == D3DXVECTOR3(-999, -999, -999))
            {
                m_updateStartPoint = intersectPoint;
            }

            const D3DXVECTOR3 mouseDir = intersectPoint - m_updateStartPoint;
            D3DXVECTOR3 normMouseDir;
            D3DXVec3Normalize(&normMouseDir, &mouseDir);
            const float moveDist = D3DXVec3Length(&mouseDir);
            const float epsilon = 0.1f;
            const bool shouldSnap = fabsf((moveDist > m_snapSize ? m_snapSize : moveDist) - m_snapSize) < epsilon;

            // Clamp the dot product between -1, 1 to not cause a undefined result.
            float dot = D3DXVec3Dot(&origTargetDir, &normMouseDir);
            dot = dot < -1.0f ? -1.0f : dot > 1.0f ? 1.0f : dot;

            // Only allow movement when mouse following axis.
            const float modifier = 1.0f - (acosf(dot) / (D3DX_PI / 2));

            if (m_modifierState == GizmoModifierState::Translate)
            {
                if (shouldSnap && m_snapToGridToggled)
                {
                    const float sign = modifier < 0 ? -1.0f : 1.0f;
                    D3DXVECTOR3 newPos = currentActor->GetPosition();
                    
                    newPos.x = Util::Snap(newPos.x * (1 / m_snapSize)) / (1 / m_snapSize);
                    newPos.y = Util::Snap(newPos.y * (1 / m_snapSize)) / (1 / m_snapSize);
                    newPos.z = Util::Snap(newPos.z * (1 / m_snapSize)) / (1 / m_snapSize);
                    changeDetected = currentActor->SetPosition(newPos + (targetDir * m_snapSize * sign));
                }
                else if (!m_snapToGridToggled)
                {
                    changeDetected = currentActor->Move(targetDir * (moveDist * modifier));
                }
            }
            else if (m_modifierState == GizmoModifierState::Scale)
            {
                changeDetected = currentActor->Scale(targetDir * (moveDist * modifier));
            }
            else
            {
                changeDetected = currentActor->Rotate(moveDist * modifier, targetDir);
            }

            // Snapping only applies to translation.
            if (selectedActor == currentActor && (shouldSnap || !m_snapToGridToggled || m_modifierState != GizmoModifierState::Translate))
            {
                m_updateStartPoint = intersectPoint;
            }
        }

        Update(selectedActor);

        return changeDetected;
    }

    void Gizmo::Reset()
    {
        m_updateStartPoint = D3DXVECTOR3(-999, -999, -999);
    }

    bool Gizmo::ToggleSpace()
    {
        return m_worldSpaceToggled = !m_worldSpaceToggled;
    }

    bool Gizmo::ToggleSnapping()
    {
        return m_snapToGridToggled = !m_snapToGridToggled;
    }

    void Gizmo::SetSnapSize(float size)
    {
        m_snapSize = size;
    }

    float Gizmo::GetSnapSize()
    {
        return m_snapSize;
    }
}
