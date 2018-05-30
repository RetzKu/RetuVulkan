#pragma once
#include <glm\glm.hpp>

class Pointlight
{
public:
	Pointlight(glm::vec3 pos, float intensity, float radius) : pos(pos), radius(radius), intensity(intensity) {};

	glm::vec2 pos;
	float intensity;
	float radius;
};
