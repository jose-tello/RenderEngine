#include "Light.h"

Light::Light(LIGHT_TYPE type, glm::vec3 color, glm::vec3 direction, glm::vec3 position) :
	type(type),
	color(color),
	direction(direction),
	position(position)
{
}


glm::mat4 Light::CalculateWorldTransform()
{
	glm::mat4 transform;
	switch (type)
	{
	case LIGHT_TYPE::DIRECTIONAL :
		transform = glm::translate(position);
		return transform;
		break;

	case LIGHT_TYPE::POINT :
		transform = glm::translate(position);
		transform = glm::scale(transform, glm::vec3(0.4, 0.4, 0.4));
		return transform;
		break;

	default:
		ELOG("Need to add light type to switch");
		return glm::mat4();
		break;
	}
}