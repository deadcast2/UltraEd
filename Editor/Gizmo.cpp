#include "Common.h"
#include "Gizmo.h"
#include "Debug.h"

namespace UltraEd
{
    CGizmo::CGizmo() :
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

    void CGizmo::SetModifier(GizmoModifierState::Value state)
    {
        m_modifierState = state;
    }

    string CGizmo::GetModifierName()
    {
        switch (m_modifierState)
        {
            case GizmoModifierState::Translate: return "Translate";
            case GizmoModifierState::Rotate: return "Rotate";
            case GizmoModifierState::Scale: return "Scale";
            default: return "Unknown";
        }
    }

    D3DXVECTOR3 CGizmo::GetModifyVector()
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

    void CGizmo::Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack, CView *view)
    {
        // Scale the size of the gizmo based on the view distance.
        D3DXVECTOR3 distance = m_models[0].GetPosition() - view->GetPosition();
        float scaleFactor = view->GetType() == ViewType::Perspective ? 0.2f : 0.1f;
        float length = D3DXVec3Length(&distance) * scaleFactor;
        SetScale(D3DXVECTOR3(length, length, length));

        // Render all gizmo handles.
        device->SetMaterial(&m_redMaterial);
        m_models[m_modifierState * 3 + 0].Render(device, stack);

        device->SetMaterial(&m_greenMaterial);
        m_models[m_modifierState * 3 + 1].Render(device, stack);

        device->SetMaterial(&m_blueMaterial);
        m_models[m_modifierState * 3 + 2].Render(device, stack);
    }

    void CGizmo::Release()
    {
        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].Release(ModelRelease::AllResources);
        }
    }

    bool CGizmo::Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir)
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

    void CGizmo::SetPosition(D3DXVECTOR3 position)
    {
        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].SetPosition(position);
        }
    }

    void CGizmo::SetScale(D3DXVECTOR3 scale)
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

    void CGizmo::SetupMaterials()
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

    void CGizmo::SetupTransHandles()
    {
        m_models[0] = CModel("Assets/trans-gizmo.fbx");
        m_models[0].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[1] = CModel("Assets/trans-gizmo.fbx");
        m_models[2] = CModel("Assets/trans-gizmo.fbx");
        m_models[2].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void CGizmo::SetupScaleHandles()
    {
        m_models[3] = CModel("Assets/scale-gizmo.fbx");
        m_models[3].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[4] = CModel("Assets/scale-gizmo.fbx");
        m_models[5] = CModel("Assets/scale-gizmo.fbx");
        m_models[5].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void CGizmo::SetupRotateHandles()
    {
        m_models[6] = CModel("Assets/rot-gizmo.fbx");
        m_models[6].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[7] = CModel("Assets/rot-gizmo.fbx");
        m_models[8] = CModel("Assets/rot-gizmo.fbx");
        m_models[8].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void CGizmo::SetupSelectorHandles()
    {
        m_models[9] = CModel("Assets/selector-gizmo.fbx");
        m_models[9].Rotate(-D3DX_PI / 2, D3DXVECTOR3(0, 0, 1));
        m_models[10] = CModel("Assets/selector-gizmo.fbx");
        m_models[11] = CModel("Assets/selector-gizmo.fbx");
        m_models[11].Rotate(D3DX_PI / 2, D3DXVECTOR3(1, 0, 0));
    }

    void CGizmo::Update(CActor *currentActor)
    {
        if (currentActor != NULL)
        {
            SetPosition(currentActor->GetPosition());

            if (!m_worldSpaceToggled)
            {
                // Keep gizmo in-sync with the actor's rotation.
                for (int i = 0; i < GIZMO_COUNT; i++)
                {
                    m_models[i].SetLocalRotationMatrix(currentActor->GetRotationMatrix());
                }
            }
        }
    }

    bool CGizmo::Update(CView *view, D3DXVECTOR3 orig, D3DXVECTOR3 dir, CActor *currentActor, CActor *selectedActor)
    {
        bool changeDetected = false;
        D3DXVECTOR3 targetDir = D3DXVECTOR3(0, 0, 0);
        D3DXVECTOR3 v0, v1, v2, intersectPoint;
        D3DXVECTOR3 look = selectedActor->GetPosition() - view->GetPosition();
        D3DXVec3Normalize(&look, &look);

        // Determine orientation for plane to produce depending on selected axis.
        if (m_axisState == GizmoAxisState::XAxis)
        {
            D3DXVECTOR3 right = m_worldSpaceToggled ? D3DXVECTOR3(1, 0, 0) : selectedActor->GetRight();
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
            D3DXVECTOR3 up = m_worldSpaceToggled ? D3DXVECTOR3(0, 1, 0) : selectedActor->GetUp();
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
            D3DXVECTOR3 forward = m_worldSpaceToggled ? D3DXVECTOR3(0, 0, 1) : selectedActor->GetForward();
            D3DXVECTOR3 up;
            D3DXVec3Cross(&up, &forward, &look);
            D3DXVec3Cross(&look, &forward, &up);

            v0 = selectedActor->GetPosition();
            v1 = v0 + forward;
            v2 = v0 + up;

            targetDir = forward;
        }

        D3DXPLANE testPlane;
        D3DXPlaneFromPoints(&testPlane, &v0, &v1, &v2);
        D3DXVECTOR3 rayEnd = orig + (dir * 1000);

        if (D3DXPlaneIntersectLine(&intersectPoint, &testPlane, &orig, &rayEnd) != NULL)
        {
            if (m_updateStartPoint == D3DXVECTOR3(-999, -999, -999))
            {
                m_updateStartPoint = intersectPoint;
            }

            D3DXVECTOR3 mouseDir = intersectPoint - m_updateStartPoint;
            D3DXVECTOR3 normMouseDir;
            D3DXVec3Normalize(&normMouseDir, &mouseDir);
            FLOAT moveDist = D3DXVec3Length(&mouseDir);
            const float epsilon = 0.1f;
            bool shouldSnap = fabs((moveDist > m_snapSize ? m_snapSize : moveDist) - m_snapSize) < epsilon;

            // Clamp the dot product between -1, 1 to not cause a undefined result.
            FLOAT dot = D3DXVec3Dot(&targetDir, &normMouseDir);
            dot = dot < -1.0f ? -1.0f : dot > 1.0f ? 1.0f : dot;
            FLOAT angle = acosf(dot);

            // Only allow movement when mouse following axis.
            FLOAT modifier = 1.0f - (angle / (D3DX_PI / 2));
            FLOAT sign = modifier < 0 ? -1.0f : 1.0f;

            if (m_modifierState == GizmoModifierState::Translate)
            {
                if (shouldSnap && m_snapToGridToggled)
                {
                    D3DXVECTOR3 newPos = currentActor->GetPosition();
                    newPos.x = snap(newPos.x * (1 / m_snapSize)) / (1 / m_snapSize);
                    newPos.y = snap(newPos.y * (1 / m_snapSize)) / (1 / m_snapSize);
                    newPos.z = snap(newPos.z * (1 / m_snapSize)) / (1 / m_snapSize);
                    currentActor->SetPosition(newPos + (targetDir * m_snapSize * sign));
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

            if (selectedActor == currentActor)
            {
                if (shouldSnap || !m_snapToGridToggled)
                {
                    m_updateStartPoint = intersectPoint;
                }
            }
        }

        Update(selectedActor);
        return changeDetected;
    }

    void CGizmo::Reset()
    {
        m_updateStartPoint = D3DXVECTOR3(-999, -999, -999);
    }

    bool CGizmo::ToggleSpace(CActor *actor)
    {
        m_worldSpaceToggled = !m_worldSpaceToggled;

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        D3DXMATRIX mat = m_worldSpaceToggled ? identity : actor->GetRotationMatrix();

        for (int i = 0; i < GIZMO_COUNT; i++)
        {
            m_models[i].SetLocalRotationMatrix(mat);
        }

        return m_worldSpaceToggled;
    }

    bool CGizmo::ToggleSnapping()
    {
        m_snapToGridToggled = !m_snapToGridToggled;
        return m_snapToGridToggled;
    }

    void CGizmo::SetSnapSize(float size)
    {
        m_snapSize = size;
    }

    float CGizmo::GetSnapSize()
    {
        return m_snapSize;
    }
}
