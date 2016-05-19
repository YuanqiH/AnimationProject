#ifndef SGE_MODEL_IMPORTER_HPP
#define SGE_MODEL_IMPORTER_HPP

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <vector>

#include "Mesh.hpp"

class ModelImporter
{
const aiScene* model;
std::vector<Mesh*> meshes;
	
private:
	void printModelInfo();
	void extractTriangles();
		
public:
	ModelImporter();
		
	bool importModel(std::string file);
	std::vector<Mesh*> getMeshes();
};

#endif
