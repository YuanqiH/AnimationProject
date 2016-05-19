#ifndef MODEL//_LOADER
#define MODEL//_LOADER

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\quaternion.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <SOIL.h>

#include "Camera.hpp"

#define MAX_BONES 64

namespace core
{
    struct Model
    {
        struct Mesh
        {
            std::vector <glm::vec3> vertices;
            std::vector <unsigned int> indices;
            std::vector <glm::vec2> uvs;
            std::vector <glm::vec3> normals;
            std::vector <glm::vec4> weights;
            std::vector <glm::vec4> boneID;
 
            glm::mat4 baseModelMatrix;
 
            GLuint vao, vbo, ebo, uvb, tex, wbo, idbo, nbo;
            GLint posAttribute, texAttribute, weightAttribute, boneAttribute, normalAttribute;
            GLuint modelID, viewID, projectionID, transID, modelTransID;
 
            int width, height;
            unsigned char* image;
        };
	
        struct Animation
        {
            std::string name;
            double duration;
            double ticksPerSecond;
            // all of the bone transformations, this is modified every frame
            // assimp calls it a channel, its anims for a node aka bone
            std::vector <glm::mat4> boneTrans;
            std::map <std::string, glm::mat4> boneOffset;
 
            struct Channel
            {
                std::string name;
                glm::mat4 offset;
                std::vector <aiVectorKey> mPositionKeys;
                std::vector <aiQuatKey> mRotationKeys;
                std::vector <aiVectorKey> mScalingKeys;
            };
            std::vector <Channel> channels;
 
            struct BoneNode
            {
                std::string name;
                BoneNode* parent;
                std::vector <BoneNode> children;
                glm::mat4 nodeTransform;
                glm::mat4 boneTransform;
            };
            BoneNode root;
 
            void buildBoneTree(const aiScene* scene, aiNode* node, BoneNode* bNode, Model* m);
 
            //aiAnimation* data;
        };
        // all of the animations
        std::vector<Animation> animations;
        unsigned int currentAnim;
            void setAniamtion(std::string name);
        std::vector <std::string> animNames;
        // map of bones
        std::map <std::string, unsigned int> boneID;

		// each animation upading 
		//======================================
        // runs every frame
        void tick(double time,  Model::Animation::BoneNode* node);
        void updateBoneTree(double time, Animation::BoneNode* node, glm::mat4 transform);
            
        glm::mat4 modelTrans;
       //GLuint modelTransID; moved to mesh
        void setModelTrans(glm::mat4);
       
       std::string rootPath;
 
        //todo: make init and render optional
        std::vector <Mesh> meshes;
        GLuint shader;
        bool modelLoaded;
		// passing the ID from outside define, load from outside
        Model(GLuint shaderID);//const char* vertfp, const char* fragfp);
        void setShader(GLuint shaderID);//const char* vertfp, const char* fragfp);

		// render the animation
		// ===================================
        void init();
        void render(glm::mat4 prjectionMatrix, glm::mat4 viewMatrix,glm::mat4 modelTrans, core::Model::Animation* render_animation, Camera * cam);// core::Window* win);
    };
 
    class ModelLoader
    {
    private:
        void processNode(const aiScene* scene, aiNode* node, Model* m);
        void processMesh(const aiScene* scene, aiNode* node, aiMesh* mesh, Model* m);
        void processAnimations(const aiScene* scene, Model* m);
 
    public:
        // this will load all of the required data and dump it into the model struct
        bool loadModel(const char* fp, Model* m);
		bool GetAnotherAnimation(const char* fp, Model* m);
    };
 
    glm::mat4 toMat4(aiMatrix4x4* ai);
}
#endif