#include "ModelImporter.hpp"

ModelImporter::ModelImporter()
{
}
	
bool ModelImporter::importModel(std::string file)
{
	std::cout << " Loading model: " << file << std::endl;

	Assimp::Importer importer;
		
	model = importer.ReadFile(file,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipWindingOrder
	);
		
	if(!model)
	{
		std::cout << "Error importing '" << file
			<< "': " << importer.GetErrorString() << std::endl;
		return false;
	}
		
	this->printModelInfo();
		
	/* Create the VAOs for the model */
	this->extractTriangles();
		
	return true;
}
	
void ModelImporter::printModelInfo()
{
		
	std::cout << " |-Materials: " << model->mNumMaterials << std::endl;
	std::cout << " |-Textures: " << model->mNumTextures << std::endl;
	std::cout << " |-Cameras: " << model->mNumCameras << std::endl;
	std::cout << " |-Animations: " << model->mNumAnimations << std::endl;
	std::cout;
}
	
void ModelImporter::extractTriangles()
{
	std::cout << " |-Meshes: " << model->mNumMeshes << std::endl;
	for(int i = 0; i < model->mNumMeshes; ++i)
	{
		aiMesh* mesh = model->mMeshes[i];
			
		std::cout << "   |-Mesh " << i << std::endl;
		std::cout << "     |-Verts: " << mesh->mNumVertices << std::endl;

		GLfloat* meshVertexData = new GLfloat[mesh->mNumVertices * 3];
		GLfloat* meshNormalsData = new GLfloat[mesh->mNumVertices * 3];
			
		/* Direct copy VBO from mesh */
		for(int j = 0; j < mesh->mNumVertices; ++j)
		{
			meshVertexData[(j * 3) + 0] = mesh->mVertices[j].x;
			meshVertexData[(j * 3) + 1] = mesh->mVertices[j].z;
			meshVertexData[(j * 3) + 2] = mesh->mVertices[j].y;
				

			meshNormalsData[(j * 3) + 0] = mesh->mNormals[j].x;
			meshNormalsData[(j * 3) + 1] = mesh->mNormals[j].z;
			meshNormalsData[(j * 3) + 2] = mesh->mNormals[j].y;
		}
			
		/* Extract the VBI from faces */
		std::cout << "     |-Polys: " << mesh->mNumFaces << std::endl;
		unsigned int* meshIndexData = new unsigned int[mesh->mNumFaces * 3];
		for(int j = 0; j < mesh->mNumFaces; ++j)
		{
			meshIndexData[(j * 3) + 0] = mesh->mFaces[j].mIndices[0];
			meshIndexData[(j * 3) + 1] = mesh->mFaces[j].mIndices[1];
			meshIndexData[(j * 3) + 2] = mesh->mFaces[j].mIndices[2];
		}
			
		/* Create a local mesh */
		Mesh* resultMesh = new Mesh();
		resultMesh->setVBOData(meshVertexData, mesh->mNumVertices);
		resultMesh->setNBOData(meshNormalsData, mesh->mNumVertices);
		resultMesh->setIBOData(meshIndexData, mesh->mNumFaces);
		meshes.push_back(resultMesh);
	}
}
	
std::vector<Mesh*> ModelImporter::getMeshes()
{
	return this->meshes;
}