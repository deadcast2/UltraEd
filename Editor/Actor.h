#pragma once

#include "deps/Assimp/include/assimp/Importer.hpp"
#include "deps/Assimp/include/assimp/scene.h"
#include "deps/Assimp/include/assimp/postprocess.h"
#include "deps/Assimp/include/assimp/cimport.h"
#include <vector>
#include "Vertex.h"
#include "Savable.h"

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
		virtual void Render(IDirect3DDevice8 *device, ID3DXMatrixStack *stack) = 0;
		D3DXMATRIX GetMatrix();
		D3DXMATRIX GetRotationMatrix();
		void SetLocalRotationMatrix(D3DXMATRIX mat);
		void Move(D3DXVECTOR3 position);
		void Scale(D3DXVECTOR3 position);
		void Rotate(FLOAT angle, D3DXVECTOR3 dir);
		GUID GetId();
		string GetName();
		void SetName(string name);
		ActorType::Value GetType();
		static ActorType::Value GetType(cJSON *item);
		void ResetId();
		D3DXVECTOR3 GetPosition();
		void SetPosition(D3DXVECTOR3 position);
		void SetRotation(D3DXVECTOR3 rotation);
		D3DXVECTOR3 GetScale();
		void SetScale(D3DXVECTOR3 scale);
		D3DXVECTOR3 GetRight();
		D3DXVECTOR3 GetForward();
		D3DXVECTOR3 GetUp();
		void GetAxisAngle(D3DXVECTOR3 *axis, float *angle);
		vector<Vertex> GetVertices();
		bool Pick(D3DXVECTOR3 orig, D3DXVECTOR3 dir, float *dist);
		void SetScript(string script);
		string GetScript();
		Savable Save();
		bool Load(IDirect3DDevice8 *device, cJSON *root);
		
	protected:
		void Import(const char *filePath);
		IDirect3DVertexBuffer8 *m_vertexBuffer;
		ActorType::Value m_type;
		IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);

	private:
		void InsertVerts(aiMatrix4x4 transform, aiMesh *mesh);
		void Process(aiNode *node, const aiScene *scene);
		bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
			D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, float *dist);

	private:
		GUID m_id;
		string m_name;
		vector<Vertex> m_vertices;
		D3DXVECTOR3 m_position;
		D3DXVECTOR3 m_scale;
		D3DXMATRIX m_localRot;
		D3DXMATRIX m_worldRot;
		string m_script;
	};
}
