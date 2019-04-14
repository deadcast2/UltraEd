#pragma once

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
	class CDebug
	{
	public:
		CDebug();
		~CDebug();
		void Release();
		void Render(IDirect3DDevice8 *device);
		static CDebug &Instance();
		static void DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to);
		static void Log(const char *format, ...);

	private:
		static CDebug *m_instance;
		D3DMATERIAL8 m_material;
		IDirect3DVertexBuffer8 *m_vertexBuffer;
		IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
		vector<Vertex> m_vertices;
		void _DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to);
	};
}
