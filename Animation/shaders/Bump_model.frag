#version 330 core

in VS_OUT{
	
	vec2 TexCoords;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
	
} fg_in;

in VS_OUT_O{
	
	vec2 TexCoord;
	vec3 fragPosition;
	vec3 frg_normal;
	vec3 frg_LightPos;
	vec3 frg_ViewPos;
	
} fg_in_o;

const float material_kd = 0.6;
const float material_ks = 0.4;

out vec4 color;

//uniform sampler2D diffuse1;
uniform float normalMapping; 
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_speculaer1;
uniform sampler2D texture_normal1;
uniform mat4 modelMat;

void main()
{    
   // color = vec4(texture(texture_diffuse1, TexCoords));
   
   // transfer RGB into normal
   vec3 normal = texture(texture_normal1, fg_in.TexCoords).rgb;
   normal = normalize(normal * 2.0 - 1.0);//transform normal into world space
   
   
   vec3 lightDir = normalize(fg_in.TangentLightPos - fg_in.TangentFragPos);
   vec3 viewDir =  normalize(fg_in.TangentViewPos - fg_in.TangentFragPos);
   
   vec4 material_color = texture(texture_diffuse1,fg_in.TexCoords);
   //material_color = vec3(1.0,1.0,1.0);
   
    //ambient color
	vec4 ambient = 0.2f * material_color;
	
	//diffuse light
	float diff = max(dot(lightDir,normal),0.0);
	vec4 diffuse = material_kd * diff* material_color;

	//specular light
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir),0.0), 32.0);
	vec4 specular = material_ks * spec* material_color ;
	
	vec4 result_color = vec4(vec3(ambient + diffuse + specular),1.0) ;
	color = vec4(result_color);
	//color = vec4(normal,1.0f);
	//color = vec4(lightDir,1.0f);
	//color = vec4(viewDir,1.0f);
	//color =  material_color;
	
	
	if(normalMapping == 0.0){ //normal map is not used
	
	vec3 normal_o = normalize(fg_in_o.frg_normal);
	vec3 lightDir_o = normalize(fg_in_o.frg_LightPos - fg_in_o.fragPosition);
	vec3 viewDir_o =  normalize(fg_in_o.frg_ViewPos - fg_in_o.fragPosition);
	
   //diffuse color
	vec4 dif_color = texture(texture_diffuse1,fg_in_o.TexCoord);
   //ambient color
	 ambient = 0.2f * dif_color;
	 
	//diffuse light
	 diff = max(dot(lightDir_o,normal_o),0.0);
	 diffuse = material_kd * diff* dif_color;
	
	//specular light
	 reflectDir = reflect(-lightDir_o, normal_o);
	 spec = pow(max(dot(viewDir_o, reflectDir),0.0), 32.0);
	 specular = material_ks * spec* dif_color ;//* spec_color;
	
	result_color = vec4(vec3(ambient + diffuse + specular),1.0) ;
	color = vec4(result_color);//,1.0);
	
	}
}