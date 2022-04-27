#pragma once
#include "platform.h"

enum class LIGHT_TYPE
{
	DIRECTIONAL = 0,
	POINT,
	MAX
};


struct Light
{
	Light(LIGHT_TYPE type, glm::vec3 color, glm::vec3 direction, glm::vec3 position);

	LIGHT_TYPE type;
	float maxDistance = 10.f;
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 position;
};