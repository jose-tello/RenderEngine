#include "Camera.h"

Camera::Camera() :
	position(0.f, 4.f, 10.f),
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


void Camera::HandleInput(Input* input)
{
	if (input->keys[K_W] == BUTTON_PRESS || input->keys[K_S] == BUTTON_PRESSED)
	{
		currentDir = glm::normalize(target - position);
	}


	if (input->keys[K_W] == BUTTON_PRESSED)
	{
		position += currentDir * 0.5f;
	}

	else if (input->keys[K_S] == BUTTON_PRESSED)
	{
		position -= currentDir * 0.5f;
	}

	if (input->keys[K_A] == BUTTON_PRESSED)
	{
		glm::vec3 left = glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(target - position));
		left = glm::normalize(left);
		position += left * 0.5f;
	}

	if (input->keys[K_D] == BUTTON_PRESSED)
	{
		glm::vec3 left = glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(target - position));
		left = glm::normalize(left);
		position -= left * 0.5f;
	}

	if (input->keys[K_Q] == BUTTON_PRESSED)
	{
		glm::vec3 up(0.f, 1.f, 0.f);
		position += up * 0.5f;
	}

	if (input->keys[K_E] == BUTTON_PRESSED)
	{
		glm::vec3 up(0.f, 1.f, 0.f);
		position -= up * 0.5f;
	}

	
}


float* Camera::GetPosition()
{
	return &position.x;
}


glm::vec3 Camera::GetPositionV3() const
{
	return position;
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