#pragma once

#include <IL\il.h>
#include <sstream>
#include <iostream>
#include <Windows.h>
#include <GL\glew.h>
#include <GL\GL.h>
#include "Shader.hpp"
class CubeMap{
private:

	GLuint vbo;
	GLuint vao;
	GLuint texID;
	//static ILuint* imageIDs;
	//static GLuint* textureIDs;
	void create_cube_map (
	const char* front,
	const char* back,
	const char* top,
	const char* bottom,
	const char* left,
	const char* right,
	GLuint* tex_cube);

	bool CubeMap::load_cube_map_side (GLuint texture, GLenum side_target, const char* file_name);

	Shader _shader;

public:
	CubeMap();

	void loadCubeMap(const char* mapFolder);
	void drawSkyBox();
	void use();


	void setShader(Shader* s);
	void setShader(std::string vrt, std::string frg);
	Shader* getShader();

};