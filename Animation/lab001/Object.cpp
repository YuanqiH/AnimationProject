

#include "Object.hpp"

float Object::objectRadius =  3.0f;

Object::Object(Model* local_model, Camera* local_camera, Shader* local_shader){
	this->local_camera = local_camera;
	this->local_model = local_model;
	this->local_shader = local_shader;
	this->isPicked = false;

}

void Object::setModleMatrix(glm::mat4 matrix){
	
	this->ModelMatrix = matrix;

}

void Object::Render(){

		local_shader->enableShader();
		glUniformMatrix4fv(glGetUniformLocation(local_shader->shaderProgramID, "projection"), 1, GL_FALSE, &local_camera->getProjectionMatrix()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(local_shader->shaderProgramID, "view"), 1, GL_FALSE, &local_camera->getViewMatrix()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(local_shader->shaderProgramID, "model"), 1, GL_FALSE, &this->ModelMatrix[0][0]);
		this->local_model->Draw(*local_shader);

}