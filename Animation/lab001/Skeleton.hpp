#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\quaternion.hpp>

#define BONE_ID_ROOT 1

#define CHANNEL_TYPE_NONE	0	
#define CHANNEL_TYPE_SRT	1

static int s_Channel_Type_Size[] = 
{
	0,
	9
};

struct t_Bone
{
	long	id;							// BONE ID
	char	name[80];					// BONE NAME
	long	flags;						// BONE FLAGS
	// HIERARCHY INFO
	t_Bone	*parent;					// POINTER TO PARENT BONE
	int 	childCnt;					// COUNT OF CHILD BONES
	t_Bone	*children;					// POINTER TO CHILDREN
	// TRANSFORMATION INFO
	glm::vec3	b_scale;					// BASE SCALE FACTORS
	glm::vec3	b_rot;						// BASE ROTATION FACTORS
	glm::vec3	b_trans;					// BASE TRANSLATION FACTORS
	glm::vec3	scale;						// CURRENT SCALE FACTORS
	glm::vec3	rot;						// CURRENT ROTATION FACTORS
	glm::vec3	trans;						// CURRENT TRANSLATION FACTORS
	glm::quat	quat;					// QUATERNION USEFUL FOR ANIMATION
	glm::mat4	matrix;						// PLACE TO STORE THE MATRIX

	// ANIMATION INFO
	long	primChanType;				// WHAT TYPE OF PREIMARY CHAN IS ATTACHED
	float	*primChannel;				// POINTER TO PRIMARY CHANNEL OF ANIMATION
	float 	primFrameCount;				// FRAMES IN PRIMARY CHANNEL
	float	primSpeed;					// CURRENT PLAYBACK SPEED
	float	primCurFrame;				// CURRENT FRAME NUMBER IN CHANNEL
	long	secChanType;				// WHAT TYPE OF SECONDARY CHAN IS ATTACHED
	float	*secChannel;				// POINTER TO SECONDARY CHANNEL OF ANIMATION
	float	secFrameCount;				// FRAMES IN SECONDARY CHANNEL
	float	secCurFrame;				// CURRENT FRAME NUMBER IN CHANNEL
	float	secSpeed;					// CURRENT PLAYBACK SPEED
	float	animBlend;					// BLENDING FACTOR (ANIM WEIGHTING)
	// DOF CONSTRAINTS
	int		min_rx, max_rx;				// ROTATION X LIMITS
	int		min_ry, max_ry;				// ROTATION Y LIMITS
	int		min_rz, max_rz;				// ROTATION Z LIMITS
	float	damp_width, damp_strength;	// DAMPENING SETTINGS
	
};

//support function

void DestroySkeleton(t_Bone *root){
		int loop;
		t_Bone *child;
		if (root->childCnt > 0)
		{
			child = root->children;
			for ( loop = 0; loop < root->childCnt; loop++, child++)
			{
				if (child->childCnt > 0)
				{
					DestroySkeleton(child);
				}
				if (child->primChannel > NULL)
				{
					free(child->primChannel);
					child->primChannel = NULL;
				}
			}
			free(root->children);
		}

		root->primChanType = CHANNEL_TYPE_NONE;
		root->secChanType = CHANNEL_TYPE_NONE;
		root->primFrameCount = 0;
		root->secFrameCount = 0;
		root->primCurFrame = 0;
		root->secCurFrame = 0;
		root->primChannel = NULL;
		root->secChannel = NULL;

	}

	void ResetBone(t_Bone *bone,t_Bone *parent){
		
		bone->b_scale = glm::vec3(1.0f);
		bone->scale = glm::vec3(1.0f);

		bone->b_rot = glm::vec3(0.0f);
		bone->rot = glm::vec3(0.0f);

		bone->b_trans = glm::vec3(0.0f);
		bone->trans = glm::vec3(0.0f);


		bone->primChanType = CHANNEL_TYPE_NONE;
		bone->secChanType = CHANNEL_TYPE_NONE;
		bone->primFrameCount = 0;
		bone->secFrameCount = 0;
		bone->primCurFrame = 0;
		bone->secCurFrame = 0;
		bone->primChannel = NULL;
		bone->secChannel = NULL;

		bone->childCnt = 0;						// COUNT OF ATTACHED BONE ELEMENTS
		bone->children = NULL;					// POINTER TO CHILDREN
		bone->parent = parent;
	}


	void BoneSetFrame(t_Bone *bone,int frame){
		// Local Variables ///////////////////////////////////////////////////////////
		float *offset;
		////////////////////////////////////////////////////////////////

		if (bone->primChannel != NULL)
		{
			offset = (float *)(bone->primChannel + (s_Channel_Type_Size[bone->primChanType] * frame));

			switch (bone->primChanType)
			{
			// TYPE_SRT HAS 9 FLOATS IN TXYZ,RXYZ,SXYZ ORDER
			case CHANNEL_TYPE_SRT:
				bone->trans.x = offset[0];
				bone->trans.y = offset[1];
				bone->trans.z = offset[2];

				bone->rot.x = offset[3];
				bone->rot.y = offset[4];
				bone->rot.z = offset[5];

				bone->scale.x = offset[6];
				bone->scale.y = offset[7];
				bone->scale.z = offset[8];
				break;

			}
		}
	}
	void BoneAdvanceFrame(t_Bone *bone,int direction, bool doChildren){
		/// Local Variables ///////////////////////////////////////////////////////////
		int loop;
		t_Bone *child;
		///////////////////////////////////////////////////////////////////////////////
		if (bone->childCnt > 0)
		{
			child = bone->children;
			for (loop = 0; loop < bone->childCnt; loop++,child++)
			{
				child->primCurFrame += direction;
				if (child->primCurFrame >= child->primFrameCount)
					child->primCurFrame = 0;
				if (child->primCurFrame < 0)
					child->primCurFrame += child->primFrameCount;
				BoneSetFrame(child,(int)child->primCurFrame);
				if (doChildren && child->childCnt > 0)				// IF THIS CHILD HAS CHILDREN
					BoneAdvanceFrame(child,direction,doChildren);	// RECURSE DOWN HIER
			}
		}
	}
