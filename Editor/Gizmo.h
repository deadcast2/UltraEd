#ifndef _GIZMO_H_
#define _GIZMO_H_

#include "View.h"
#include "Model.h"

namespace UltraEd
{
    class Gizmo
    {
    public:
        Gizmo();
        void SetModifier(GizmoModifierState state);
        GizmoModifierState GetModifier() { return m_modifierState; }
        std::string GetModifierName();
        bool ToggleSpace();
        bool ToggleSnapping();
        void SetSnapSize(float size);
        float GetSnapSize();
        void Update(Actor *currentActor);
        bool Update(View *view, D3DXVECTOR3 orig, D3DXVECTOR3 dir, Actor *currentActor, Actor *selectedActor);
        void Reset();
        bool Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir);
        void Release();
        void Render(IDirect3DDevice9 *device, ID3DXMatrixStack *stack, View *view);
        bool IsWorldSpace();
        bool IsSnapToGrid() { return m_snapToGridToggled; }

    private:
        void ScaleBasedOnView(View *view, IDirect3DDevice9 *device);
        void RenderHandles(IDirect3DDevice9 *device, ID3DXMatrixStack *stack);
        void SetPosition(D3DXVECTOR3 position);
        void SetScale(D3DXVECTOR3 scale);
        D3DXVECTOR3 GetModifyVector();
        void SetupMaterials();
        void SetupScaleHandles();
        void SetupTransHandles();
        void SetupRotateHandles();
        void SetupSelectorHandles();

    private:
        static const int GIZMO_COUNT = 12;
        D3DMATERIAL9 m_redMaterial;
        D3DMATERIAL9 m_greenMaterial;
        D3DMATERIAL9 m_blueMaterial;
        Model m_models[GIZMO_COUNT];
        GizmoAxisState m_axisState;
        GizmoModifierState m_modifierState;
        D3DXVECTOR3 m_updateStartPoint;
        bool m_worldSpaceToggled;
        bool m_snapToGridToggled;
        float m_snapSize;
    };
}

#endif
