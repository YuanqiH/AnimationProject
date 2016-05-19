#pragma once

#include "bone.hpp"
#include "Model_loader.h" // not so good one

#define M_PI 3.1415926535897932384626433832795
#define IK_POS_THRESH 1.0f
#define MAX_IK_TRIES 100 // TIMES THROUGH THE CCD LOOP
#define FRAME_TIME 2
//////////////////////////////////////////////////////////
const int BONE_NUM = 5;
bool isCCD = false;
bool isPause = false;

struct animationInfo
{
	glm::quat ani_quat[2];
	glm::vec3 ani_tran[2];
	glm::vec3 ani_scale[2];
	//only save 2 frame for simplicity 
};

animationInfo m_aniInfo[BONE_NUM];

class SkeletonSim
{
public:
	SkeletonSim();
	~SkeletonSim();
	void initial(Shader *shaderID);
	void skeletonDraw(Shader *shaderID);
	bool ComputeCCD(glm::vec3 endPos);
	Bone *hand;
	void Animation(float localDT, glm::vec3 endPoint, Shader *shaderID);

private:
	Bone *root, *temp, * temp2;
	int boneCout;
	Shader *m_shaderID;
	
	Model bone[BONE_NUM]; 
	void skeletondraw(Bone *root, glm::mat4 ParentMatrix, bool drawOn);
	
	glm::quat rotateBetweenVectors(glm::vec3 start, glm::vec3 dest);
	float TimerSpeed, CurrentTime;
};

SkeletonSim::SkeletonSim()
{
}

SkeletonSim::~SkeletonSim()
{
}

void SkeletonSim::initial(Shader *shaderID){

	glm::vec3 scale_vector = glm::vec3(1);//= glm::vec3(0.005f,0.005f,0.005f);
	for (int i = 0; i < BONE_NUM-1; i++)
	{
		bone[i].load("../models/bone/bone.3ds");  // FEMUR.3ds");
	}
	bone[BONE_NUM-1].load("../models/hand/hand1.3ds");

	if (!(root = boneAddChild(NULL, scale_vector , glm::vec3(0.0f), glm::vec3(0.0f), 0, "body")))
 	{
 		fprintf(stderr, "Error! Can't create a root!\n");
 		exit(EXIT_FAILURE);
 	}

	temp = boneAddChild(root, scale_vector, glm::vec3(0.0f), glm::vec3(0.0f,0.0f,0.0f), BONE_ABSOLUTE, "upperArm");
	temp2 = boneAddChild(temp, scale_vector, glm::vec3(0.0f),  glm::vec3(4.0f,0.0f,0.0f), 0, "lowerArm");
	temp = boneAddChild(temp2, scale_vector, glm::vec3(0.0f, 0.0f, 0.0f ),  glm::vec3(4.0f,0.0f,0.0f), 0, "hand");
	temp2 = boneAddChild(temp, scale_vector, glm::vec3(0.0f),  glm::vec3(4.0f,0.0f,0.0f), 0, "finer");
	hand = temp2;
	boneDumpTree(root, 0);
	//root = boneFreeTree(root);

	TimerSpeed = 0.2f;
	CurrentTime = 0;
	//initial 
	for (int i = 0; i < BONE_NUM; i++)
	{
	m_aniInfo[i].ani_scale[0] = m_aniInfo[i].ani_scale[1] = glm::vec3(1);
	m_aniInfo[i].ani_tran[0] = m_aniInfo[i].ani_tran[1] = glm::vec3(0);
	m_aniInfo[i].ani_quat[0] = m_aniInfo[i].ani_quat[1] = glm::quat(1.0f,0.0f,0.0f,0.0f); // identity
	}

	m_shaderID = shaderID;
	skeletondraw(root, glm::mat4(1.0), false); // initial matrix

}

void SkeletonSim::skeletonDraw(Shader *shaderID){
	boneCout = 0;
	m_shaderID = shaderID;
	skeletondraw(root, glm::mat4(1.0), true);
	//boneDumpTree(root, 0);
}

void SkeletonSim::skeletondraw(Bone *root, glm::mat4 parentMatrix, bool drawOn){
	
	/*
	glm::quat m_quaternionz = glm::angleAxis(root->rot.z,glm::vec3(0.0f,0.0f,1.0f));
	glm::quat m_quaterniony = glm::angleAxis(root->rot.y, glm::vec3(0.0f,1.0f,0.0f));
	glm::quat m_quaternionx = glm::angleAxis(root->rot.x, glm::vec3(1.0f,0.0f,0.0f));
	glm::quat temp =  m_quaternionx * m_quaterniony * m_quaternionz;
	
	 root->quat = temp;
	 */
	//for animation 
	glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), root->trans)
        * glm::toMat4(root->quat) *  glm::scale(glm::mat4(1.0), root->scale);
	/*
	//update for animation in start keyframe;
	m_aniInfo[boneCout].ani_quat[1] = root->quat;
	m_aniInfo[boneCout].ani_scale[1] = root->scale;
	m_aniInfo[boneCout].ani_tran[1] = root->trans;
	*/
	//finalMatrix = glm::mat4(1.0);

	//for locatoin 
	root->matrix = parentMatrix * root->offsetMatrix * localMatrix ;
	//send this to shader for rendering 
	glm::mat4 scaleM = glm::scale(glm::mat4(1), glm::vec3(1.005f,1.005f,1.005f));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderID->shaderProgramID, "scale"), 1, GL_FALSE, &scaleM[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderID->shaderProgramID, "BoneModel"), 1, GL_FALSE, &root->matrix[0][0]);

	if (boneCout < BONE_NUM && drawOn == true)
	{
		
		bone[boneCout++].Draw(*m_shaderID);
		//cout<<"drawing bone : "<< root->name << endl;
		
	}
	else{
		if (boneCout > BONE_NUM )
			{
			cout<<"drawing number is "<< boneCout<<endl;
			return;
			}
	}

	for(int i = 0; i < root->childCnt; i++)
	{
		skeletondraw(root->children[i], root->matrix, drawOn); // send this matrix as parentMatrix
	}

}


bool SkeletonSim::ComputeCCD(glm::vec3 endPos){
	//local varaibles
	glm::vec3 rootPos, curEnd, desiredEnd, TargetVector, curVector, CrossResult;
	float cosAngle, turnAngle, trunDeg;
	Bone *rootPrt, *workPrt;
	int tries;
	//start from the hand pointer;
	rootPrt = hand->parent;
	workPrt = hand;
	tries = 0;
	do
	{
		rootPos = glm::vec3(rootPrt->matrix[3]); //take this as the joint point now 

		curEnd = glm::vec3(hand->matrix[3]);

		desiredEnd = endPos;
		float dist = glm::distance(curEnd,desiredEnd);
		//cout<< "the distance is: "<<dist<<endl;
		//see if i am already close enough
		if( dist > IK_POS_THRESH )
		{
			//CREATE THE VECTOR TO THE CURRENT EFFETOR
			curVector = glm::normalize(curEnd - rootPos);
			TargetVector = glm::normalize(endPos - rootPos);
			cosAngle = glm::dot(curVector, TargetVector);
			
			if (cosAngle < 0.9999)
			{
				
				turnAngle = glm::acos(cosAngle); // [0, pi]
				trunDeg = glm::degrees(turnAngle);
				CrossResult = glm::normalize(glm::cross(curVector, TargetVector));

				if (trunDeg > rootPrt->damp_width)
				{
					trunDeg = rootPrt->damp_width;
				}
		
			
			glm::quat rotation = glm::angleAxis( turnAngle, CrossResult); // radias 
			
			glm::quat rotation_t = rotateBetweenVectors(curVector, TargetVector);
			//need to do the damping and restriction
			rootPrt->quat *= rotation_t;// * rootPrt->quat; // is the limit needed? 
			

			//update 
			boneCout = 0;
			skeletondraw(root, glm::mat4(1), false);
			//isPause = true; // debug every iteration
			//boneDumpTree(root, 0);
			}

			workPrt = rootPrt;
			rootPrt = rootPrt->parent; // go back 
			
			if (rootPrt->parent == NULL)
				rootPrt = hand->parent; // is pointer reach the root, restart
		}


	} while (tries++ < MAX_IK_TRIES && 
		glm::distance(curEnd,desiredEnd) > IK_POS_THRESH);
	//boneDumpTree(root, 0);
	return true;
}


glm::quat SkeletonSim::rotateBetweenVectors(glm::vec3 start, glm::vec3 dest){

 	start = normalize(start);
 	dest = normalize(dest);
 
 	float cosTheta = dot(start, dest);
 	glm::vec3 rotationAxis;
 
 	if (cosTheta < -1 + 0.001f){
 		// special case when vectors in opposite directions:
 		// there is no "ideal" rotation axis
 		// So guess one; any will do as long as it's perpendicular to start
 		rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
 		if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
 			rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
 
 		rotationAxis = normalize(rotationAxis);
 		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
 	}
 
 	rotationAxis = cross(start, dest);
 
 	float s = sqrt( (1+cosTheta)*2 );
 	float invs = 1 / s;
 
 	return glm::quat(
 		s * 0.5f, 
 		rotationAxis.x * invs,
 		rotationAxis.y * invs,
 		rotationAxis.z * invs
 	);
 
}

void SkeletonSim::Animation(float localDT, glm::vec3 endPoint, Shader *shaderID){
	m_shaderID = shaderID;
	//initialise 
	//boneCout = 0;
	//skeletondraw(root, glm::mat4(1), false);

	CurrentTime += TimerSpeed;
	if( CurrentTime > FRAME_TIME){
	//only get renew the second keyframe in certain time
		CurrentTime = 0; // reset timer
		Bone * anibonePrt = root;
		
		ComputeCCD(endPoint);
		for (int i = 0; i < BONE_NUM; i++)
		{
		m_aniInfo[i].ani_quat[0] = m_aniInfo[i].ani_quat[1];
		m_aniInfo[i].ani_scale[0] = m_aniInfo[i].ani_scale[1];
		m_aniInfo[i].ani_tran[0] = m_aniInfo[i].ani_tran[1]; 

		//update for animation in start keyframe;
		m_aniInfo[i].ani_quat[1] = anibonePrt->quat;
		m_aniInfo[i].ani_scale[1] = anibonePrt->scale;
		m_aniInfo[i].ani_tran[1] = anibonePrt->trans;
		anibonePrt = anibonePrt->children[0];

		}
		
	}
	// draw the animation !!!
	glm::mat4 parent = glm::mat4(1);
	Bone * anibonePrt = root;

	for (int i = 0; i < BONE_NUM; i++) // for everybone
	{
		
	glm::quat ani_quat_local = glm::mix(m_aniInfo[i].ani_quat[0], m_aniInfo[i].ani_quat[1], CurrentTime/FRAME_TIME);
	glm::vec3 ani_trans_local =glm::mix(m_aniInfo[i].ani_tran[0], m_aniInfo[i].ani_tran[1], CurrentTime/FRAME_TIME);
	glm::vec3 ani_scale_local =glm::mix(m_aniInfo[i].ani_scale[0], m_aniInfo[i].ani_scale[1], CurrentTime/FRAME_TIME);

	glm::mat4 ani_localMatrix = glm::translate(glm::mat4(1.0f), ani_trans_local)
    * glm::toMat4(ani_quat_local) *  glm::scale(glm::mat4(1.0), ani_scale_local);

	glm::mat4 totalMatrix = parent * anibonePrt->offsetMatrix * ani_localMatrix;
	//pass to shader and draw
	glm::mat4 scaleM = glm::scale(glm::mat4(1), glm::vec3(1.005f,1.005f,1.005f));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderID->shaderProgramID, "scale"), 1, GL_FALSE, &scaleM[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderID->shaderProgramID, "BoneModel"), 1, GL_FALSE, &totalMatrix[0][0]);

	if ( i>0 )
	{
		bone[i].Draw(*m_shaderID);
	}
		

	//update parameters
	parent = totalMatrix;
	anibonePrt = anibonePrt->children[0];
	}
	


}