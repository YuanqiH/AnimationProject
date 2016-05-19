#pragma once


#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\quaternion.hpp>

#define BONE_ID_ROOT 1
#define MAX_CHCOUNT 5 //max child
#define BONE_ABSOLUTE_ANGLE		0x01	/* Bone angle is absolute or relative to parent */
#define BONE_ABSOLUTE_POSITION		0x02	/* Bone position is absolute in the world or relative to the parent */
#define BONE_ABSOLUTE			(BONE_ABSOLUTE_ANGLE | BONE_ABSOLUTE_POSITION)

struct Bone
{
	long	id;							// BONE ID
	char	name[80];					// BONE NAME
	long	flags;						// BONE FLAGS
	// HIERARCHY INFO
	Bone	*parent;					// POINTER TO PARENT BONE
	int 	childCnt;					// COUNT OF CHILD BONES
	Bone	*children[MAX_CHCOUNT];					// POINTER TO CHILDREN

	// TRANSFORMATION INFO
	glm::vec3	b_scale;					// BASE SCALE FACTORS
	glm::vec3	b_rot;						// BASE ROTATION FACTORS
	glm::vec3	b_trans;					// BASE TRANSLATION FACTORS
	glm::vec3	scale;						// CURRENT SCALE FACTORS
	glm::vec3	rot;						// CURRENT ROTATION FACTORS
	glm::vec3	trans;						// CURRENT TRANSLATION FACTORS
	glm::quat	quat;					// QUATERNION USEFUL FOR ANIMATION
	glm::mat4	matrix;		
	glm::mat4  offsetMatrix; // for store offset parameters

		// DOF CONSTRAINTS
	int		min_rx, max_rx;				// ROTATION X LIMITS
	int		min_ry, max_ry;				// ROTATION Y LIMITS
	int		min_rz, max_rz;				// ROTATION Z LIMITS
	float	damp_width, damp_strength;	// DAMPENING SETTINGS
	
};


//suport function

 Bone *boneAddChild(Bone *root, glm::vec3 b_scale, glm::vec3 b_rot, glm::vec3 b_trans , long flags, char *name)
 {
 	Bone *t;
 	int i;
 
 	if (!root) /* If there is no root, create one */
 	{
 		if (!(root = (Bone *)malloc(sizeof(Bone))))
 			return NULL;
 		root->parent = NULL;
 	}
 	else if (root->childCnt < MAX_CHCOUNT) /* If there is space for another child */
 	{
 		/* Allocate the child */
 		if (!(t = (Bone *)malloc(sizeof(Bone))))
 			return NULL; /* Error! */
 
 		t->parent = root; /* Set it's parent */
 		root->children[root->childCnt++] = t; /* Increment the childCounter and set the pointer */
 		root = t; /* Change the root */
 	}
 	else /* Can't add a child */
 		return NULL;
 
 	/* Set data */
 	root->b_scale = b_scale;
 	root->b_rot = b_rot;
 	root->b_trans = b_trans;
	//initialise fro animation
	root->scale = glm::vec3(1);//glm::vec3(0.005f,0.005f,0.005f); // since the model sucks
	root->rot = glm::vec3(0);
	root->trans = glm::vec3(0);
	root->quat = glm::quat(1.0f,0.0f,0.0f,0.0f); // identity

 	root->flags = flags;
 	root->childCnt = 0;
	glm::mat4 matrix;
	glm::mat4 rot;
	rot = glm::rotate(rot, b_rot.x, glm::vec3(1.0f,0.0f,0.0f));
	rot = glm::rotate(rot, b_rot.y, glm::vec3(0.0f,1.0f,0.0f));
	rot = glm::rotate(rot, b_rot.z, glm::vec3(0.0f,0.0f,1.0f));
	if (root->parent != NULL)
	{
	root->offsetMatrix = /* glm::translate(matrix, glm::vec3(root->parent->offsetMatrix[3])) */  glm::translate(matrix, b_trans); 
         // * rot *  glm::scale(matrix, b_scale); // build offset matrix based on parent
	}
	else
	{
		root->offsetMatrix = glm::translate(matrix, b_trans) *
         rot *  glm::scale(matrix, b_scale);
	}

	root->damp_width = 100.0f;// 30 degrees
	

 	if (name)
 		strcpy(root->name, name);
 	else
 		strcpy(root->name, "Bone");
 
 	for (i = 0; i < MAX_CHCOUNT; i++)
 		root->children[i] = NULL;
 
 	return root;
 }

 /* Free the bones */
 Bone *boneFreeTree(Bone *root)
 {
 	int i;
 
 	if (!root)
 		return NULL;
 
 	/* Recursively call this function to free subtrees */
 	for (i = 0; i < root->childCnt; i++)
 		boneFreeTree(root->children[i]);
 
 	free(root);
 
 	return NULL;
 }
 
 /* Dump on stdout the bone structure. Root of the tree should have level 1 */
 void boneDumpTree(Bone *root, int level)
 {
 	int i;
 
 	if (!root)
 		return;
 
 	for (i = 0; i < level; i++)
 		printf("#"); /* We print # to signal the level of this bone. */
 
 	printf(" %4.4f %4.4f %4.4f %d %d %s\n", root->matrix[3].x, root->matrix[3].y,
               root->matrix[3].z, root->childCnt, root->flags, root->name);
 
 	/* Recursively call this on my children */
 	for (i = 0; i < root->childCnt; i++)
 		boneDumpTree(root->children[i], level + 1);
 }
