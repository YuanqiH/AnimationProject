#pragma once

#include "Model_loader.h"
#include "Camera.hpp"
#include <glm\gtx\quaternion.hpp>

class Object
{
public:

	Object(Model* local_model, Camera* local_camera, Shader* local_shader);
	~Object(){};

	void setModleMatrix(glm::mat4);
	void Update(float deltaTime);
	void Render();
	
	bool isPicked;
	static float objectRadius;
private:
	Model* local_model;
	Camera* local_camera;
	Shader* local_shader;
	glm::mat4 ModelMatrix;
};

