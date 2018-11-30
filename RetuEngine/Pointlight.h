#pragma once
#include <glm\glm.hpp>

class Pointlight
{
public:
	//Pointlight(glm::vec3 pos, float radius, glm::vec3 intensity,glm::vec3 color) 
	Pointlight(glm::vec3 pos, float radius, float intensity, glm::vec4 color) 
	{
		//this->pos = glm::vec4(pos, 1.0);
		this->pos = glm::vec4(pos,1);
		this->radius = glm::vec4(radius);
		this->intensity = glm::vec4(intensity);
		this->color = color;
	};

	glm::vec4 pos;
	glm::vec4 radius;
	glm::vec4 intensity;
	glm::vec4 color;
};
