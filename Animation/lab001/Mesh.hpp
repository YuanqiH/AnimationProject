#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <string.h>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render();

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    //bool InitMaterials(const aiScene* pScene, const std::string& Filename);
   /// void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        //MeshEntry();

        bool Init(const std::vector<float> & Vertices, const std::vector<float> & Indices);

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };


    std::vector<MeshEntry> m_Entries;
    std::vector<int> m_Textures;
};
