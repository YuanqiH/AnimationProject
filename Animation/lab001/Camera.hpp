#pragma once

#include <glm\gtx\quaternion.hpp>
#include <glm\gtx\associated_min_max.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

struct  playerTransform
{
	glm::vec3 playerPosition;
	glm::vec3 playerDirection;
	glm::vec3 playerUp;
};

class Camera
{
public:
	glm::vec3 CamPosition;
	glm::vec3 Direction;
	glm::vec3 Up;
	glm::vec3 rightv;
	float speed; 
	float mouseSpeed;
	float FoV;

	bool fixCamera;
	bool thirdPersonOn,firstPersonOn, quaternionOn, EulerOn;
	bool freeCameraOn;
	glm::quat m_quaternionTotal;


	Camera(int window_width,int window_height);
	~Camera(){};

	void Update(GLFWwindow* window, float deltaTime, bool* keyState);

	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::mat4 getThirdPlaneModelMatrix();

	void ProcessKeys(bool* keyState);
	void setTirdpersonTransform(glm::vec3 position, glm::vec3 direction, glm::vec3 up);
	void setTirdpersonTransform(playerTransform);

private:
	int window_width, window_height;
	// horizontal angle : toward -Z
	float horizontalAngle;
	// vertical angle : 0, look at the horizon
	float verticalAngle;
	// Initial Field of View

	float ThirdPersonVerticalAngle, ThirdPersonHorizontalAngle, rotateSpeedCamera,ThirdpersonDistance;
	
	float rotz,rotx,roty;// for third person
	GLFWwindow* window;
	float deltaTime;

	void computeMatricesFromInputs();
	void ThirdPersonMode();
	void FirstPersonMode();

	playerTransform playerTrans; // can be set from outside
	
};


