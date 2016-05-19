

#include "Npc.hpp"

float Npc::raidus = 3.0f;

Npc::Npc(core::Model* model, Camera* camera){

	this->local_camera = camera;
	this->local_model = model;
	this->skeleton =  &model->animations[0].root; // get the root of the animation as the skeleton for rendering 
	this->local_animations = model->animations;
	setController();  // put all the animation info to controller 

	this->state =NpcName::State::idle; // initial state
	m_controller.setAnimationInQ(0); 
	
	// have to set the player trans manually 
	playerTrans.playerPosition = glm::vec3(-30.0f,0.0f,10.0f );   //skeleton->boneTransform[3]);
	playerTrans.playerDirection = glm::vec3(skeleton->boneTransform[2]); 
	playerTrans.playerUp =  glm::vec3(skeleton->boneTransform[1]);//glm::vec3(1.0f,0.0f,0.0f);

	std::cout<< "playerPosition"<<playerTrans.playerPosition.x<<playerTrans.playerPosition.y<<playerTrans.playerPosition.z<<std::endl;
	std::cout<< "playerDirection"<<playerTrans.playerDirection.x<<playerTrans.playerDirection.y<<playerTrans.playerDirection.z<<std::endl;
	std::cout<< "playerUp"<<playerTrans.playerUp.x<<playerTrans.playerUp.y<<playerTrans.playerUp.z<<std::endl;

	local_model->setModelTrans(this->getAdjustMatrix());

	this->rotateSpeed = 3.0f;
	this->chractorMoveSpeed = 4.0f;
	rotz = 0.0f;
	roty = 0.0f;
	rotx = 0.0f;
	
}

void Npc::update(double deltaTime, bool *keys){
	
	if (m_controller.m_blender.isIdle) // set idle as default 
	{
		SetState(NpcName::State::idle);
	}

	this->keys = keys;

	processKeys(deltaTime);
	detect();
	move();
	/*
	local_model->setModelTrans(this->getThirdPlaneModelMatrix() * this->getAdjustMatrix()); // actual movement Matrix
	this->local_camera->setTirdpersonTransform(playerTrans); // upadte the cameraMatrix
	*/
	renderAnimation = m_controller.update(deltaTime); // animation Matrix 

}

void Npc::SetState(NpcName::State newstate){

	if (state != newstate)
	{
		if(newstate == NpcName::State::ridded)
			m_controller.setAnimationInQ(NpcName::State::ridded, true, 0.2, TransitionType::Smooth);
		else if(newstate == NpcName::State::idle)
			m_controller.setAnimationInQ(NpcName::State::idle, true, 0.2, TransitionType::Smooth);
		else if(newstate == NpcName::State::redy)
			m_controller.setAnimationInQ(NpcName::State::redy, true, 0.2, TransitionType::Smooth);
		else if(newstate == NpcName::State::ridd_idle)
			m_controller.setAnimationInQ(NpcName::State::ridd_idle, true, 0.3, TransitionType::Smooth);

		state = newstate;
	}

}

void Npc::playerRender(){

	glm::mat4 projectionMatrix_local = local_camera->getProjectionMatrix();
	glm::mat4 ViewMatrix_local = local_camera->getViewMatrix();
	glm::mat4 modelMatrix = local_camera->getThirdPlaneModelMatrix(); // no longer use modelMatrix from camera

	local_model->render(projectionMatrix_local,ViewMatrix_local, modelMatrix, &renderAnimation, local_camera);
}

void Npc::setController(){
	m_controller.init(local_animations, local_model->boneID, skeleton);
}


glm::mat4 Npc::getAdjustMatrix(){
	
	glm::quat m_quaternionz = glm::angleAxis( 1.57f ,glm::vec3(0.0f,0.0f,1.0f));
	glm::quat m_quaterniony = glm::angleAxis( 3.14f, glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 RotationMatrix = glm::toMat4(m_quaterniony * m_quaternionz);
	RotationMatrix =  glm::scale(RotationMatrix, glm::vec3(0.1));
	return RotationMatrix;
}

glm::mat4 Npc::getThirdPlaneModelMatrix(){
	glm::mat4 model,out_model;
	glm::mat4 TranslationMatrix, RotationMatrix,ScaleMatrix; // slip the whole model matrix into 3 

	//create scale matrix
	ScaleMatrix = glm::scale(model,glm::vec3(1.0));

	if(local_camera->quaternionOn == true){
	//creata rotation matrix using quaternion 
	glm::quat m_quaternionz = glm::angleAxis(rotz,glm::vec3(0.0f,0.0f,1.0f));
	glm::quat m_quaterniony = glm::angleAxis(roty, glm::vec3(0.0f,1.0f,0.0f));
	glm::quat m_quaternionx = glm::angleAxis(rotx, glm::vec3(1.0f,0.0f,0.0f));
	glm::quat temp_quaternion =  m_quaternionx * m_quaterniony * m_quaternionz;
	// use an acumulative quaternion to save current rotation state, overcome gimbal lock !
	m_quaternionTotal *= temp_quaternion;
	RotationMatrix = glm::toMat4(m_quaternionTotal);
	//reset important
	rotz = 0.0f;
	roty = 0.0f;
	rotx = 0.0f;
	}
	if(local_camera->EulerOn == true){
	// update rotation with Euler angles
	glm::mat4 m_quaternionz = glm::rotate(model,rotz,glm::vec3(0.0f,0.0f,1.0f));
	glm::mat4 m_quaterniony = glm::rotate(model,roty, glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 m_quaternionx = glm::rotate(model,rotx, glm::vec3(1.0f,0.0f,0.0f));
	glm::mat4 tempMatrix =  m_quaternionx * m_quaterniony * m_quaternionz;
	RotationMatrix *= tempMatrix;
	}
	//update planeDirction, reset planeDirction, since in loop
	playerTrans.playerDirection = glm::vec3(skeleton->boneTransform[2]);
	playerTrans.playerDirection = glm::vec3(RotationMatrix * glm::vec4(playerTrans.playerDirection,1.0f));
	playerTrans.playerDirection = glm::normalize(playerTrans.playerDirection);
	//std::cout<<planeDirection.x<<","<<planeDirection.y<<","<<planeDirection.z<<endl;
	// update planeUp vector for first person mode
	playerTrans.playerUp =  glm::vec3(skeleton->boneTransform[1]); 
	playerTrans.playerUp = glm::vec3(RotationMatrix * glm::vec4(playerTrans.playerUp,1.0f));
	playerTrans.playerUp = glm::normalize(playerTrans.playerUp);

	//create translation matrix
	TranslationMatrix = glm::translate(model,playerTrans.playerPosition);
	//update camera position

	out_model =  TranslationMatrix * RotationMatrix * ScaleMatrix;
	return out_model;
}

void Npc::processKeys(float deltaTime){
	



}

void Npc::setPlayer(Player* player){
	this->player = player;
}

void Npc::detect(){
	if (player != nullptr)
	{
		if (glm::distance(player->playerTrans.playerPosition, this->playerTrans.playerPosition) <= this->raidus) // in radius
		{
			player->doable = true;
		}
		else
			player->doable = false;

	}

	if (player->riddingOn) // copy all the player Transform
	{
		playerTrans = player->playerTrans;
	}
	
}

void Npc::move(){
	// send MoveMatrix based on playerTransform
	glm::mat4 transformMatrix;
	transformMatrix[3] = glm::vec4(playerTrans.playerPosition, 1.0f);
	transformMatrix[2] = glm::vec4(playerTrans.playerDirection, 0.0f);
	transformMatrix[1] = glm::vec4(playerTrans.playerUp, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(playerTrans.playerDirection, playerTrans.playerUp));
	transformMatrix[0] = glm::vec4( right, 0.0f);
	local_model->setModelTrans(transformMatrix* getAdjustMatrix());

	if (player->doable) // can be ridded
	{
		if (player->riddingOn == false) // on the ground 
		{
			SetState(NpcName::State::redy);
		}
		else
		{
			//std::cout<<"running "<< player->runningOn << std::endl;
			if (player->runningOn )
			{
				SetState(NpcName::State::ridded);
			}
			else
			{
				SetState(NpcName::State::ridd_idle);
			}
		}
	}
	else
	{
		SetState(NpcName::State::idle);
	}


}