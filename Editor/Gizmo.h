#pragma once

#include "View.h"
#include "Model.h"

namespace UltraEd
{
    struct GizmoAxisState
    {
        enum Value { XAxis, YAxis, ZAxis };
    };

    struct GizmoModifierState
    {
        enum Value { Translate, Scale, Rotate };
    };

    class CGizmo
    {
    public:
        CGizmo();
        void SetModifier(GizmoModifierState::Value state);
        bool ToggleSpace(CActor *actor);
        bool ToggleSnapping();
        void Update(CActor *currentActor);
        void Update(CView *view, D3DXVECTOR3 orig, D3DXVECTOR3 dir, CActor *currentActor, CActor *selectedActor);
        void Reset();
        bool Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir);
        void Release();
        void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack, CView *view);

    private:
        void SetPosition(D3DXVECTOR3 position);
        void SetScale(D3DXVECTOR3 scale);
        D3DXVECTOR3 GetModifyVector();
        void SetupMaterials();
        void SetupScaleHandles();
        void SetupTransHandles();
        void SetupRotateHandles();

    private:
        D3DMATERIAL8 m_redMaterial;
        D3DMATERIAL8 m_greenMaterial;
        D3DMATERIAL8 m_blueMaterial;
        CModel m_models[9];
        GizmoAxisState::Value m_axisState;
        GizmoModifierState::Value m_modifierState;
        D3DXVECTOR3 m_updateStartPoint;
        D3DXVECTOR3 m_xAxisRot;
        D3DXVECTOR3 m_yAxisRot;
        D3DXVECTOR3 m_zAxisRot;
        bool m_worldSpaceToggled;
        bool snapToGridToggled;
    };
}
