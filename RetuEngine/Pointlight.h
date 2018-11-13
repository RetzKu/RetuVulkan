#pragma once
#include <glm\glm.hpp>

class Pointlight
{
public:
	Pointlight(glm::vec3 pos, float radius, glm::vec3 intensity,glm::vec3 color) : pos(pos), radius(radius), intensity(intensity), color(color) {};

	glm::vec3 pos;
	float radius;
	glm::vec3 intensity;
	glm::vec3 color;
};
