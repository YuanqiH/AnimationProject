#include <iostream>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

//GL include
#include "ShaderManager.hpp"
#include "CubeMap.hpp"
#include "Model_loader.h"
#include "loadTexture.h"
#include "cube.h"
#include "Floor.h"
#include "ShadowMap.h"


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

GLFWwindow* window;
#define window_width 1024
#define window_height 980
 
float a = 0.0f;
// position
glm::vec3 Camposition = glm::vec3( 0.0f, 5.0f, 5.0f );
glm::vec3 direction = glm::vec3( 0.0f, 0.0f, -1.0f );
glm::vec3 up = glm::vec3( 0.0f, 1.0f, 0.0f );
glm::vec3 rightv = glm::vec3( 0.0f, 0.0f, 0.0f );
glm::mat4 projectionMatrix;
glm::mat4 ViewMatrix;

// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float FoV = 45.0F;

float speed = 4.0f; // 4 units / second
float mouseSpeed = 0.005f;
bool keys[1024];
float lastTime;
float deltaTime;
//cursor parameter
bool mouse[20] = {false};
bool firstPush = true;
float preRotX,preRotY,RotX,RotY;
float rotateSpeed = 0.01;

//parameter for shaders
bool bump = false;
float swich =1.0f;
bool fixCamera;

//position parameter for cubes 
glm::vec3 cubes_pos_wor[] = {
	glm::vec3 (-5.0f, 1.5f, 2.0),
	glm::vec3 (6.0f, 0.0f, 4.0),
	glm::vec3 (-1.0f, 2.0f, 3.0),
	glm::vec3 (1.5, 1.0, -1.0)
};
//create lamp object
glm::vec3 lightPosition;
bool lampIsPicked;
glm::vec3 NewPosition;

//detection radius
int NumOfCube = 4;
const float sphere_radius = 0.9f;
int selected_cube = -1;

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
//Camera caculation
void computeMatricesFromInputs()
	{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	// Compute new orientation
	horizontalAngle += mouseSpeed * deltaTime * float(window_width/2 - xpos );
	verticalAngle   += mouseSpeed * deltaTime * float( window_height/2 - ypos );
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3 (
    cos(verticalAngle) * sin(horizontalAngle),
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
	);
	// Right vector

	rightv = glm::normalize(glm::vec3(glm::cross(direction,glm::vec3(0.0,1.0,0.0))));
	// Up vector : perpendicular to both direction and right
	up = glm::cross( rightv, direction );

	}
glm::mat4 getViewMatrix()
{
	// Camera matrix
	glm::mat4 ViewMatrix = glm::lookAt(
    Camposition,           // Camera is here
    Camposition+direction, // and looks here : at the same position, plus "direction"
    up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{ 
	 glm::mat4 projectionMatrix = glm::perspective(FoV, float(window_width)/float(window_height), 1.0f, 1000.0f);
	 return projectionMatrix;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(FoV>=1.0f && FoV<=45.0f)
	{
	FoV -= 0.1f* yoffset;
	}
	if (FoV<1.0f)
	{
		FoV=1.0f;
	}
	if (FoV>45.0f)
	{
		FoV = 45.0f;
	}
	std::cout<<"yoffset "<<yoffset<<std::endl;
	std::cout<<"Fov IS "<<FoV<<std::endl;
} 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;

}
void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(action == GLFW_PRESS)
		mouse[button] = true;
	else if (action == GLFW_RELEASE)
		mouse[button] = false;

	if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT){
		double xpos = 0 , ypos= 0 ;
		glfwGetCursorPos(window, &xpos, &ypos);
		//get the ray in world coords
		glm::vec3 ray_world = get_ray_from_mouse(xpos,ypos);
		// check ray against all cubes in scene
		int closest_sphere_clicked = -1;
		float closest_intersection = 0.0f;
		for (int i = 0; i < NumOfCube; i++) {
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

		//picking lamp
		float lamp_intersection = 0.0f;
		if(ray_sphere(Camposition,ray_world,lightPosition,sphere_radius,&lamp_intersection)){
			lampIsPicked = true;
			printf("the lamp has been pickup, use E to leave it on new position");
			if(lamp_intersection < closest_intersection)//if the lamp is picked, dont care about cube
				selected_cube = -1;
		}
		else
			lampIsPicked =false;
	}
}
// Input action
void KeyAction()
{
		if( keys[GLFW_KEY_W])
	{
		 Camposition += direction * deltaTime * speed;
	}
	if( keys[GLFW_KEY_S])
	{
		 Camposition -= direction * deltaTime * speed;
	}
	if( keys[ GLFW_KEY_D])
	{
		Camposition += rightv * deltaTime * speed;
	}
	if(   keys[ GLFW_KEY_A] )
	{
		 Camposition -= rightv * deltaTime * speed;
	}

	if( keys[ GLFW_KEY_LEFT_SHIFT] ){
			speed = 12.0f;
	}
	else{
			speed = 4.0f;
	}

	//key behavior for bump-map
	if(keys[GLFW_KEY_N])
	{
		bump = 1.0 ;
	}
	
	if (keys[GLFW_KEY_M])
	{
		bump = 0.0;
	}
	
	//key behavior for reflection
	if (keys[GLFW_KEY_1])//reflection
	{
		swich = 1.0;
	}
	if (keys[GLFW_KEY_2])//refraction
	{
		swich = 2.0;
	}
	if (keys[GLFW_KEY_3])//combine
	{
		swich = 3.0;
	}

	// key for fix camera
	if(keys[GLFW_KEY_F]){
		fixCamera = true;
	}
	else{
		fixCamera = false;
	}			

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

	// Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

	glfwSetScrollCallback(window,scroll_callback);
	glfwSetKeyCallback(window,key_callback);
	glfwSetMouseButtonCallback(window,mousebutton_callback);

	//initial shadowmap
	ShadowMap shadow;

	//set at the initial lamp position
	Cube lamp;
	lamp.setPosition(glm::vec3(4.0f, 8.0f, -4.0f));
	lightPosition = lamp.position;

	
	// Initialise vertex buffers for cube 
	CubeMap cubeMap;
	cubeMap.loadCubeMap("../textures/cubemaps/SwedishRoyalCastle/");
	cubeMap.setShader(ShaderManager::loadShader("cubemapShader"));

	/*
	// Load bump map model 
	Model meat("../models/pistachio_obj/pistachio.obj");// this model's normal is wrong! but can show the normal map
	//Model meat("../models/Earth/Earth.obj");
	Shader meatShader;
	meatShader.loadShadersFromFiles("../shaders/Bump_model.vert","../shaders/Bump_model.frag");
	*/
	Model orange("../models/buffman_obj/buffman2.dae");
	Shader orangeShader;
	orangeShader.loadShadersFromFiles("../shaders/Bump_model.vert","../shaders/Bump_model.frag");
	MyTexture orangeText[2];// load two texture manually, 0 for diffuse texture, 1 for normal map
	orangeText[0].TextureFromFile("buffman2Texture.png","../models/buffman_obj");
	orangeText[1].TextureFromFile("buffman2NormalMap.png","../models/buffman_obj");



	MyTexture meatTex;
	meatTex.TextureFromFile("WoodFine.jpg","../textures/");
	
	// load reflect model 
	Model rock("../models/Rock1.dae");
	Shader rockShader;
	rockShader.loadShadersFromFiles("../shaders/simpleReflect.vert","../shaders/simpleReflect.frag");

	//Shader for create depth map
	Shader simpleDepthShader;
	simpleDepthShader.loadShadersFromFiles("../shaders/simpleDepthShader.vert","../shaders/simpleDepthShader.frag");
	Shader debugDepthQuad;
	debugDepthQuad.loadShadersFromFiles("../shaders/debug_quad.vert","../shaders/debug_quad.frag");
	//shader for rendering cubes
	Shader renderNormalShader;
	renderNormalShader.loadShadersFromFiles("../shaders/simple.vert","../shaders/simple.frag");
	//shader for lamp
	Shader lampShader;
	lampShader.loadShadersFromFiles("../shaders/lampShader.vert","../shaders/lampShader.frag");
	
	//Model Sphere("../models/Moon_3D_Model/moon.obj");
	Shader SphereShader;
	SphereShader.loadShadersFromFiles("../shaders/sphereReflect.vert","../shaders/sphereReflect.frag");

	// Wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		//Compute the MV matrix from the keyboard and mouse
		KeyAction();
		MouseAction();
		if (!mouse[GLFW_MOUSE_BUTTON_LEFT] && fixCamera == false)//when dragging, when picking, fix the camera
		{
			computeMatricesFromInputs();
		}

		projectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();



		//create shadow map matrix
		if(keys[GLFW_KEY_UP]){
			lamp.position.y += 0.05f;
		}
		if (keys[GLFW_KEY_DOWN])
		{
			lamp.position.y -= 0.05f;
		}
		glm::vec3 lampDirct = glm::vec3(0.0f,0.0f,0.0f) - lightPosition;
		glm::vec3 lampRight = glm::cross(lampDirct,up);
		glm::vec3 lampUp = glm::cross(lampDirct,lampRight);
		GLfloat near_plane =1.0f, far_plane = 20.0f;
		//glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f,-10.0f,10.0f,near_plane,far_plane);
		glm::mat4 lightProjection = glm::perspective(FoV, float(window_width)/float(window_height),near_plane,far_plane);
		glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lampDirct, lampUp); 
		glm::mat4 lightSpaceMatrix = lightProjection * lightView; 

		//render shadow scene
		simpleDepthShader.enableShader();
		glm::mat4 model_shadow;
        model_shadow = glm::translate(model_shadow, glm::vec3(0.0f,2.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model_shadow = glm::rotate(model_shadow, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model_shadow = glm::rotate(model_shadow, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.shaderProgramID, "model"), 1, GL_FALSE, &model_shadow[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.shaderProgramID, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		glViewport(0, 0, shadow.SHADOW_WIDTH, shadow.SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.getDepthMapFBO());
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		rock.Draw(simpleDepthShader);
		//TRY THE EXAMPLE ONE
		RenderScene(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
		glCullFace(GL_BACK);

		// Reset viewport
        glViewport(0, 0, window_width, window_height);        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		
		//draw the skymap
		cubeMap.getShader()->enableShader();
		cubeMap.getShader()->setUniformMatrix4fv("viewMat", ViewMatrix);
		cubeMap.getShader()->setUniformMatrix4fv("projectionMat", projectionMatrix);
		cubeMap.use(); // bind the related texture here
		cubeMap.drawSkyBox();
		

		//render normally to check whether all objects are inside scene
		renderNormalShader.enableShader();
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model = glm::rotate(model, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
        glUniformMatrix4fv(glGetUniformLocation(renderNormalShader.shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
		glUniform3fv(glGetUniformLocation(renderNormalShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
		glUniform3fv(glGetUniformLocation(renderNormalShader.shaderProgramID,"lightPos"),1,&lightPosition[0]);
		glUniformMatrix4fv(glGetUniformLocation(renderNormalShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(renderNormalShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(renderNormalShader.shaderProgramID, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		meatTex.use(renderNormalShader.shaderProgramID,"diffuseTexture");
		// when using different texture unit, has to rebind the Texture;
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadow.getDepthMap());
		glUniform1i(glGetUniformLocation(renderNormalShader.shaderProgramID, "depthMap"), 1);
		rock.Draw(renderNormalShader);
		RenderScene(renderNormalShader);

		//render lamp, do this after other rendering
		lampShader.enableShader();
		glm::mat4 model_lamp;
		if(lampIsPicked==true){
		//if lamp is picked move the lamp based on mouse position, so as move light position
		model_lamp = glm::translate(model_lamp,get_mouse_move(4.0,lamp.position));
		//update light position when moving 
		lightPosition = get_mouse_move(4.0,lamp.position);
		NewPosition =  get_mouse_move(4.0,lamp.position);
			if(keys[GLFW_KEY_E]){//if press e, set the new lamp position
				lamp.setPosition(NewPosition);
				lampIsPicked = false;
				}
		}
		else{
		model_lamp = glm::translate(model_lamp, lamp.position); // Translate it to lightPosition
		//reset light positon as lamp positoin
		lightPosition = lamp.position;
		}// end if
		model_lamp = glm::scale(model_lamp, glm::vec3(0.5f, 0.5f, 0.5f));	// make lamp smaller
		glUniformMatrix4fv(glGetUniformLocation(lampShader.shaderProgramID, "model"), 1, GL_FALSE, &model_lamp[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lampShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lampShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
		lamp.render();
		
        // Render Depth map to quad
        debugDepthQuad.enableShader();
        glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgramID, "near_plane"), near_plane);
        glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgramID, "far_plane"), far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow.getDepthMap());
        RenderQuad();
        
		//render orange with bump map
		orangeShader.enableShader();
		glUniformMatrix4fv(glGetUniformLocation(orangeShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(orangeShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
        glm::mat4 model_bump;
		model_bump = glm::rotate(model_bump, 90.0f ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model_bump = glm::translate(model_bump, glm::vec3(-6.0f, 4.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model_bump = glm::scale(model_bump, glm::vec3(0.03f, 0.03f, 0.03f));	// It's a bit too big for our scene, so scale it down
		model_bump = glm::rotate(model_bump, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model_bump = glm::rotate(model_bump, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
        glUniformMatrix4fv(glGetUniformLocation(orangeShader.shaderProgramID, "modelMat"), 1, GL_FALSE, &model_bump[0][0]);
		glUniform3fv(glGetUniformLocation(orangeShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
		glUniform3fv(glGetUniformLocation(orangeShader.shaderProgramID,"lightPos"),1,&lightPosition[0]);
		glUniform1f(glGetUniformLocation(orangeShader.shaderProgramID,"normalMapping"),bump);
		orangeText[0].use(orangeShader.shaderProgramID,"texture_diffuse1",0);
		orangeText[1].use(orangeShader.shaderProgramID,"texture_normal1",1);
		//orange.Draw(orangeShader);
		
		/*


		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, window_width, window_height);
		// Rendering Code 
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		
		//model using texture
		
		meatShader.enableShader();
		glUniformMatrix4fv(glGetUniformLocation(meatShader.shaderProgramID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(meatShader.shaderProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
        glm::mat4 model;
		model = glm::translate(model, glm::vec3(-6.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model = glm::rotate(model, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
        glUniformMatrix4fv(glGetUniformLocation(meatShader.shaderProgramID, "modelMat"), 1, GL_FALSE, &model[0][0]);
		glUniform3fv(glGetUniformLocation(meatShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
		glUniform1f(glGetUniformLocation(meatShader.shaderProgramID,"normalMapping"),bump);
		//glUniform1f(glGetUniformLocation(meatShader.shaderProgramID,"dragging"),mouse[GLFW_MOUSE_BUTTON_LEFT]);
        //meatTex.use(meatShader.shaderProgramID,"diffuse1");
		meat.Draw(meatShader);
		
		
		//model doing reflection
		rockShader.enableShader();
		glUniformMatrix4fv(glGetUniformLocation(rockShader.shaderProgramID, "projectionMat"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(rockShader.shaderProgramID, "viewMat"), 1, GL_FALSE, &ViewMatrix[0][0]);
        glm::mat4 model1;
        model1 = glm::translate(model1, glm::vec3(0.0f,0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model1 = glm::scale(model1, glm::vec3(1.02f, 1.02f, 1.02f));	// It's a bit too big for our scene, so scale it down
		model1 = glm::rotate(model1, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model1 = glm::rotate(model1, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
        glUniformMatrix4fv(glGetUniformLocation(rockShader.shaderProgramID, "modelMat"), 1, GL_FALSE, &model1[0][0]);
		glUniform3fv(glGetUniformLocation(rockShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
		glUniform1f(glGetUniformLocation(rockShader.shaderProgramID,"swichFlag"),swich);
        rock.Draw(rockShader);
		
		SphereShader.enableShader();
		glUniformMatrix4fv(glGetUniformLocation(SphereShader.shaderProgramID, "projectionMat"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(SphereShader.shaderProgramID, "viewMat"), 1, GL_FALSE, &ViewMatrix[0][0]);
        glm::mat4 model2;
        model2 = glm::translate(model2, glm::vec3(7.0f,0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model2 = glm::scale(model2, glm::vec3(0.02f, 0.02f, 0.02f));	// It's a bit too big for our scene, so scale it down
		model2 = glm::rotate(model2, (preRotX + RotX) ,glm::vec3(0.0,1.0,0.0));//rotate with y axis
		model2 = glm::rotate(model2, (preRotY + RotY) ,glm::vec3(1.0,0.0,0.0));//rotate with x axis
        glUniformMatrix4fv(glGetUniformLocation(SphereShader.shaderProgramID, "modelMat"), 1, GL_FALSE, &model2[0][0]);
		glUniform3fv(glGetUniformLocation(SphereShader.shaderProgramID,"viewPos"),1,&Camposition[0]);
		glUniform1f(glGetUniformLocation(SphereShader.shaderProgramID,"swichFlag"),swich);
        Sphere.Draw(SphereShader);


	    glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
void RenderScene(Shader &shader)
{
	 // Floor
    glm::mat4 model;
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
	Floor floor;
	floor.render();
    // Cubes
	Cube cube[4];
	for(int i = 0; i<NumOfCube;i++){
    model = glm::mat4();
	if(selected_cube == i){
	model = glm::translate(model, get_mouse_move(4.0,cubes_pos_wor[i]));
	NewPosition =  get_mouse_move(4.0,cubes_pos_wor[i]);
			if(keys[GLFW_KEY_E]){//if press e, set the new lamp position
				cubes_pos_wor[i] = NewPosition;
				selected_cube = -1;
				}
	}
	else{
    model = glm::translate(model, cubes_pos_wor[i]);
	}
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
	cube[i].render();
	}
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
