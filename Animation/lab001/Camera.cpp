#include "Camera.hpp"

Camera::Camera(int window_width,int window_height){ // initialise
		
		this->window_width = window_width;
		this->window_height = window_height;
		this->CamPosition = glm::vec3( 0.0f, 5.0f, 5.0f );
		this->Direction = glm::vec3( 0.0f, 0.0f, -1.0f );
		this->Up = glm::vec3( 0.0f, 1.0f, 0.0f );
		this->rightv = glm::vec3( 0.0f, 0.0f, 0.0f );
		this->horizontalAngle = 3.14f;
		this->verticalAngle = 0.0f;
		this->FoV = 45.0F;
		this->speed = 4.0f; // 4 units / second
		this->mouseSpeed = 0.005f;
		this->quaternionOn = true;
		this->EulerOn = false;
		this-> rotateSpeedCamera = 5.0f;
		this-> ThirdPersonHorizontalAngle =0.0f;  // TODO : adjust camera horizontally
		this-> ThirdPersonVerticalAngle = -0.785f;
		this-> ThirdpersonDistance = 60.0f;

		this->playerTrans.playerPosition = glm::vec3(0.0f,0.0f,0.0f);
		this->playerTrans.playerDirection = glm::vec3(0.0f,0.0f,1.0f);
		this->playerTrans.playerUp = glm::vec3(0.0f,1.0f,0.0f);	
}


//Camera caculation
void Camera::computeMatricesFromInputs()
	{
	double xpos, ypos;
	glfwGetCursorPos(this->window, &xpos, &ypos);
	// Compute new orientation
	horizontalAngle += mouseSpeed * this->deltaTime * float(window_width/2 - xpos );
	verticalAngle   += mouseSpeed * this->deltaTime * float( window_height/2 - ypos );
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	Direction = glm::vec3 (
    cos(verticalAngle) * sin(horizontalAngle),
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
	);
	// Right vector

	rightv = glm::normalize(glm::vec3(glm::cross(Direction,glm::vec3(0.0,1.0,0.0))));
	// Up vector : perpendicular to both direction and right
	Up = glm::cross( rightv, Direction );

	}

glm::mat4 Camera::getProjectionMatrix()
{ 
	 glm::mat4 projectionMatrix = glm::perspective(FoV, float(window_width)/float(window_height), 1.0f, 500.0f);
	 return projectionMatrix;
}


glm::mat4 Camera::getViewMatrix()
{
	// Camera matrix
	glm::mat4 ViewMatrix = glm::lookAt(
    CamPosition,           // Camera is here
    CamPosition+Direction, // and looks here : at the same position, plus "direction"
    Up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	return ViewMatrix;
}

glm::mat4 Camera::getThirdPlaneModelMatrix(){
	glm::mat4 model,out_model;
	glm::mat4 TranslationMatrix, RotationMatrix,ScaleMatrix; // slip the whole model matrix into 3 

	//create scale matrix
	ScaleMatrix = glm::scale(model,glm::vec3(1.0));

	if(quaternionOn == true){
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
	if(EulerOn == true){
	// update rotation with Euler angles
	glm::mat4 m_quaternionz = glm::rotate(model,rotz,glm::vec3(0.0f,0.0f,1.0f));
	glm::mat4 m_quaterniony = glm::rotate(model,roty, glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 m_quaternionx = glm::rotate(model,rotx, glm::vec3(1.0f,0.0f,0.0f));
	glm::mat4 tempMatrix =  m_quaternionx * m_quaterniony * m_quaternionz;
	RotationMatrix *= tempMatrix;
	}
	//update planeDirction, reset planeDirction, since in loop
	playerTrans.playerDirection = glm::vec3(0.0f,0.0f,1.0f);
	playerTrans.playerDirection = glm::vec3(RotationMatrix * glm::vec4(playerTrans.playerDirection,1.0f));
	playerTrans.playerDirection = glm::normalize(playerTrans.playerDirection);
	//std::cout<<planeDirection.x<<","<<planeDirection.y<<","<<planeDirection.z<<endl;
	// update planeUp vector for first person mode
	playerTrans.playerUp = glm::vec3(0.0f,1.0f,0.0f);
	playerTrans.playerUp = glm::vec3(RotationMatrix * glm::vec4(playerTrans.playerUp,1.0f));
	playerTrans.playerUp = glm::normalize(playerTrans.playerUp);

	//create translation matrix
	TranslationMatrix = glm::translate(model,playerTrans.playerPosition);
	//update camera position

	out_model =  TranslationMatrix * RotationMatrix * ScaleMatrix;
	return out_model;
}


void Camera::ThirdPersonMode(){

	//======================================

	

	//keep using camera direction
	//Direction = glm::normalize(playerTrans.playerPosition - CamPosition);
	ThirdPersonVerticalAngle = glm::clamp(ThirdPersonVerticalAngle, -1.57f,1.57f); // [-pi/2,pi/2]
	Direction = glm::normalize(glm::vec3(0.0f,sin(ThirdPersonVerticalAngle),cos(ThirdPersonVerticalAngle)));
	//std::cout<< "direction"<<Direction.x<<Direction.y<<Direction.z<<std::endl;


	CamPosition = playerTrans.playerPosition - Direction * ThirdpersonDistance;
	//std::cout<< "CamPosition"<<CamPosition.x<<CamPosition.y<<CamPosition.z<<std::endl;
	rightv = glm::normalize(glm::vec3(glm::cross(Direction,glm::vec3(0.0,1.0,0.0))));
	// Up vector : perpendicular to both direction and right
	// up towards y axis, differ from plane up 
	Up = glm::cross( rightv, Direction );
	
}

void Camera::FirstPersonMode(){
	// raplace camera direction with plane direction
	CamPosition = playerTrans.playerPosition;
	Direction = playerTrans.playerDirection;
	Up = playerTrans.playerUp;
}

void Camera::Update(GLFWwindow* window,float deltaTime, bool* keyState){

		this->window = window;
		this->deltaTime = deltaTime;

		this->ProcessKeys(keyState);

		if ( fixCamera == false && freeCameraOn == true) // if not fix
		{
			computeMatricesFromInputs();
		}
		if(thirdPersonOn == true && firstPersonOn == false){
			ThirdPersonMode();
		}
		if(firstPersonOn == true && thirdPersonOn == false){
			FirstPersonMode();
		}

}

void Camera::setTirdpersonTransform(glm::vec3 position, glm::vec3 direction, glm::vec3 up){
		playerTrans.playerDirection = direction;
		playerTrans.playerPosition = position;
		playerTrans.playerUp = up;
	}


void Camera::setTirdpersonTransform(playerTransform transfrom){
	playerTrans = transfrom;
}

void Camera::ProcessKeys(bool* keyState){

	if (freeCameraOn)
	{
		if( keyState[GLFW_KEY_W])
		{
			 CamPosition += Direction * deltaTime * speed;
		}
		if( keyState[GLFW_KEY_S])
		{
			 CamPosition -= Direction * deltaTime * speed;
		}
		if( keyState[ GLFW_KEY_D])
		{
			CamPosition += rightv * deltaTime * speed;
		}
		if(   keyState[ GLFW_KEY_A] )
		{
			 CamPosition -= rightv * deltaTime * speed;
		}
	}
	

	if( keyState[ GLFW_KEY_LEFT_SHIFT] ){
			speed = 20.0f;
	}
	else{
			speed = 5.0f;
	}

	if (this->thirdPersonOn)
	{

		if( keyState[GLFW_KEY_W])
		{
			 ThirdpersonDistance -=  deltaTime * speed;
		}
		if( keyState[GLFW_KEY_S])
		{
			 ThirdpersonDistance +=  deltaTime * speed;
		}




	}

	/*
	// input for plane cotrol 
	// these controls can be seperate from camera class
	if( keyState[GLFW_KEY_UP])
	{
		 playerTrans.playerPosition += playerTrans.playerDirection * deltaTime * speed;
		 CamPosition += playerTrans.playerDirection * deltaTime * speed;
	}
	if( keyState[GLFW_KEY_DOWN])
	{
		  playerTrans.playerPosition -= playerTrans.playerDirection * deltaTime * speed;
		 CamPosition -= playerTrans.playerDirection * deltaTime * speed;
	}
	// rolling by z axis
	if(keyState[GLFW_KEY_J])
	{
		rotz -= deltaTime * rotateSpeed_plane; 
	}
		if(keyState[GLFW_KEY_L])
	{
		rotz += deltaTime * rotateSpeed_plane; 
	}

	//yawing by y axis
	if(keyState[GLFW_KEY_LEFT])
	{
		roty += deltaTime * rotateSpeed_plane;
	}
	if(keyState[GLFW_KEY_RIGHT])
	{
		roty -= deltaTime * rotateSpeed_plane;
	}

	//pitching by x axis
	if(keyState[GLFW_KEY_I])
	{
		rotx -= deltaTime * rotateSpeed_plane;
	}
	if(keyState[GLFW_KEY_K])
	{
		rotx += deltaTime * rotateSpeed_plane;
	}
	*/
}