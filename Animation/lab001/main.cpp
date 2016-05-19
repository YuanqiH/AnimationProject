#include <iostream>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtx\associated_min_max.hpp>
#include <glm/gtc/type_ptr.hpp>


//GL include


//#include "Model_loader.h"
//#include "skeletonSim.hpp"
#include "loadTexture.h"
#include "cube.h"
#include "Floor.h"
#include "ShadowMap.h"
#include "CubeMap.hpp"

// for animation
#include "Model.hpp" 
#include "Camera.hpp"
#include "player.hpp"
#include "Npc.hpp"
#include "Object.hpp"
#include "LevelManager.hpp"

/*
glm::vec3 CubicInterpolate(glm::vec3 point0, glm::vec3 point1, glm::vec3 point2,glm::vec3 point3, float time);
void MovePoint(glm::vec3 *point, float delt);
void drawEndPoint(Shader &shader);
void RenderScene(Shader &shader);
void RenderQuad();
glm::vec3 get_ray_from_mouse(float mouse_x,float mouse_y);
glm::vec3 get_mouse_move(float speed, glm::vec3 currentPosition);
bool ray_sphere (
	glm::vec3 ray_origin_wor,
	glm::vec3 ray_direction_wor,
	glm::vec3 sphere_centre_wor,
	float sphere_radius,
	float* intersection_distance
);
*/
GLFWwindow* window;
#define window_width 1024
#define window_height 980
 
float a = 0.0f;

bool keys[1024];
float lastTime;
float deltaTime;
bool mouse[20] = {false};

//cursor parameter

bool firstPush = true;
float preRotX,preRotY,RotX,RotY;
float rotateSpeed = 0.01;
glm::mat4 projectionMatrix;
glm::mat4 ViewMatrix;
float speed = 4.0f; 

//detection radius
int NumOfCube = 4;
const float sphere_radius = 0.9f;
int selected_cube = -1;
glm::vec3 endPoint;

bool isfound = false;
bool isLinear = false;

bool initWindow()
{
	if (!glfwInit())
	{
		std::cout << "ERROR: Could not initialise GLFW...";
		std::cin;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(window_width, window_height, "Assignmet_2", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		std::cout << "ERROR: Could not create winodw...";
		std::cin;
		return false;
	}

	glfwMakeContextCurrent(window);

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		std::cout << "ERROR: Problem initialising GLEW: " << glewGetErrorString(err);
		std::cin;
		return false;
	}

	//glViewport(0, 0, window_width, window_height);

	return true;
}

Camera m_camera(window_width, window_height);

glm::mat4 getDragMoelMatrix(){
		glm::mat4 model;
	  	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model = glm::rotate(model, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
		return model;
}

// can be put into camera
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(m_camera.FoV>=1.0f && m_camera.FoV<=45.0f)
	{
	m_camera.FoV -= 0.1f* yoffset;
	}
	if (m_camera.FoV<1.0f)
	{
		m_camera.FoV=1.0f;
	}
	if (m_camera.FoV>45.0f)
	{
		m_camera.FoV = 45.0f;
	}
	std::cout<<"yoffset "<<yoffset<<std::endl;
	std::cout<<"Fov IS "<<m_camera.FoV<<std::endl;
} 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;

	//using up side signal to switch between mode
	if(action == GLFW_PRESS && key == GLFW_KEY_3){
		m_camera.thirdPersonOn = true;
		m_camera.firstPersonOn = false;
		
		// initial camposition in third person mode
	}
		
	if(action == GLFW_PRESS && key == GLFW_KEY_1){
		m_camera.firstPersonOn = true;
		m_camera.thirdPersonOn = false;
		
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_2){
		m_camera.firstPersonOn = false;
		m_camera.thirdPersonOn = false;
		m_camera.freeCameraOn = true;
	}

	if(action == GLFW_PRESS && key == GLFW_KEY_Q){
		//using quaternion mode
		m_camera.quaternionOn = true;
		m_camera.EulerOn = false;
		cout<<"quaternion mode is on"<< endl;
	}
	if(action == GLFW_PRESS && key == GLFW_KEY_E){
		//using euler mode
		m_camera.quaternionOn = false;
		m_camera.EulerOn = true;
		cout<<"euler mode is on"<< endl;
	}

	if(action == GLFW_PRESS && key == GLFW_KEY_F){
		m_camera.fixCamera = ! m_camera.fixCamera;
	}

	if(action == GLFW_PRESS && key == GLFW_KEY_C){
	//	isCCD = ! isCCD;
	}
	
	if(action == GLFW_PRESS && key == GLFW_KEY_X){
		 //isfound = false;
		isLinear = !isLinear;
	}

}
void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(action == GLFW_PRESS)
		mouse[button] = true;
	else if (action == GLFW_RELEASE)
		mouse[button] = false;

	
}
void MouseAction()
{
	if (mouse[GLFW_MOUSE_BUTTON_LEFT])
	{
		double xpos = 0 , ypos= 0 ;
		float Ori_rotateX = 0.0, Ori_rotateY = 0.0;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (firstPush)//the first push
		{
			firstPush = false;
			Ori_rotateX = xpos;
			Ori_rotateY = ypos;
			//std::cout<<"X: "<<xpos<<" , "<<"Y: "<<ypos<<std::endl;
		}
		else//listen course position when pushing
		{
			RotX = (xpos - Ori_rotateX) * rotateSpeed;
			RotY = (ypos - Ori_rotateY) * rotateSpeed;
			//std::cout<<"RotX: "<<RotX<<" , "<<"RotY: "<<RotY<<std::endl;
		}
	}
	else if (!mouse[GLFW_MOUSE_BUTTON_LEFT])
	{
		firstPush = true;
		preRotX += RotX;
		preRotY += RotY;
		RotX = 0;
		RotY = 0;
	}
	//use right button to pick cubes
	/*
	if(mouse[GLFW_MOUSE_BUTTON_RIGHT]){
		double xpos = 0 , ypos= 0 ;
		glfwGetCursorPos(window, &xpos, &ypos);
		//get the ray in world coords
		glm::vec3 ray_world = get_ray_from_mouse(xpos,ypos);
		// check ray against all spheres in scene
		int closest_sphere_clicked = -1;
		float closest_intersection = 0.0f;
		for (int i = 0; i < 4; i++) {
			float t_dist = 0.0f;
			if (ray_sphere (
				Camposition, ray_world, cubes_pos_wor[i], sphere_radius, &t_dist
			)) {
				// if more than one sphere is in path of ray, only use the closest one
				if (-1 == closest_sphere_clicked || t_dist < closest_intersection) {
					closest_sphere_clicked = i;
					closest_intersection = t_dist;
				}
			}
		} // endfor
		selected_cube = closest_sphere_clicked;
		//set the position of the sphere as mouse;
		//sphere_pos_wor[g_selected_sphere] = ray_wor;
		printf ("sphere %i was clicked\n", closest_sphere_clicked);
	}
	*/
}


int main()
{
	
	/* Create GL Window */
	if (!initWindow())
		return -1;
	glEnable(GL_BLEND);// you enable blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);
	glfwSetScrollCallback(window,scroll_callback);
	glfwSetKeyCallback(window,key_callback);
	glfwSetMouseButtonCallback(window,mousebutton_callback);

	/*
	// Initialise vertex buffers for cube 
	CubeMap cubeMap;
	cubeMap.loadCubeMap("../textures/cubemaps/hw_sahara/");
	cubeMap.setShader("../shaders/cubemapShader.vert","../shaders/cubemapShader.frag");

	MyTexture meatTex;
	meatTex.TextureFromFile("superman.bmp","../textures/");
	MyTexture floorTex;
	floorTex.TextureFromFile("WoodFine.jpg","../textures/");
	MyTexture boneTex;
	boneTex.TextureFromFile("bones_texture.jpg","../textures/");
	*/
	// load plane model 
	Model model_mountain("../models/Map/ghv.obj");
	Model model_carpet("../models/3dm-welcome/3dm-welcome.3ds");
	Shader objectShader;
	objectShader.loadShadersFromFiles("../shaders/planeShader.vert","../shaders/planeShader.frag");
	Shader CharactorShader;
	CharactorShader.loadShadersFromFiles("../shaders/handShader.vert","../shaders/handShader.frag");
	/*	
	Shader floorShader;
	floorShader.loadShadersFromFiles("../shaders/simpleShader.vert","../shaders/simpleShader.frag");
	Shader PointShader;
	PointShader.loadShadersFromFiles("../shaders/PointShader.vert","../shaders/PointShader.frag");
	//for lab_3
	//SkeletonSim m_skeleton;
	//m_skeleton.initial(&planeShader); // put the model loading inside the simulation
	*/
	Object carpet( &model_carpet , &m_camera, &objectShader);
	Object mountain(&model_mountain, &m_camera, &objectShader); 
	//using Model headfile
	core::ModelLoader m_model_loader;
	core::Model m_player_model_animated(CharactorShader.shaderProgramID);
	core::Model m_npc_model_animated(CharactorShader.shaderProgramID);

	m_model_loader.loadModel("../models/player/player_idle.dae", &m_player_model_animated);
	m_model_loader.GetAnotherAnimation("../models/player/player_run.dae", &m_player_model_animated);
	m_model_loader.GetAnotherAnimation("../models/player/player_slap.dae", &m_player_model_animated);
	m_model_loader.GetAnotherAnimation("../models/player/player_rid_idle.dae", &m_player_model_animated);
	Player m_player( &m_player_model_animated , &m_camera); 
	
	m_model_loader.loadModel("../models/NPC/Wolf/Wolf_idle.dae", &m_npc_model_animated);
	m_model_loader.GetAnotherAnimation("../models/NPC/Wolf/Wolf_walk_idle.dae", &m_npc_model_animated);
	m_model_loader.GetAnotherAnimation("../models/NPC/Wolf/Wolf_walk.dae", &m_npc_model_animated);
	m_model_loader.GetAnotherAnimation("../models/NPC/Wolf/Wolf_redy.dae", &m_npc_model_animated);
	Npc m_npc( &m_npc_model_animated,  &m_camera);
	m_npc.setPlayer(&m_player);
	
	Level first_level(&m_camera, &m_player, &carpet, &mountain);
	first_level.init();
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	while (!glfwWindowShouldClose(window))
	{

		// Reset viewport
        glViewport(0, 0, window_width, window_height);   
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		double currentTime = glfwGetTime();
		deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		// for lab3
		MouseAction();
		m_camera.Update(window,deltaTime, keys);
		projectionMatrix = m_camera.getProjectionMatrix();
		ViewMatrix =m_camera.getViewMatrix();
		
		m_player.update(deltaTime, keys);
		m_npc.update(deltaTime, keys);
		first_level.Upate(deltaTime, keys);

		/*
		//draw the skymap
		cubeMap.getShader()->enableShader();
		cubeMap.getShader()->setUniformMatrix4fv("viewMat", ViewMatrix);
		cubeMap.getShader()->setUniformMatrix4fv("projectionMat", projectionMatrix);
		cubeMap.use(); // bind the related texture here
		//cubeMap.drawSkyBox();
		*/
		/*
			//plane render
			// for lab_3 IK
			
			planeShader.enableShader();
			glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
			glm::mat4 Worldmodel = getThirdPlaneModelMatrix();
			glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "model"), 1, GL_FALSE, &Worldmodel[0][0]);
			glUniform3fv(glGetUniformLocation(planeShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
			//glUniform1f(glGetUniformLocation(meatShader.shaderProgramID,"dragging"),mouse[GLFW_MOUSE_BUTTON_LEFT]);
			boneTex.use(planeShader.shaderProgramID,"diffuse1");
			//bone[i].Draw(planeShader);
			/*
			m_skeleton.skeletonDraw(&planeShader);
			if (isCCD && !isfound)
			{
				if(m_skeleton.ComputeCCD(endPoint))
					isfound = true;	
			}
			*/
		/*
			m_skeleton.Animation(deltaTime, endPoint, &planeShader);
			
			
			if (isCCD)
			{
				glm::vec3 startPoint = glm::vec3(8.0f,5.0f,-5.0f);
				glm::vec3 controlPoint1 =glm::vec3(10.0f,7.0f,-7.0f);
				glm::vec3 controlPoint2 = glm::vec3(9.0f,-7.0f,7.0f);
 				glm::vec3 DestPoint = glm::vec3(7.0f,-5.0f,5.0f);
				//MovePoint(&endPoint, currentTime);
				endPoint = CubicInterpolate(startPoint, controlPoint1, controlPoint2, DestPoint, Timer);
			}

			

			

			if (keys[GLFW_KEY_N])
			{
				endPoint.x += speed/100;
			//	m_skeleton.hand->parent->parent->rot.z += speed/100; 
			}
			if (keys[GLFW_KEY_M])
			{
				endPoint.x -= speed/100;
			//	m_skeleton.hand->parent->parent->rot.z -= speed/100; 
			}
			if (keys[GLFW_KEY_V])
			{
				endPoint.z -= speed/100;
			//	m_skeleton.hand->parent->rot.y += speed/100; 
			}
			if (keys[GLFW_KEY_B])
			{
				endPoint.z += speed/100;
			//	m_skeleton.hand->parent->rot.y -= speed/100; 
			}
			if (keys[GLFW_KEY_G ])
			{
				endPoint.y -= speed/100;
			//	m_skeleton.hand->parent->rot.y += speed/100; 
			}
			if (keys[GLFW_KEY_H])
			{
				endPoint.y += speed/100;
			//	m_skeleton.hand->parent->rot.y -= speed/100; 
			}
			PointShader.enableShader();
			drawEndPoint(PointShader);

		*/
		/*
		// draw map
		planeShader.enableShader();
		glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
		glm::mat4 Worldmodel = glm::mat4(1);
		Worldmodel = glm::translate(Worldmodel,glm::vec3(0.0f,-5.0f,100.0));
		Worldmodel = glm::scale(Worldmodel, glm::vec3(20));
		glUniformMatrix4fv(glGetUniformLocation(planeShader.shaderProgramID, "model"), 1, GL_FALSE, &Worldmodel[0][0]);
		plane.Draw(planeShader);
		*/
		

		m_player.playerRender();
		m_npc.playerRender();
		first_level.Render();

		/*
		floorShader.enableShader();
		floorTex.use(floorShader.shaderProgramID,"diffuse1");
		RenderScene(floorShader);
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

//========================================================
// for previous labs
void RenderScene(Shader &shader)
{
	 // Floor
    glm::mat4 model;
	model = glm::scale(model,glm::vec3(1.5f,1.0f,1.5f));
	model = glm::translate(model,glm::vec3(0.0f, -5.0f,0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
	Floor floor;
	floor.render();

}
void drawEndPoint(Shader &shader){
	glEnable(GL_PROGRAM_POINT_SIZE);

	GLfloat position[] = {
		endPoint.x, endPoint.y, endPoint.z,
	};
	GLuint vao,vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shader.shaderProgramID, "position");
	glm::mat4 model;
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(position_attribute);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_POINTS, 0, 1); // maybe better visulisation 
	glBindVertexArray(0);
}
void MovePoint(glm::vec3 *point, float delt){
	
	glm::vec3 center = glm::vec3( 10.0f, 0.0f,0.0f);
	float radi = 3.0f;
	float thetaSpeed = speed/40;
	*point = center + 2.0f*radi * glm::vec3 (  cos( delt * thetaSpeed), 0.0f,  sin(delt * thetaSpeed)) + radi * glm::vec3 ( 0.0f, sin(delt * thetaSpeed), 2.0f * cos( delt * thetaSpeed));

}
glm::vec3 get_ray_from_mouse(float mouse_x,float mouse_y){
	//screen space
	float x =(2.0f * mouse_x)/window_width - 1.0f;
	float y = 1.0f - (2.0f * mouse_y)/window_height;
	float z = 1.0f;
	// normalized
	glm::vec3 ray_nds(x,y,z);
	// tramsform into clip coords
	glm::vec4 ray_clip(ray_nds.x,ray_nds.y, -1.0, 1.0);
	// tramsform into eye coods
	glm::vec4 ray_eye = glm::inverse(projectionMatrix)* ray_clip;
	ray_eye = glm::vec4(ray_eye.x,ray_eye.y,-1.0,0.0); // set as forward instead of a point
	// tramsform into world coods
	glm::vec3 ray_world = glm::vec3(glm::inverse(ViewMatrix) * ray_eye);
	ray_world = glm::normalize(ray_world);

	return ray_world;
}
bool ray_sphere (
	glm::vec3 ray_origin_wor,
	glm::vec3 ray_direction_wor,
	glm::vec3 sphere_centre_wor,
	float sphere_radius,
	float* intersection_distance
) {
	// work out components of quadratic
	glm::vec3 dist_to_sphere = ray_origin_wor - sphere_centre_wor;
	float b = dot (ray_direction_wor, dist_to_sphere);
	float c = dot (dist_to_sphere, dist_to_sphere) -
		sphere_radius * sphere_radius;
	float b_squared_minus_c = b * b - c;
	// check for "imaginary" answer. == ray completely misses sphere
	if (b_squared_minus_c < 0.0f) {
		return false;
	}
	// check for ray hitting twice (in and out of the sphere)
	if (b_squared_minus_c > 0.0f) {
		// get the 2 intersection distances along ray
		float t_a = -b + sqrt (b_squared_minus_c);
		float t_b = -b - sqrt (b_squared_minus_c);
		*intersection_distance = t_b;
		// if behind viewer, throw one or both away
		if (t_a < 0.0) {
			if (t_b < 0.0) {
				return false;
			}
		} else if (t_b < 0.0) {
			*intersection_distance = t_a;
		}
		
		return true;
	}
	// check for ray hitting once (skimming the surface)
	if (0.0f == b_squared_minus_c) {
		// if behind viewer, throw away
		float t = -b + sqrt (b_squared_minus_c);
		if (t < 0.0f) {
			return false;
		}
		*intersection_distance = t;
		return true;
	}
	// note: could also check if ray origin is inside sphere radius
	return false;
}
// for changing the position of picked objects
glm::vec3 get_mouse_move(float speed, glm::vec3 currentPosition){
	double xpos, ypos;
	glfwGetCursorPos (window, &xpos, &ypos);
	float x = (2.0f * xpos) / window_width - 1.0f;
	float y = 1.0f - (2.0f * ypos) / window_height;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3 (x, y, z);

	//ray_nds = get_ray_from_mouse (xpos,ypos);
	
	//keep the height value of the cube, move in xz plane
	ray_nds = glm::vec3(currentPosition.x + ray_nds.x*speed,currentPosition.y,currentPosition.z - ray_nds.y*speed);

	return ray_nds;
}
