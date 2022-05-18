#pragma once
#include "platform.h"

#define DIRECTIONAL_LIGHT_DEBUG_DRAW_DISTANCE 8.f

enum class LIGHT_TYPE
{
	DIRECTIONAL = 0,
	POINT,
	MAX
};


struct Light
{
	Light(LIGHT_TYPE type, glm::vec3 color, glm::vec3 direction, glm::vec3 position);

	glm::mat4 CalculateWorldTransform();

	LIGHT_TYPE type;
	float maxDistance = 10.f;
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 position;

	u32 localParamsOffset;
	u32 localParamsSize;
};