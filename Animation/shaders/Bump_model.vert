#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;


uniform mat4 modelMat;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float dragging;
uniform vec3 lightPos;
//const vec3 lightPos = vec3(4.0,4.0,4.0);
//tangent vectors
out VS_OUT{
	
	vec2 TexCoords;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
	
} vs_out;
//original vectors
out VS_OUT_O{
	
	vec2 TexCoord;
	vec3 fragPosition;
	vec3 frg_normal;
	vec3 frg_LightPos;
	vec3 frg_ViewPos;
	
} vs_out_o;

void main()
{
	
    gl_Position = projection * view * modelMat * vec4(position, 1.0f);
	//std::cout<<dragging<<std::endl;
	
	//original shader vectors
    vs_out_o.TexCoord = texCoords;
	vs_out_o.frg_normal = normalize(vec3(transpose(inverse(modelMat)) * vec4(normal, 1.0f)));
	//vs_out_o.frg_normal = normal;
	vs_out_o.fragPosition = vec3( modelMat * vec4(position, 1.0f));
	vs_out_o.frg_LightPos = vec3(/*modelMat */ vec4(lightPos,1.0));
	vs_out_o.frg_ViewPos = vec3(/*modelMat */ vec4(viewPos,1.0));
	
	
	// transform position into world space
	vs_out.TexCoords = texCoords;
	
	vec3 T = normalize(vec3( modelMat * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(transpose(inverse(modelMat)) * vec4(normal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(T, N);
	mat3 TBN = transpose(mat3(T, B, N));  
	//mat3 TBN = mat3(T,B,N);
	//transform all the relative vectors into TBN space
	vs_out.TangentLightPos = TBN * vec3( /*modelMat  */ vec4(lightPos,1.0));
    vs_out.TangentViewPos  = TBN * vec3( /*modelMat */ vec4(viewPos,1.0));
    vs_out.TangentFragPos  = TBN * vec3(  modelMat * vec4(position, 0.0));
	
}