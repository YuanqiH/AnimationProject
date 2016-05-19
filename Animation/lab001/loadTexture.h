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

class MyTexture
{
private:
	GLuint textureID;
public:
	
	
	MyTexture(){}

	void TextureFromFile(const char* name, string directory);
	void use(GLuint shaderProgramID,std::string name, int textUnit = 0);


};
