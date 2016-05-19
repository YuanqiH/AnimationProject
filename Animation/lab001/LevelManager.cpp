

#include "LevelManager.hpp"


Level::Level(Camera* camera, Player* player, Object* object, Object* ob2)
{
	this->m_camera = camera;
	this->m_Player = player; 
	this->m_carpet = object;
	this->m_mountain = ob2;
}


void Level::init(){
	evenNum = 0;
	endPoint = glm::vec3(0);
	Timer = 0.0f;
	freezeTimer = 0.0f;
}


void Level::Upate(float deltaTime, bool* keys){
		//=====================================================
		//timer

		double m_startTime = 0.0;
		if (!timerFreeze)
		{
			Timer += 0.05f * deltaTime;
		}
		else
		{
			freezeTimer += 0.2f * deltaTime;
		}
		if (Timer >= 1){Timer = m_startTime; evenNum++; timerFreeze = true;}
		if(freezeTimer >= 1){freezeTimer = m_startTime; timerFreeze = false; }
		
		//======================================================
		//path of the carpet
		glm::vec3 startPoint = glm::vec3(100, 10, 10 ), controlPoint1 = glm::vec3(0, 0, 0), controlPoint2 = glm::vec3(0, 100, 70) , DestPoint = glm::vec3(100,30,30);
		glm::vec3 startPoint2 = glm::vec3(-100, 10, 10 ), DestPoint2 = glm::vec3(-100,30,30);
		evenNum = evenNum%2;
		if (evenNum == 0)
			endPoint = CubicInterpolate(startPoint, controlPoint1, controlPoint2, DestPoint, Timer);
		else
			endPoint = CubicInterpolate(startPoint2, controlPoint2, controlPoint1, DestPoint2, Timer);
		glm::mat4 Worldmodel = glm::mat4(1);
		Worldmodel = glm::translate(Worldmodel, glm::vec3(0.0,-2.0f,0.0)); // adjust
		Worldmodel = glm::translate(Worldmodel,endPoint);
		Worldmodel = glm::rotate(Worldmodel, -1.57f, glm::vec3(0,0,1));
		Worldmodel = glm::rotate(Worldmodel, 3.14f, glm::vec3(0,1,0));
		Worldmodel = glm::scale(Worldmodel, glm::vec3(0.2));
		m_carpet->setModleMatrix(Worldmodel);

		if (glm::distance(endPoint, m_Player->playerTrans.playerPosition) < m_carpet->objectRadius)
		{
			if (m_Player->riddingOn)
			{
				m_carpet->isPicked = true;
			}
		}
		else
		{
			m_carpet->isPicked = false;
		}
		//std::cout<<endPoint.y<<std::endl;
		if ( endPoint.y != 0 && endPoint.y != 100)  // only take the player on ridding TODO: player could change the rid
		{
			if(m_carpet->isPicked && m_Player->riddingOn){
			 m_Player->playerTrans.playerPosition = endPoint;
			}
		}

		//=======================================
		//setting the mountain

		Worldmodel = glm::mat4(1);
		Worldmodel = glm::translate(Worldmodel, glm::vec3(0.0,-2.0f,70.0)); // adjust
		Worldmodel = glm::scale(Worldmodel, glm::vec3(10));
		Worldmodel = glm::rotate(Worldmodel, 0.0f, glm::vec3(0,0,1));
		m_mountain->setModleMatrix(Worldmodel);
		//std::cout<<m_camera->CamPosition.x<<" "<<m_camera->CamPosition.y<<" "<<m_camera->CamPosition.z<<std::endl;

}

void Level::Render(){

	m_carpet->Render();
	m_mountain->Render();

}


glm::vec3 Level::CubicInterpolate(glm::vec3 point0, glm::vec3 point1, glm::vec3 point2,glm::vec3 point3, float time){
	double a0[3],a1[3],a2[3],a3[3],mu2;
	float resultValue[3];
	glm::vec3 resultPoint;
	mu2 = time*time;
	//3D cubic
	for (int i = 0; i < 3; i++)
	{
	a0[i] = -0.5*point0[i] + 1.5*point1[i] - 1.5*point2[i]  + 0.5*point3[i];
	a1[i] = point0[i] - 2.5*point1[i] + 2*point2[i]  - 0.5*point3[i];
	a2[i] = -0.5*point0[i] + 0.5*point2[i] ;
	a3[i] = point1[i];
	resultValue[i] = a0[i]*time*mu2+a1[i]*mu2+a2[i]*time+a3[i];
	}
	resultPoint = glm::vec3(resultValue[0],resultValue[1],resultValue[2]);
	return resultPoint;
}