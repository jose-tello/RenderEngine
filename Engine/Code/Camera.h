#pragma once

#include "platform.h"

#define DEFAULT_FOV 60.f
#define DEFAULT_Z_NEAR 0.1f
#define DEFAULT_Z_FAR 500.f

class Camera
{
public:
	Camera();
	~Camera();

	void HandleInput(Input* input);


	float* GetPosition();
	glm::vec3 GetPositionV3() const;

	void SetAspectRatio(float ratio);

	float* GetFOV();

	float* GetZNear();
	float* GetZFar();

	float* GetTarget();

	glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewMatrix() const;

private:
	glm::vec3 position;
	glm::vec3 rotation;

	glm::vec3 currentDir;

	float aspectRatio;
	float FOV;
	float zNear;
	float zFar;

	glm::vec3 target;
};

