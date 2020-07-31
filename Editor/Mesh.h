#ifndef _MESH_H_
#define _MESH_H_

#include <assimp/scene.h>
#include "FileIO.h"

namespace UltraEd
{
    class Mesh
    {
    public:
        Mesh(const char *filePath);
        std::vector<Vertex> GetVertices() { return m_vertices; }
        FileInfo GetFileInfo() { return m_info; }

    private:
        void InsertVerts(aiMatrix4x4 transform, aiMesh *mesh);
        void Process(aiNode *node, const aiScene *scene);
        std::vector<Vertex> m_vertices;
        FileInfo m_info;
    };
}

#endif
