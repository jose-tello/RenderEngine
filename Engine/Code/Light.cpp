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
	{
		glm::vec3 normDir = glm::normalize(direction);
		glm::vec3 pos = (direction - glm::vec3(0.f, 0.f, 0.f)) * 8.f;

		transform = glm::translate(pos);

		transform *= glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::normalize(pos), glm::vec3(0.f, 1.f, 0.f));

		return transform;
	}
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