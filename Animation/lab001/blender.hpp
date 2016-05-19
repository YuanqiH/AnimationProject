#pragma once

#include "AnimationEx.h"
#include <queue>

enum TransitionType { Smooth = 0, Immediate};
struct  Command 
{
	AnimationExtension* animation;
	float blendDuration;
	TransitionType transisiontype;
	bool loop;

	Command(){}
	Command (AnimationExtension* animation, bool loop, float blendDuration, TransitionType transitionType)
	{
		this->animation = animation;
		this->loop = loop;
		this->blendDuration = blendDuration;
		this->transisiontype = transitionType;
	}
};


struct Blender
{
	static float blendScalar;
	static bool frozen;

	bool isBlending;
	bool isIdle;
	
	float blendTimer;
	float blendDuration;

	AnimationExtension* current;
	AnimationExtension* next;

	TransitionType transitionType;

	std::queue<Command> commandQ; //FIFO

	void initBlender()
	{
		isBlending = false;
		blendTimer = 0.0;
		blendDuration = 0.0;

		current = 0;
		next = 0;

		transitionType = TransitionType::Smooth;
	}

	void ResetBlender()
	{
		isBlending = false;
		blendTimer = 0.0;
		blendDuration = 0.0;
	}

	void Update(double deltaTime)
	{
		isIdle = false;

		if(isBlending) //If blending do that
		{
			blendTimer += (deltaTime/1000) * blendScalar;
			float t = blendTimer / blendDuration;

			next->weight = t*t * (3.0f - 2.0f*t);//lerp(0.0f, 1.0f, t);
			current->weight = 1 - next->weight;

			if(t >= 1)
			{
				next->weight = 1; //clamp
				current->Stop();
				current = next;

				ResetBlender();
			}
		}
		else 
		{   //otherwise pop an animation off the command queue, if there is something in it
			if(commandQ.size() > 0)
			{
				Command command = commandQ.front();
				commandQ.pop();

				if(command.animation != current)
				{
					transitionType = command.transisiontype;

					if(transitionType == TransitionType::Immediate)
					{
						//if(current != 0)
							//current->Stop();
						//if(next != 0)
							//next->Stop();
						//commandQueue.empty();

						current = command.animation;
						current->Start(1, command.loop);
					}
					else
					{
						if(current != 0)
						{
							//if(transitionType == TransitionType::Frozen)
							if(frozen)
								current->frozen = true; 
		
							next = command.animation;
							next->Start(0, command.loop);

							blendDuration = command.blendDuration;
							isBlending = true;
						}
					}
				}
			}
			else //If there's nothing in the queue
			{
				if(current != 0)
					if(current->weight == 0) //And the current animation is stopped
						isIdle = true; //I'm Idle!
			}
		}
	}
};