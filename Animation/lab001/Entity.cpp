#include "Entity.hpp"

Entity::Entity()
{
	this->_modelMatrix = glm::mat4(1.0f);
}
	
bool Entity::loadFromFile(std::string file)
{
	ModelImporter* importer = new ModelImporter();
	importer->importModel(file);
	this->meshes = importer->getMeshes();
	return true;
}
	
void Entity::draw()
{
	/* If this entity has a shader, use it */
	if (this->_shader != NULL)
	{
		this->_shader->enableShader();
		this->_shader->setUniformMatrix4fv("modelMat", this->_modelMatrix);
	}

	for(int i = 0; i < this->meshes.size(); ++i)
	{
		this->meshes[i]->renderGL();
	}
}
	
glm::mat4 Entity::getModelMat()
{
	return this->_modelMatrix;
}

void Entity::setShader(Shader* s)
{
	this->_shader = s;
}

void Entity::setShader(std::string shader)
{
	Shader* s = ShaderManager::getShader(shader);
	if (s != NULL)
	{
		this->setShader(s);
	}
}

Shader* Entity::getShader()
{
	return this->_shader;
}

void Entity::rotate(glm::vec3 rotation)
{
	this->_modelMatrix = glm::rotate(this->_modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	this->_modelMatrix = glm::rotate(this->_modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	this->_modelMatrix = glm::rotate(this->_modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
}