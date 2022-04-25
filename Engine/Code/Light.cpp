#include "Light.h"

Light::Light(LIGHT_TYPE type, glm::vec3 color, glm::vec3 direction, glm::vec3 position) :
	type(type),
	color(color),
	direction(direction),
	position(position)
{
}