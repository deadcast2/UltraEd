#pragma once

#include "Actor.h"

using namespace std;

namespace UltraEd
{
	struct GameObjectRelease
	{
		enum Value { AllResources, VertexBufferOnly };
	};

	class CGameObject : public CActor
	{
	public:
		CGameObject();
		CGameObject(const char *filePath);
		CGameObject(const CGameObject &gameObject);
		Savable Save();
		bool Load(IDirect3DDevice8 *device, cJSON *root);
		bool LoadTexture(IDirect3DDevice8 *device, const char *filePath);	
		void Release(GameObjectRelease::Value type);
		void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack);

	private:
		LPDIRECT3DTEXTURE8 m_texture;
		float m_collisionRadius;
	};
}
