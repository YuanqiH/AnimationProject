#pragma once

#include "loadTexture.h"

#include "Model.hpp" 
#include "Camera.hpp"
#include "player.hpp"
#include "Npc.hpp"
#include "Object.hpp"

class Level
{
public:
	Level(Camera* camera, Player*,  Object*, Object* );
	~Level(){};
	 void init();
	 	
	Shader planeShader;
	void Upate(float, bool* );
	void Render();

	Player* m_Player;
	Npc* m_Npc;
	Object* m_carpet;
	Object* m_mountain;

	glm::vec3 CubicInterpolate(glm::vec3 point0, glm::vec3 point1, glm::vec3 point2,glm::vec3 point3, float time);
private:
	Camera* m_camera;
	int evenNum;
	glm::vec3 endPoint;
	float Timer, freezeTimer;
	bool timerFreeze;
};

