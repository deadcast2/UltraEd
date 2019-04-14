#pragma once

#include <vector>
#include "Vertex.h"

using namespace std;

namespace UltraEd
{
	class CGrid
	{
	public:
		CGrid();
		~CGrid();
		void Release();
		void Render(IDirect3DDevice8 *device);

	private:
		D3DMATERIAL8 m_material;
		IDirect3DVertexBuffer8 *m_vertexBuffer;
		IDirect3DVertexBuffer8 *GetBuffer(IDirect3DDevice8 *device);
		vector<Vertex> m_vertices;
	};
}
