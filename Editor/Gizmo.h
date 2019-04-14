#ifndef _GIZMO_H_
#define _GIZMO_H_

#include "Camera.h"
#include "GameObject.h"

namespace UltraEd
{
	enum GizmoState { XAxis, YAxis, ZAxis };

	enum GizmoModifierState { Translate, Scale, Rotate };

	class CGizmo
	{
	public:
		CGizmo();
		~CGizmo();
		void SetModifier(GizmoModifierState state);
		bool ToggleSpace(CGameObject *gameObject);
		bool ToggleSnapping();
		void Update(CCamera *camera, D3DXVECTOR3 orig, D3DXVECTOR3 dir, CGameObject *currentGameObject, CGameObject *selectedGameObject);
		void Reset();
		bool Select(D3DXVECTOR3 orig, D3DXVECTOR3 dir);
		void Release();
		void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack, CCamera *camera);

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
		CGameObject m_gameObjects[9];
		GizmoState m_state;
		GizmoModifierState m_modifierState;
		D3DXVECTOR3 m_updateStartPoint;
		D3DXVECTOR3 m_xAxisRot;
		D3DXVECTOR3 m_yAxisRot;
		D3DXVECTOR3 m_zAxisRot;
		bool m_worldSpaceToggled;
		bool snapToGridToggled;
	};
}

#endif
