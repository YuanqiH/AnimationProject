#version 330 core


in vec2 tex;

out vec4 finalcolor;
uniform sampler2D diffuse1;

void main()
{
	
	vec4 color = vec4( 1.0 , 1.0 , 1.0 , 1.0); //vec4( 0.055 , 0.2 , 0.25 , 1.0);
	finalcolor = texture(diffuse1,tex) * color;
}