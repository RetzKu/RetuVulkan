#pragma once
#include <glm\glm.hpp>

class Pointlight
{
public:
	//Pointlight(glm::vec3 pos, float radius, glm::vec3 intensity,glm::vec3 color) 
	Pointlight(glm::vec3 pos, float radius, glm::vec3 intensity) 
	{
		//this->pos = glm::vec4(pos, 1.0);
		this->pos = pos;
		this->radius = radius;
		//this->intensity = glm::vec4(intensity);
		this->intensity = intensity;
		//this->color = glm::vec4(color, 1.0);
	};

	glm::vec3 pos;
	float radius;
	glm::vec3 intensity;
	//glm::vec4 color;
};
