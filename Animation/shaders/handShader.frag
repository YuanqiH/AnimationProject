#version 330 core

in vec4 TexCoord0;
in vec3 Normal0; 
in vec3 WorldPos0; 
in vec4 DebugBoneColor;


uniform sampler2D tex;
uniform vec3 eyeP;

out vec4 finalColor;
#define shininess 3.0
const float material_kd = 0.6;
const float material_ks = 0.4;

void main(){

		
		vec4 color_back = vec4(0.0,0.0,0.0,1.0);
		vec4 color_shade = vec4(1.0,0.8,0.0,1.0);;  
		vec4 color_highlight = vec4(1.0,1.0,1.0,1.0);
		vec3 WorldLightP = vec3(30.0,100.0,30.0);
		
		vec3 EyeDir = normalize( eyeP - WorldPos0 );
		vec3 LightVert =  normalize( WorldLightP - WorldPos0 );
		vec3 EyeLight = normalize( LightVert + EyeDir );
		
		
		float sil = max( dot( Normal0, EyeDir ), 0.0 );
		if( sil< 0.5 ) finalColor = color_back; // closing line
		else{
			finalColor =  color_shade;
			float spc = pow( max( dot(Normal0, EyeLight), 0.0 ),shininess);
			if( spc < 0.2 ) finalColor *= 0.8;
			else finalColor =  color_highlight;
			
			float diffuse = max( dot ( Normal0, LightVert), 0.0);
			if ( diffuse < 0.5 ) finalColor *= 0.8;
			
		}
		//finalColor = DebugBoneColor;

		//finalColor = vec4(EyeDir,1.0);
		//finalColor = vec4(sil,sil,sil, 1.0 ) * color1;
		//finalColor = texture(tex,TexCoord0) * Color_white;
}
