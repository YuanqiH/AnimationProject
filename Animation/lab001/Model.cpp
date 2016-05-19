#include "Model.hpp"

bool core::ModelLoader::loadModel(const char* fp, Model* m)
{
    std::cout<<"Loading " + (std::string)fp <<std::endl;

    Assimp::Importer importer; // used to import the model

    const aiScene* scene = importer.ReadFile(fp,
        aiProcess_Triangulate |
        aiProcess_OptimizeMeshes |
        aiProcess_JoinIdenticalVertices |
        //aiProcess_PreTransformVertices |
        aiProcess_FlipUVs);

    if (!scene)
    {
        std::cout<<"Error importing " << (std::string)fp << ": " << importer.GetErrorString()<<std::endl;
        return false;
    }
    
    // get the root fp
    m->rootPath = (std::string)fp;
    for (int x = m->rootPath.size() - 1; x >= 0; x--)
    {
		if (m->rootPath[x] == '/' || m->rootPath[x] == '\\')
			{
				m->rootPath = m->rootPath.substr(0,  x + 1);
				x = -1;
			}
	}

    // some debug stuff, delete later
    std::cout << "Number of total meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Animations: " << scene->HasAnimations() << std::endl;
    
     // set 64 bones to identity, 64 is current limit, might increase it later
	// save all the animatoin date !!!
    processAnimations(scene, m);
 
    // start processing the model, through node to mesh 
    processNode(scene, scene->mRootNode, m);
    
    // must be called after processNode
	// building skeleton from scene 
    if (scene->HasAnimations())
        m->animations[m->currentAnim].buildBoneTree(scene, scene->mRootNode, &m->animations[m->currentAnim].root, m);
        
    m->modelTrans = glm::mat4(1.0f);
 
    m->modelLoaded = true;
 
    std::cout << (std::string)fp + " loaded successfully."<<std::endl;

	m->init();// adding rendering ini() here

    return true;
};

bool core::ModelLoader::GetAnotherAnimation(const char* fp, Model* m){
	std::cout<<"Loading " + (std::string)fp <<std::endl;

    Assimp::Importer importer; // used to import the model

    const aiScene* scene = importer.ReadFile(fp,
        aiProcess_Triangulate |
        aiProcess_OptimizeMeshes |
        aiProcess_JoinIdenticalVertices |
        //aiProcess_PreTransformVertices |
        aiProcess_FlipUVs);

    if (!scene)
    {
        std::cout<<"Error importing " << (std::string)fp << ": " << importer.GetErrorString()<<std::endl;
        return false;
    }

	// some debug stuff, delete later
    std::cout << "Number of total meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Animations: " << scene->HasAnimations() << std::endl;
    
     // set 64 bones to identity, 64 is current limit, might increase it later
	// save all the animatoin date !!!
    processAnimations(scene, m);

}
 
void core::ModelLoader::processAnimations(const aiScene* scene, Model* m)
{
    if (scene->HasAnimations())
    {
        for (int x = 0; x < scene->mNumAnimations; x++)
        {
            Model::Animation tempAnim;
            tempAnim.name = scene->mAnimations[x]->mName.data;
            tempAnim.duration = scene->mAnimations[x]->mDuration;
            tempAnim.ticksPerSecond = scene->mAnimations[x]->mTicksPerSecond;
            //tempAnim.data = scene->mAnimations[x];
 
            // load in required data for animation so that we don't have to save the entire scene
            for (int y = 0; y < scene->mAnimations[x]->mNumChannels; y++)
            {
                Model::Animation::Channel tempChan;
                tempChan.name = scene->mAnimations[x]->mChannels[y]->mNodeName.data;
 
                for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumPositionKeys; z++)
                    tempChan.mPositionKeys.push_back(scene->mAnimations[x]->mChannels[y]->mPositionKeys[z]);
 
                for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumRotationKeys; z++)
                    tempChan.mRotationKeys.push_back(scene->mAnimations[x]->mChannels[y]->mRotationKeys[z]);
 
                for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumScalingKeys; z++)
                    tempChan.mScalingKeys.push_back(scene->mAnimations[x]->mChannels[y]->mScalingKeys[z]);
 
                tempAnim.channels.push_back(tempChan);
            }
 
            m->currentAnim = 0;
 
            for (int z = 0; z < MAX_BONES; z++)
            {
                tempAnim.boneTrans.push_back(glm::mat4(1.0f));
            }
 
            m->animations.push_back(tempAnim);
        }
 
        m->animations[m->currentAnim].root.name = "rootBoneTreeNode";
    }
};
 
void core::ModelLoader::processNode(const aiScene* scene, aiNode* node, Model* m)
{
    std::cout << "Processing a node: " << node->mName.C_Str() << std::endl; //debug
    // this is where the fun part starts.
 
    // cycle through each mesh within this node
    if (node->mNumMeshes > 0)
    {
        // cycle through each mesh
        for (unsigned int x = 0; x < node->mNumMeshes; x++)
        {
 
            processMesh(scene, node,
                scene->mMeshes[node->mMeshes[x]], // scene contains all of the meshes, nodes simply have indices into the scene mesh array
                m);
        }
    }
    if (m->boneID.find(node->mName.data) != m->boneID.end())
        std::cout << node->mName.data << " IS A BONE NODE!!!!";
 
    // then go through each child in the node and process them as well
    if (node->mNumChildren > 0)
    {
        for (unsigned int x = 0; x < node->mNumChildren; x++)
        {
            processNode(scene, node->mChildren[x], m);
        }
    }
 
};
 
// add some error handling (not all models have uvs, etc)
void core::ModelLoader::processMesh(const aiScene* scene, aiNode* node, aiMesh* mesh, Model* m)
{
    std::cout << "Processing a mesh: " << mesh->mName.C_Str() << std::endl; //debug
 
    std::cout << "Has bones? " << mesh->mNumBones << std::endl;

	// prepare all the weights and boneID
    Model::Mesh tempMesh;
    tempMesh.weights.resize(mesh->mNumVertices);
        std::fill(tempMesh.weights.begin(), tempMesh.weights.end(), glm::vec4(0.0f));
    tempMesh.boneID.resize(mesh->mNumVertices);
        std::fill(tempMesh.boneID.begin(), tempMesh.boneID.end(), glm::vec4(-123.0f));
 
    tempMesh.baseModelMatrix = toMat4(&node->mTransformation);
    if (node->mParent != NULL)
        tempMesh.baseModelMatrix = toMat4(&node->mParent->mTransformation) * toMat4(&node->mTransformation);
 
    // cycle through each vertex in the mesh
    for (unsigned x = 0; x < mesh->mNumVertices; x++)
    {
        // load the vertices
        glm::vec3 tempV;
        tempV.x = mesh->mVertices[x].x;
        tempV.y = mesh->mVertices[x].y;
        tempV.z = mesh->mVertices[x].z;
        tempMesh.vertices.push_back(tempV);
 
		if(mesh->HasTextureCoords(x)){
        //	 load the uvs
        glm::vec2 tempUV;
        tempUV.x = mesh->mTextureCoords[0][x].x;
        tempUV.y = mesh->mTextureCoords[0][x].y;
        tempMesh.uvs.push_back(tempUV);
		}
        // load the normals (if they exist)
        if (mesh->HasNormals())
        {
            glm::vec3 tempN;
            tempN.x = mesh->mNormals[x].x;
            tempN.y = mesh->mNormals[x].y;
            tempN.z = mesh->mNormals[x].z;
            tempMesh.normals.push_back(tempN);
        }
    }
 
    // cycle through each face to get the indices
    for (unsigned int x = 0; x < mesh->mNumFaces; x++)
    {
        // ALWAYS USE AIPROCESS_TRIANGULATE!!! not doing so will make all of the indices wrong!!!
        tempMesh.indices.push_back(mesh->mFaces[x].mIndices[0]);
        tempMesh.indices.push_back(mesh->mFaces[x].mIndices[1]);
        tempMesh.indices.push_back(mesh->mFaces[x].mIndices[2]);
    }
 
    if (scene->HasMaterials())
    {
        // so that we don't have to type out that whole thing every time
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        std::cout << "Has diffuse texture: " << mat->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
 
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            // don't know why you have to get the texture name like this
            aiString path;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            std::cout << path.C_Str() << std::endl;
 
			std::string finalFP = m->rootPath + path.C_Str();
            tempMesh.image = SOIL_load_image(finalFP.c_str(), &tempMesh.width, &tempMesh.height, 0, SOIL_LOAD_RGB);
            
            if (tempMesh.image == NULL)
				std::cout << SOIL_last_result()<< std::endl;
        }
    }
 
    if (mesh->HasBones())
    {
        for (int x = 0; x < mesh->mNumBones; x++)
        {
            // bone index, decides what bone we modify
            unsigned int index = 0;
 
            if (m->boneID.find(mesh->mBones[x]->mName.data) == m->boneID.end())
            { // create a new bone
                // current index is the new bone
                index = m->boneID.size();
            }
            else
            {
                index = m->boneID[mesh->mBones[x]->mName.data];
            }
 
            m->boneID[mesh->mBones[x]->mName.data] = index;
 
            for (int y = 0; y < m->animations[m->currentAnim].channels.size(); y++) // get the offset matrix from aiBone
            {
                if (m->animations[m->currentAnim].channels[y].name == mesh->mBones[x]->mName.data)
                    m->animations[m->currentAnim].boneOffset[mesh->mBones[x]->mName.data] = toMat4(&mesh->mBones[x]->mOffsetMatrix);
            }
 
 
            for (int y = 0; y < mesh->mBones[x]->mNumWeights; y++)
            {
                unsigned int vertexID = mesh->mBones[x]->mWeights[y].mVertexId;
                // first we check if the boneid vector has any filled in
                // if it does then we need to fill the weight vector with the same value
                if (tempMesh.boneID[vertexID].x == -123)
                {
                    tempMesh.boneID[vertexID].x = index;
                    tempMesh.weights[vertexID].x = mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].y == -123)
                {
                    tempMesh.boneID[vertexID].y = index;
                    tempMesh.weights[vertexID].y =  mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].z == -123)
                {
                    tempMesh.boneID[vertexID].z = index;
                    tempMesh.weights[vertexID].z = mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].w == -123)
                {
                    tempMesh.boneID[vertexID].w = index;
                    tempMesh.weights[vertexID].w = mesh->mBones[x]->mWeights[y].mWeight;
                }
            }
           
        }
    }
    m->meshes.push_back(tempMesh);
};
 
void core::Model::Animation::buildBoneTree(const aiScene* scene, aiNode* node, BoneNode* bNode, Model* m)
{
    if (scene->HasAnimations())
    {
        // found the node
        if (m->boneID.find(node->mName.data) != m->boneID.end())
        {
            std::cout << "Found a bone node: " << node->mName.data << std::endl;
            BoneNode tempNode;
            tempNode.name = node->mName.data;
            tempNode.parent = bNode;
            tempNode.nodeTransform = toMat4(&node->mTransformation);
            // bones and their nodes always share the same name
            tempNode.boneTransform = boneOffset[tempNode.name];
            bNode->children.push_back(tempNode);
        }
 
        if (node->mNumChildren > 0)
        {
            for (unsigned int x = 0; x < node->mNumChildren; x++)
            {
                // if the node we just found was a bone node then pass it in (current bone node child vector size - 1)
                if (m->boneID.find(node->mName.data) != m->boneID.end())
                    buildBoneTree(scene, node->mChildren[x], &bNode->children[bNode->children.size() - 1], m);
                else
                    buildBoneTree(scene, node->mChildren[x], bNode, m);
            }
        }
    }
};


// used a better interpolate method

void core::Model::tick(double deltatime, Model::Animation::BoneNode* skeleton_refer)
{
    double timeInTicks = deltatime * animations[currentAnim].ticksPerSecond ;
	this->currentAnim = 1;
    //updateBoneTree(timeInTicks, &animations[currentAnim].root, glm::mat4(1.0f));
	updateBoneTree(timeInTicks, skeleton_refer, glm::mat4(1.0f));
};



void core::Model::updateBoneTree(double timeInTicks, Model::Animation::BoneNode* node, glm::mat4 parentTransform)
{
    int chanIndex = 0;
    for (int x = 0; x < animations[currentAnim].channels.size(); x++)
    {
        if (node->name == animations[currentAnim].channels[x].name)
        {
            chanIndex = x;
        }
    }

	double animTime = std::fmod(timeInTicks, animations[currentAnim].duration * animations[currentAnim].ticksPerSecond );
	std::cout<< animTime << std::endl; //debug the time
    aiQuaternion aiRotation(animations[currentAnim].channels[chanIndex].mRotationKeys[0].mValue);
    aiVector3D aiTranslation(animations[currentAnim].channels[chanIndex].mPositionKeys[0].mValue);
    aiVector3D aiScale(animations[currentAnim].channels[chanIndex].mScalingKeys[0].mValue);
 
    Assimp::Interpolator<aiQuaternion> slerp;
    Assimp::Interpolator<aiVector3D> lerp;
 
	// using the keyframe period is not better than treating the period equally
	
	int keysize = animations[currentAnim].channels[chanIndex].mPositionKeys.size();
	 int key1, key2;
	 double lowerTimeTicker, upperTimeTicker;
	 double Factor;
	for (int i = 0; i < keysize-1; i++)
	{
		 lowerTimeTicker = animations[currentAnim].channels[chanIndex].mPositionKeys[i].mTime ;
		 upperTimeTicker = animations[currentAnim].channels[chanIndex].mPositionKeys[i+1].mTime;
		if (animTime > lowerTimeTicker && animTime <= upperTimeTicker)
		{
			key1 = i;
			key2 = i+1;
			break;
		}
		if (animTime <= animations[currentAnim].channels[chanIndex].mPositionKeys[0].mTime ) // handle anim not from the 0 second
		{
			key1 = 0;
			key2 = 0;
			lowerTimeTicker = animTime;
			break;	
		}
		if (animTime > animations[currentAnim].channels[chanIndex].mPositionKeys[keysize-1].mTime) // handle anim end before last keyframe
		{
			key1 = keysize-1;
			key2 = keysize-1;
			upperTimeTicker = animTime;
			break;
		}
	}
	
	Factor = (animTime - lowerTimeTicker)/(upperTimeTicker-lowerTimeTicker);
	assert(Factor >= 0.0 && Factor <= 1.0);

	/* // old method interpolate wrongly 
    // get the two animation keys it is between for lerp and slerp
    int key1, key2;
    if (glm::round(animTime) < animTime)
    {
        key1 = glm::round(animTime); key2 = key1 + 1;
    }
    else
    {
        key1 = glm::round(animTime) - 1; key2 = glm::round(animTime);
    }
	*/
    if (animations[currentAnim].channels[chanIndex].mPositionKeys.size() > 1)
        lerp(aiTranslation, animations[currentAnim].channels[chanIndex].mPositionKeys[key1].mValue, animations[currentAnim].channels[chanIndex].mPositionKeys[key2].mValue, /*animTime - key1);*/Factor); // translation
    if (animations[currentAnim].channels[chanIndex].mScalingKeys.size() > 1)
        lerp(aiScale, animations[currentAnim].channels[chanIndex].mScalingKeys[key1].mValue, animations[currentAnim].channels[chanIndex].mScalingKeys[key2].mValue, /*animTime - key1);*/Factor); // scale
    if (animations[currentAnim].channels[chanIndex].mRotationKeys.size() > 1)
        slerp(aiRotation, animations[currentAnim].channels[chanIndex].mRotationKeys[key1].mValue, animations[currentAnim].channels[chanIndex].mRotationKeys[key2].mValue, /* animTime - key1);*/Factor); // rotation
 
    glm::vec3 translation((GLfloat)aiTranslation.x, (GLfloat)aiTranslation.y, (GLfloat)aiTranslation.z);
    glm::vec3 scaling((GLfloat)aiScale.x, (GLfloat)aiScale.y, (GLfloat)aiScale.z);
    glm::quat rotation((GLfloat)aiRotation.w, (GLfloat)aiRotation.x, (GLfloat)aiRotation.y, (GLfloat)aiRotation.z);
 
    glm::mat4 finalModel =
		   parentTransform
        * glm::translate(glm::mat4(1.0f), translation)
        * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scaling);
	
	// set all the acumulative transform into indentity to check the offset matrix
	//finalModel = glm::rotate(finalModel, (float)(200.0f * animTime),glm::vec3(0.0f,1.0f,0.0f));
        
    animations[currentAnim].boneTrans[boneID[node->name]] = finalModel * node->boneTransform;//animations[currentAnim].boneOffset[node->name];
 
    // loop through every child and use this bone's transformations as the parent transform
    for (int x = 0; x < node->children.size(); x++)
    {
        updateBoneTree(timeInTicks, &node->children[x], finalModel);
    }
};
 
core::Model::Model(GLuint shaderID)//const char* vfp, const char* ffp)
{
	//passing id from outside
    shader = shaderID;//core::loadShader(vfp, ffp);
    modelLoaded = false;
};
 
void core::Model::setShader(GLuint shaderID)//const char* vfp, const char* ffp)
{
    shader = shaderID;//core::loadShader(vfp, ffp);
	
};
 
// this is just a generic render function for quick and easy rendering
void core::Model::render(glm::mat4 prjectionMatrix, glm::mat4 viewMatrix, glm::mat4 modelTrans, core::Model::Animation* render_animation, Camera* cam) //, core::Window* win)
{
    if (!modelLoaded)
    {
        //core::log("Please load in a model before trying to render one.", core::warning);
        return;
    }
 
    glUseProgram(shader);
    for (int x = 0; x < meshes.size(); x++)
    {
        glBindVertexArray(meshes[x].vao);
 
       // cam->Tick(win->getWindow(), dt);
		//===================================================
		// update the matrix 
		//tick(dt);
		//===================================================
        glUniformMatrix4fv(meshes[x].modelID, 1, GL_FALSE, &meshes[x].baseModelMatrix[0][0]);
        glUniformMatrix4fv(meshes[x].viewID, 1, GL_FALSE, &viewMatrix[0][0]);//&cam->GetView()[0][0]);
        glUniformMatrix4fv(meshes[x].projectionID, 1, GL_FALSE, &prjectionMatrix[0][0]);//&cam->GetProjection()[0][0]);
       //glUniformMatrix4fv(meshes[x].transID, animations[currentAnim].boneTrans.size(), GL_FALSE, (GLfloat*)&animations[currentAnim].boneTrans[0][0]);
		 glUniformMatrix4fv(meshes[x].transID, render_animation->boneTrans.size(), GL_FALSE, (GLfloat*)&render_animation->boneTrans[0][0]);
        glUniformMatrix4fv(meshes[x].modelTransID, 1, GL_FALSE, /*&modelTrans[0][0]);*/(GLfloat*)&this->modelTrans[0][0]);
		glUniform3fv(glGetUniformLocation(shader,"eyeP"),1,&cam->CamPosition[0]);


        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].vbo);
        glEnableVertexAttribArray(meshes[x].posAttribute);
        glVertexAttribPointer(meshes[x].posAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
 
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].uvb);
        glEnableVertexAttribArray(meshes[x].texAttribute);
        glVertexAttribPointer(meshes[x].texAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
 
        glBindTexture(GL_TEXTURE_2D, meshes[x].tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, meshes[x].width, meshes[x].height, 0, GL_RGB, GL_UNSIGNED_BYTE, meshes[x].image);
        glUniform1i(glGetUniformLocation(shader, "tex"), 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
        glDrawElements(GL_TRIANGLES, meshes[x].indices.size(), GL_UNSIGNED_INT, 0);
 
        glBindVertexArray(0);
    }
};
// why even have this as an external function? just call it in loadModel
void core::Model::init()
{
    if (!modelLoaded)
    {
        std::cout<<"Please load in a model before initializing buffers."<<std::endl;
        return;
    }
 
    // loop through each mesh and initialize them
    for (int x = 0; x < meshes.size(); x++)
    {
        glGenVertexArrays(1, &meshes[x].vao);
        glBindVertexArray(meshes[x].vao);
 
        glGenBuffers(1, &meshes[x].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * meshes[x].vertices.size(), &meshes[x].vertices[0], GL_STATIC_DRAW);
 
        glGenBuffers(1, &meshes[x].ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[x].ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * meshes[x].indices.size(), &meshes[x].indices[0], GL_STATIC_DRAW);
 
		if(meshes[x].uvs.size() != 0){
        glGenBuffers(1, &meshes[x].uvb);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].uvb);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * meshes[x].uvs.size(), &meshes[x].uvs[0], GL_STATIC_DRAW);
		}
		
		glGenBuffers(1, &meshes[x].nbo);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].nbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * meshes[x].normals.size(), &meshes[x].normals[0], GL_STATIC_DRAW);
		

        glGenBuffers(1, &meshes[x].wbo);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].wbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * meshes[x].weights.size(), &meshes[x].weights[0], GL_STATIC_DRAW);
 
        glGenBuffers(1, &meshes[x].idbo);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].idbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * meshes[x].boneID.size(), &meshes[x].boneID[0], GL_STATIC_DRAW);
 
        glGenTextures(1, &meshes[x].tex);
        glBindTexture(GL_TEXTURE_2D, meshes[x].tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, meshes[x].width, meshes[x].height, 0, GL_RGB, GL_UNSIGNED_BYTE, meshes[x].image);
        // tex data bound to uniform
        glUniform1i(glGetUniformLocation(shader, "tex"), 0);

        // now send uv data
		if(meshes[x].uvs.size() != 0){
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].uvb);
        meshes[x].texAttribute = glGetAttribLocation(shader, "Texcoord");
        glEnableVertexAttribArray(meshes[x].texAttribute);
        glVertexAttribPointer(meshes[x].texAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].vbo);
        meshes[x].posAttribute = glGetAttribLocation(shader, "position");
        glEnableVertexAttribArray(meshes[x].posAttribute);
        glVertexAttribPointer(meshes[x].posAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
		glBindBuffer(GL_ARRAY_BUFFER, meshes[x].nbo);
        meshes[x].normalAttribute = glGetAttribLocation(shader, "Normal");
        glEnableVertexAttribArray(meshes[x].normalAttribute);
        glVertexAttribPointer(meshes[x].normalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
		

        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].wbo);
        meshes[x].weightAttribute = glGetAttribLocation(shader, "weight");
        glEnableVertexAttribArray(meshes[x].weightAttribute);
        glVertexAttribPointer(meshes[x].weightAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
 
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].idbo);
        meshes[x].boneAttribute = glGetAttribLocation(shader, "boneID");
        glEnableVertexAttribArray(meshes[x].boneAttribute);
        glVertexAttribPointer(meshes[x].boneAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
		//glVertexAttribIPointer(meshes[x].boneAttribute, 4, GL_INT, 0, 0);
	
        meshes[x].modelID = glGetUniformLocation(shader, "model");
        meshes[x].viewID = glGetUniformLocation(shader, "view");
        meshes[x].projectionID = glGetUniformLocation(shader, "projection");
        meshes[x].transID = glGetUniformLocation(shader, "boneTransformation");
        meshes[x].modelTransID = glGetUniformLocation(shader, "modelTransform");
 
        glBindVertexArray(0);
    }
};
 
 void core::Model::setModelTrans (glm::mat4 in)
 {
	 modelTrans = in;
 };
 
glm::mat4 core::toMat4(aiMatrix4x4* ai)
{
    glm::mat4 mat;
 
    mat[0][0] = ai->a1; mat[1][0] = ai->a2; mat[2][0] = ai->a3; mat[3][0] = ai->a4;
    mat[0][1] = ai->b1; mat[1][1] = ai->b2; mat[2][1] = ai->b3; mat[3][1] = ai->b4;
    mat[0][2] = ai->c1; mat[1][2] = ai->c2; mat[2][2] = ai->c3; mat[3][2] = ai->c4;
    mat[0][3] = ai->d1; mat[1][3] = ai->d2; mat[2][3] = ai->d3; mat[3][3] = ai->d4;
 
    return mat;
}