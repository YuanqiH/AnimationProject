#pragma once

#include "Model.hpp"
#include <queue>
#include "blender.hpp"
#include "AnimationEx.h"
#include <glm/gtx/compatibility.hpp>
//========================================
//purpose for make the blending between animations 





struct Pose // storing the temp position extracted from each time tick
{
	glm::vec3 translation;
	glm::quat orientation;
	float weight;
};


class animationController
{
public:

	static float animationSpeedScale;

	std::vector<core::Model::Animation>  in_animations; // hold all the animations
	std::vector<AnimationExtension> working_animations;
	
	animationController(){};
	~animationController();
	void init(std::vector<core::Model::Animation>  in_animations, std::map <std::string, unsigned int> local_boneID, core::Model::Animation::BoneNode* refer_skeleton);
	
	core::Model::Animation update(float deltaTime);
	Blender m_blender;
	void setAnimationInQ(int index, bool loop = true, float blendDuration = 0, TransitionType transition = TransitionType::Immediate);


private:
	//void getKeyframe(float timeInTicks, core::Model::Animation* animations, std::map<int, std::vector<Pose>>*  contributes);
	std::map <std::string, unsigned int> local_boneID;
	core::Model::Animation::BoneNode* refer_skeleton; // for get the keyframe

	void tickAllAnimationClock(float deltaTime);
	std::map<int, std::vector<Pose>>  getKeyframe();
	//std::queue<Command> commandQ; // for saving all the commond make by player

	void getResultAnimation(std::map<int, std::vector<Pose>> contributingPoses, glm::mat4 parentMatrix,  core::Model::Animation::BoneNode* refer_skele);

};



