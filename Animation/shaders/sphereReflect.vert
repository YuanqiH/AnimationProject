#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Fg_Position;
out vec3 Fg_Normal;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;


//uniform vec4 camPos;


void main()
{	
	mat4 MVP = projectionMat * viewMat * modelMat;
	mat4 VP = viewMat * modelMat;
	gl_Position = MVP * vec4(position.xyz, 1.0);
	//calculate in the world space 
	
	/*
	Fg_Position = vec3(modelMat * vec4(position.xyz, 1.0));
	//Fg_Normal = vec3(transpose(inverse(modelMat)) * vec4(normal,0.0)); 
	Fg_Normal = vec3(modelMat * vec4(normal,0.0)); 
	*/
	
	// working in eye space 
	Fg_Position = vec3(VP * vec4(position.xyz, 1.0));
	//Fg_Normal = vec3(transpose(inverse(modelMat)) * vec4(normal,0.0)); 
	Fg_Normal = vec3(VP * vec4(normal,0.0)); 
	
	
	 
}