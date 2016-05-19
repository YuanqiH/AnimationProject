#pragma once

#include "Model.hpp"

struct  AnimationExtension
{
	int animationID;
	std::string name;
	core::Model::Animation animationDate;
	
	double localClock;
	double duration;

	float weight;
	bool frozen;

	bool loop;

	void setAnimation(std::string name,int id, double duration){
		this->name = name;
		this->animationID = id;
		this->duration = duration;

		this->localClock = 0.0;
		this->weight = 0.0; // weight for blending for each animation
		this->frozen = true; // froze the local clock
	}

	void Start(float weight, bool loop){
		this->localClock = 0.0;
		this->weight = weight;
		this->loop = loop;
		this->frozen = false;
	}

	void Stop(){
		this->localClock = 0.0;
		this->weight = 0.0;
		this->frozen = true;
	}



};