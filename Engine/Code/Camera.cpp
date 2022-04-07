#include "Camera.h"

Camera::Camera() :
	position(0.f, 0.f, 0.f),
	rotation(0.f, 0.f, 0.f),
	scale(1.f, 1.f, 1.f),
	aspectRatio(0.f),
	FOV(DEFAULT_FOV),
	zNear(DEFAULT_Z_NEAR),
	zFar(DEFAULT_Z_FAR),
	target(0.0f, 0.4f, 0.0f)
{
}


Camera::~Camera()
{
}


float* Camera::GetPosition()
{
	return &position.x;
}


void Camera::SetPosition(glm::vec3& pos)
{
	position = pos;
}


float* Camera::GetRotation()
{
	return &rotation.x;
}


void Camera::SetRotation(glm::vec3& rot)
{
	rotation = rot;
}


float* Camera::GetScale()
{
	return &scale.x;
}


void Camera::SetScale(glm::vec3& scl)
{
	scale = scl;
}


void Camera::SetAspectRatio(float ratio)
{
	aspectRatio = ratio;
}

float* Camera::GetFOV()
{
	return &FOV;
}


float* Camera::GetZNear()
{
	return &zNear;
}


float* Camera::GetZFar()
{
	return &zFar;
}


float* Camera::GetTarget()
{
	return &target.x;
}


glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(FOV), aspectRatio, zNear, zFar);
}


glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, target, glm::vec3(0.f, 1.f, 0.f));
}