#pragma once
#include <glm\glm.hpp>

class Pointlight
{
public:
	Pointlight(glm::vec3 pos, float radius, glm::vec3 intensity) : pos(pos), radius(radius), intensity(intensity) {};

	glm::vec3 pos;
	float radius;
	glm::vec3 intensity;
};
