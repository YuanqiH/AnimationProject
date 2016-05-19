#ifndef SGE_ENTITY_HPP
#define SGE_ENTITY_HPP

#include <vector>
#include <glm/glm.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Mesh.hpp"
#include "ModelImporter.hpp"
#include "ShaderManager.hpp"

class Entity
{
private:
	std::vector<Mesh*> meshes;
	glm::mat4 _modelMatrix;

	Shader* _shader;
		
public:
	Entity();
		
	bool loadFromFile(std::string file);
	void draw();
	glm::mat4 getModelMat();

	void rotate(glm::vec3 r);
	void setRotation(glm::vec3 r);

	void setShader(Shader* s);
	void setShader(std::string shader);
	Shader* getShader();
};

#endif
