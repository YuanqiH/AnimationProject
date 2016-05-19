#version 330 

layout (location = 0) in vec2 Texcoord; 
layout (location = 1) in vec3 position; 
layout (location = 2) in vec3 Normal; 
layout (location = 3) in vec4 weight;
layout (location = 4) in vec4 boneID;


out vec2 TexCoord0;
out vec3 Normal0; 
out vec3 WorldPos0; 
out vec3 WorldLightP;
out vec4 DebugBoneColor;

const int MAX_BONES = 100;

uniform mat4 gWVP;
uniform mat4 gWorld;
uniform mat4 gBones[MAX_BONES];

uniform mat4 model; // base model
uniform mat4 view;
uniform mat4 projection;
uniform mat4 boneTransformation[MAX_BONES];
uniform mat4 modelTransform; // world model

void main()
{ 
	DebugBoneColor = vec4( weight.xyz, 1.0);
    mat4 BoneTransform = boneTransformation[int(boneID.x)] * weight.x;
    BoneTransform += boneTransformation[int(boneID.y)] * weight.y;
    BoneTransform += boneTransformation[int(boneID.z)] * weight.z;
    BoneTransform += boneTransformation[int(boneID.w)] * weight.w;

    vec4 PosL = BoneTransform * vec4(position, 1.0);
    gl_Position = projection * view * modelTransform * model /*vec4(position, 1.0); */* PosL;
	
    TexCoord0 = Texcoord;
    vec4 NormalL = BoneTransform * vec4(Normal, 0.0);
    Normal0 = (transpose(inverse(modelTransform * model)) * NormalL).xyz;
    WorldPos0 = (modelTransform * model * PosL).xyz; 
	
	
}