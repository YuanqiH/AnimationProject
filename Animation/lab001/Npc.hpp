#pragma once

#include "Model.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "player.hpp"
#include "animationCotroller.hpp"
#include <glm\gtx\quaternion.hpp>


namespace NpcName{enum State { idle = 0, ridd_idle ,ridded, redy };} // for different Anim

class Npc
{
public:
	
	core::Model* local_model;
	std::vector<core::Model::Animation> local_animations; // extract from model for blending 
	playerTransform playerTrans;

	Npc(core::Model* model, Camera* camera);
	Npc(){};
	~Npc(){};
	void setPlayer(Player* );
	void update(double deltaTime, bool *keyState);

	void SetState(NpcName::State newstate);
		int GetState(){
			return this->state;
		}
	void move(); // to-do change the state in this 
	void playerRender(); // called in the game loop
	void detect();

private:

		Player* player;
		static float raidus;
		
		int state;
		Shader playerShader;
		Camera *local_camera;
		core::Model::Animation::BoneNode* skeleton;
		animationController m_controller;
		core::Model::Animation renderAnimation;
		bool *keys;
		glm::mat4 getThirdPlaneModelMatrix();
		glm::mat4 getAdjustMatrix();
		glm::quat m_quaternionTotal;
		float rotz,rotx,roty;// for third person
		float chractorMoveSpeed, rotateSpeed;
		void processKeys(float);
		void setController(); 

};

