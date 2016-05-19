
#include"animationCotroller.hpp"


float animationController::animationSpeedScale = 1000.0f;
float Blender::blendScalar = 1000.0f;
bool Blender::frozen =  true;

void animationController::init(std::vector<core::Model::Animation>  in_animations, std::map <std::string, unsigned int> local_boneID, core::Model::Animation::BoneNode* refer_skeleton)
{
	this->in_animations =  in_animations;
	this->local_boneID =  local_boneID;
	this->refer_skeleton = refer_skeleton;
	for (int i = 0; i < in_animations.size(); i++)
	{
		AnimationExtension tempAni;
		tempAni.setAnimation( "",i,in_animations[i].duration);
		tempAni.animationDate = in_animations[i];
		this->working_animations.push_back(tempAni);
	}
	m_blender.initBlender();

}

animationController::~animationController()
{
}


core::Model::Animation animationController::update(float deltaTime){ // to do: need a local timmer

	m_blender.Update(deltaTime);

	tickAllAnimationClock(deltaTime);

	//getKeyframe(deltaTime, refer_skeleton, &in_animations[1], glm::mat4(1.0)); // update the anmiation 
	std::map<int, std::vector<Pose>> contributingPoses = getKeyframe();

	getResultAnimation(contributingPoses, glm::mat4(1),refer_skeleton);
	return in_animations[0];
}

//================================
// tweak the tick rate before need fixed it !!!!!!

void animationController::tickAllAnimationClock(float deltaTime){
	for (int aniIn = 0; aniIn < working_animations.size(); aniIn++)
	{
		AnimationExtension* animatoin = &working_animations[aniIn];
		if (!animatoin->frozen)
		{
			animatoin->localClock += (deltaTime/ 1000) * animationSpeedScale;
		}
		if (animatoin->localClock >= animatoin->duration)
		{
			if(!animatoin->loop)
			{
				animatoin->Stop();
			}
			animatoin->localClock = 0.0;
		}
	}


}

std::map<int, std::vector<Pose>>  animationController::getKeyframe(){//float timeInTicks, core::Model::Animation* animations, std::map<int, std::vector<Pose>>*  contributes){

	std::map<int, std::vector<Pose> >  contributes;
	for (int aniI = 0; aniI < working_animations.size(); aniI++) // checking all the animations to get the keyframe 
	{
		AnimationExtension* animation = &working_animations[aniI];
		if (animation->weight > 0 )
		{   
			//core::Model::Animation::BoneNode tempskeleton = refer_skeleton;
			core::Model::Animation* animations = &animation->animationDate;
			for(int chanIndex = 0; chanIndex < animations->channels.size() ; chanIndex++) // go through all the bones
			{


			//double animTime = std::fmod(timeInTicks, animations->duration * animations->ticksPerSecond );
			double animTime = animation->localClock;
		//	std::cout<< animTime << std::endl; //debug the time
			aiQuaternion aiRotation(animations->channels[chanIndex].mRotationKeys[0].mValue);
			aiVector3D aiTranslation(animations->channels[chanIndex].mPositionKeys[0].mValue);
			aiVector3D aiScale(animations->channels[chanIndex].mScalingKeys[0].mValue);
 
			Assimp::Interpolator<aiQuaternion> slerp;
			Assimp::Interpolator<aiVector3D> lerp;
 
			// using the keyframe period is not better than treating the period equally
	
			int keysize = animations->channels[chanIndex].mPositionKeys.size();
			 int key1, key2;
			 double lowerTimeTicker, upperTimeTicker;
			 double Factor;
			for (int i = 0; i < keysize-1; i++)
			{
				 lowerTimeTicker = animations->channels[chanIndex].mPositionKeys[i].mTime ;
				 upperTimeTicker = animations->channels[chanIndex].mPositionKeys[i+1].mTime;
				if (animTime > lowerTimeTicker && animTime <= upperTimeTicker)
				{
					key1 = i;
					key2 = i+1;
					break;
				}
				if (animTime <= animations->channels[chanIndex].mPositionKeys[0].mTime ) // handle anim not from the 0 second
				{
					key1 = 0;
					key2 = 0;
					lowerTimeTicker = animTime;
					break;	
				}
				if (animTime > animations->channels[chanIndex].mPositionKeys[keysize-1].mTime) // handle anim end before last keyframe
				{
					key1 = keysize-1;
					key2 = keysize-1;
					upperTimeTicker = animTime;
					break;
				}
			}
	
			Factor = (animTime - lowerTimeTicker)/(upperTimeTicker-lowerTimeTicker);
			assert(Factor >= 0.0 && Factor <= 1.0);

			if (animations->channels[chanIndex].mPositionKeys.size() > 1)
				lerp(aiTranslation, animations->channels[chanIndex].mPositionKeys[key1].mValue, animations->channels[chanIndex].mPositionKeys[key2].mValue, Factor); // translation
			if (animations->channels[chanIndex].mScalingKeys.size() > 1)
				lerp(aiScale, animations->channels[chanIndex].mScalingKeys[key1].mValue, animations->channels[chanIndex].mScalingKeys[key2].mValue, Factor); // scale
			if (animations->channels[chanIndex].mRotationKeys.size() > 1)
				slerp(aiRotation, animations->channels[chanIndex].mRotationKeys[key1].mValue, animations->channels[chanIndex].mRotationKeys[key2].mValue, Factor); // rotation
 
			glm::vec3 translation((GLfloat)aiTranslation.x, (GLfloat)aiTranslation.y, (GLfloat)aiTranslation.z);
			glm::vec3 scaling((GLfloat)aiScale.x, (GLfloat)aiScale.y, (GLfloat)aiScale.z);
			glm::quat rotation((GLfloat)aiRotation.w, (GLfloat)aiRotation.x, (GLfloat)aiRotation.y, (GLfloat)aiRotation.z);
 
			Pose tempPose;
			tempPose.orientation = rotation;
			tempPose.translation = translation;
			tempPose.weight = animation->weight;
			contributes[local_boneID[animations->channels[chanIndex].name]].push_back(tempPose);

			} // end bone search
		} // end if
	} // end all animation checks

	return contributes;

};

void animationController::setAnimationInQ(int index, bool loop, float blendDuration , TransitionType transition ){
	
	if (index < working_animations.size())
	{
		Command command = Command(&working_animations[index], loop, blendDuration, transition);
		this->m_blender.commandQ.push(command);
	}

}

void animationController::getResultAnimation(std::map<int, std::vector<Pose>> contributingPoses, glm::mat4 parentMatrix, core::Model::Animation::BoneNode* refer_skele ){
	
	int boneIndex = 0;
    for (int x = 0; x <  in_animations[0].channels.size(); x++)
    {
        if (refer_skele->name == in_animations[0].channels[x].name)
        {
            boneIndex = x;
        }
    }
	// get the blending result
	glm::mat4 blenderMatrix;
	if(contributingPoses[boneIndex].size() == 1)
		{
			blenderMatrix = glm::translate(glm::mat4(1), contributingPoses[boneIndex][0].translation)
				* glm::toMat4(contributingPoses[boneIndex][0].orientation);
		}
	else if (contributingPoses[boneIndex].size() == 2)
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), (glm::lerp (contributingPoses[boneIndex][0].translation, 
				contributingPoses[boneIndex][1].translation, contributingPoses[boneIndex][1].weight))  );

			glm::mat4 orientation = glm::toMat4(glm::slerp(contributingPoses[boneIndex][0].orientation, 
				contributingPoses[boneIndex][1].orientation, contributingPoses[boneIndex][1].weight));

			blenderMatrix = translation * orientation; 
		}

	glm::mat4 passingMatrix = parentMatrix * blenderMatrix ; 

	in_animations[0].boneTrans[ local_boneID[refer_skele->name] ] = passingMatrix * refer_skele->boneTransform;
	
    // loop through every child and use this bone's transformations as the parent transform
	for (int x = 0; x < refer_skele->children.size(); x++)
    {
        getResultAnimation( contributingPoses, passingMatrix , &refer_skele->children[x] );
    }

}