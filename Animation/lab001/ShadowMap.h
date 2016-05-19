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
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ShadowMap
	
{
public:
	GLuint SHADOW_WIDTH, SHADOW_HEIGHT;
	glm::vec3 lightPosition;

	ShadowMap(GLuint SHADOW_WIDTH,GLuint SHADOW_HEIGHT);
	GLuint getDepthMap();
	GLuint getDepthMapFBO();

private:
	GLuint depthMap;
	GLuint depthMapFBO;
};

ShadowMap::ShadowMap( GLuint SHADOW_WIDTH = 1024,GLuint SHADOW_HEIGHT = 1024)
{
		
		glGenFramebuffers(1, &this->depthMapFBO);
		// generate texture
		glGenTextures(1, &this->depthMap);
		glBindTexture(GL_TEXTURE_2D,this->depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); //erase repeating shadow
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  
		GLfloat borderColor[] = {1.0,1.0,1.0,1.0};
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
		glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

GLuint ShadowMap::getDepthMap(){
	return this->depthMap;
}

GLuint ShadowMap::getDepthMapFBO(){
	return this->depthMapFBO;
}
