#pragma once

#include "FileIO.h"
#include "deps/Assimp/include/assimp/scene.h"

namespace UltraEd
{
	class CMesh
	{
	public:
		CMesh(const char *filePath);
		vector<Vertex> GetVertices() { return m_vertices; }
		FileInfo GetFileInfo() { return m_info; }

	private:
		void InsertVerts(aiMatrix4x4 transform, aiMesh *mesh);
		void Process(aiNode *node, const aiScene *scene);
		vector<Vertex> m_vertices;
		FileInfo m_info;
	};
}
