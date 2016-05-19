
#include "player.hpp"

float Player::riddingOffset = 1.0f;

//initial
Player::Player(core::Model* model, Camera* camera){
	
	this->local_camera = camera;
	this->local_model = model;
	this->skeleton =  &model->animations[0].root; // get the root of the animation as the skeleton for rendering 
	this->local_animations = model->animations;
	setController();  // put all the animation info to controller 

	this->state = State::idle; // initial state
	m_controller.setAnimationInQ(0); 
	
	// have to set the player trans manually 
	playerTrans.playerPosition = glm::vec3(70.0f, 0.0f, 30.0f);      //skeleton->boneTransform[3]);
	playerTrans.playerDirection = glm::vec3(skeleton->boneTransform[2]); //glm::vec3(0.0f,0.0f,-1.0f);
	playerTrans.playerUp =  glm::vec3(skeleton->boneTransform[1]);//glm::vec3(1.0f,0.0f,0.0f);
	this->local_camera->setTirdpersonTransform(playerTrans);

	std::cout<< "playerPosition"<<playerTrans.playerPosition.x<<playerTrans.playerPosition.y<<playerTrans.playerPosition.z<<std::endl;
	std::cout<< "playerDirection"<<playerTrans.playerDirection.x<<playerTrans.playerDirection.y<<playerTrans.playerDirection.z<<std::endl;
	std::cout<< "playerUp"<<playerTrans.playerUp.x<<playerTrans.playerUp.y<<playerTrans.playerUp.z<<std::endl;

	local_model->setModelTrans(this->getAdjustMatrix());

	this->rotateSpeed = 3.0f;
	this->chractorMoveSpeed = 4.0f;
	this->riddingOn = false;
	this->doable = false;

	rotz = 0.0f;
	roty = 0.0f;
	rotx = 0.0f;
	
}

void Player::update(double deltaTime, bool *keys){
	
	if (m_controller.m_blender.isIdle) // set idle as default 
	{
		SetState(State::idle);
	}

	this->keys = keys;
	runningOn = false;
	attackOn = false;
	processKeys(deltaTime);
	move();
	local_model->setModelTrans(this->getThirdPlaneModelMatrix() * this->getAdjustMatrix()); // actual movement of player
	this->local_camera->setTirdpersonTransform(playerTrans); // upadte the camera
	renderAnimation = m_controller.update(deltaTime);

}

void Player::SetState(State newstate){

	if (state != newstate)
	{
		if(newstate == State::run)
			m_controller.setAnimationInQ(State::run, true, 0.2, TransitionType::Smooth);
		else if(newstate == State::idle)
			m_controller.setAnimationInQ(State::idle, true, 0.4, TransitionType::Smooth);
		else if(newstate == State::attack)
			m_controller.setAnimationInQ(State::attack, false, 1.0, TransitionType::Smooth);
		else if (newstate == State::rid_idle)
			m_controller.setAnimationInQ(State::rid_idle, true, 0.3, TransitionType::Smooth);
		state = newstate;
	}

}

void Player::playerRender(){

	glm::mat4 projectionMatrix_local = local_camera->getProjectionMatrix();
	glm::mat4 ViewMatrix_local = local_camera->getViewMatrix();
	glm::mat4 modelMatrix = local_camera->getThirdPlaneModelMatrix();

	local_model->render(projectionMatrix_local,ViewMatrix_local, modelMatrix, &renderAnimation, local_camera);
}

void Player::setController(){
	m_controller.init(local_animations, local_model->boneID, skeleton);
}

void Player::processKeys(float deltaTime){
	
	if (doable)
	{
		if (keys[GLFW_KEY_N])
			riddingOn = true;
		if (keys[GLFW_KEY_M])
			riddingOn = false;
	}

	if( keys[ GLFW_KEY_LEFT_SHIFT] ){
			chractorMoveSpeed = 20.0f;
	}
	else{
			chractorMoveSpeed = 10.0f;
			if (riddingOn)
				chractorMoveSpeed = 5.0f;
	}

	// input for plane cotrol 
	// these controls can be seperate from camera class
	if( keys[GLFW_KEY_UP])
	{
		 playerTrans.playerPosition += playerTrans.playerDirection * deltaTime * chractorMoveSpeed;
		 runningOn = true;
	}
	/* // only allow forward, since animation is limited
	if( keys[GLFW_KEY_DOWN])
	{
		  playerTrans.playerPosition -= playerTrans.playerDirection * deltaTime * chractorMoveSpeed;
		  runningOn = true;
	}
	*/
	if (runningOn) // only response in running
	{
		// rolling by z axis
		if(keys[GLFW_KEY_J])
		{
			rotz -= deltaTime * rotateSpeed; 
		}
			if(keys[GLFW_KEY_L])
		{
			rotz += deltaTime * rotateSpeed; 
		}

		//yawing by y axis
		if(keys[GLFW_KEY_LEFT])
		{
			roty += deltaTime * rotateSpeed;
		}
		if(keys[GLFW_KEY_RIGHT])
		{
			roty -= deltaTime * rotateSpeed;
		}

		//pitching by x axis
		if(keys[GLFW_KEY_I])
		{
			rotx -= deltaTime * rotateSpeed;
		}
		if(keys[GLFW_KEY_K])
		{
			rotx += deltaTime * rotateSpeed;
		}
	}
	
}

// for the unmatch of blender orientation
glm::mat4 Player::getAdjustMatrix(){
	
	glm::quat m_quaternionz = glm::angleAxis( 1.57f ,glm::vec3(0.0f,0.0f,1.0f));
	glm::quat m_quaterniony = glm::angleAxis( 3.14f, glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 RotationMatrix = glm::toMat4(m_quaterniony * m_quaternionz);
	RotationMatrix =  glm::scale(RotationMatrix, glm::vec3(0.1));
	if (riddingOn)
	{
		glm::mat4 translateM = glm::translate(glm::mat4(1) , (riddingOffset * playerTrans.playerUp) );
		RotationMatrix = translateM * RotationMatrix;
	}
	return RotationMatrix;
}

glm::mat4 Player::getThirdPlaneModelMatrix(){
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

// state changing here
void Player::move(){

	if (riddingOn == false){
		if (runningOn){
		SetState(State::run);
		}
	else // no running 
		{
			if (attackOn)
			{
				SetState(State::attack);
			}
			else
				SetState(State::idle);
		}
	}
	else // ridding 
	{
		SetState(State::rid_idle);
	}
	
	//std::cout<< "ridding " << riddingOn << std::endl;
	//std::cout<< "doable" << doable << std::endl;


}

