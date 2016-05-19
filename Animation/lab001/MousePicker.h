#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class MousePicker
{
public:
	MousePicker(glm::vec3 camPos, glm::mat4 view, glm::mat4 projection,GLFWwindow* window);
	glm::vec3 getCurrentRay();
	void update();

private:
	glm::vec3 currentRay;

	GLFWwindow* window;
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::vec3 CameraPosition;

	glm::vec3 calculateMouseRay();

};

MousePicker::MousePicker(glm::vec3 camPos, glm::mat4 view, glm::mat4 projection, GLFWwindow* window)
{
	this->CameraPosition = camPos;
	this->viewMatrix = view;
	this->projectionMatrix = projection;
	this->window = window;
}

void MousePicker::update(){
	currentRay = calculateMouseRay();
}

glm::vec3 MousePicker::calculateMouseRay(){
	double xpos = 0 , ypos= 0 ;
	glfwGetCursorPos(window, &xpos, &ypos);
}

glm::vec3 getNormalizedDeviceCoords(float mouseX, float mouseY){
	float x = (2 * mouseX);
}

glm::vec3 MousePicker::getCurrentRay(){
	return currentRay;
}


