#pragma once

#include "Model.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "animationCotroller.hpp"
#include <glm\gtx\quaternion.hpp>

enum State { idle = 0, run, attack, rid_idle }; // for different Anim

class Player
{
	private:

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
		static float riddingOffset;
		

	public:
		
		core::Model* local_model;
		std::vector<core::Model::Animation> local_animations; // extract from model for blending 

		playerTransform playerTrans;

		Player(core::Model* model, Camera* camera); // passing the model 
		Player(){};
		~Player(){};

		void update(double deltaTime, bool *keyState);

		void SetState(State newstate);
		int GetState(){
			return this->state;
		}
		void move(); // to-do change the state in this 
		void playerRender(); // called in the game loop
		bool doable, riddingOn; // for Npc
		bool runningOn,attackOn; // maybe use by NPC
};

