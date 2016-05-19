#version 330 core


//const float Eta = 1/1.66;
const float EtaR = (1.0/1.15);
const float EtaG = (1.0/1.1);
const float EtaB = (1.0/1.05);
const float FresnelPower = 5.0;

const float F = ((1.0-EtaG)*(1.0-EtaG))/((1.0+EtaG)*(1.0+EtaG));
float Ratio;

in vec3 Fg_Position;
in vec3 Fg_Normal;

uniform samplerCube tex_cube;
uniform vec3 viewPos;
uniform float swichFlag;
uniform mat4 viewMat;

out vec4 final_color;

 void main(void)
 {
	
	//vec3 viewDir = normalize (viewPos-Fg_Position);
	//vec3 viewDir = normalize (Fg_Position-viewPos);
	vec3 viewDir = normalize (0-Fg_Position);
	vec3 Normal = normalize(Fg_Normal);
	
	Ratio = F + (1.0 - F) * pow((1.0 - dot(viewDir, Normal)), FresnelPower);
	
	//calculate refraction
	vec3 refractColor;
	vec3 RefractR = refract(-viewDir, Normal, EtaR); 
	RefractR = vec3( inverse(viewMat) * vec4(RefractR, 0.0));//transform back into world space
	refractColor.r = vec3(texture(tex_cube, RefractR)).r;
	vec3 RefractG = refract(-viewDir, Normal, EtaG); 
	RefractG = vec3( inverse(viewMat) * vec4(RefractG, 0.0));//transform back into world space
	refractColor.g = vec3(texture(tex_cube, RefractG)).g;
	vec3 RefractB = refract(-viewDir, Normal, EtaB); 
	RefractB = vec3( inverse(viewMat) * vec4(RefractB, 0.0));//transform back into world space
	refractColor.b = vec3(texture(tex_cube, RefractB)).b;
	
	
	
	//calculate reflection
    vec3 reflectedDirection = normalize(reflect(-viewDir, Normal));
	reflectedDirection = vec3( inverse(viewMat) * vec4(reflectedDirection, 0.0));// transform back into world space
    vec3 reflectColor = vec3(texture(tex_cube, reflectedDirection));
	
	
	
	vec3 color = mix(refractColor, reflectColor, Ratio);
	
		if (swichFlag==1.0){
			final_color = vec4(reflectColor,1.0);
			//final_color = vec4(viewDir,1.0);
		}
		if (swichFlag==2.0){
			final_color = vec4(refractColor,1.0);
			//final_color = vec4(viewDir,1.0);
		}
		if (swichFlag==3.0){
			final_color = vec4(color,1.0);
			//final_color = vec4(Normal,1.0);
		}

	
	

}