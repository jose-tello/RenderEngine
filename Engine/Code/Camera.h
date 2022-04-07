#pragma once

#include "platform.h"

#define DEFAULT_FOV 60.f
#define DEFAULT_Z_NEAR 0.1f
#define DEFAULT_Z_FAR 1000.f

class Camera
{
public:
	Camera();
	~Camera();

	float* GetPosition();
	void SetPosition(glm::vec3& pos);

	float* GetRotation();
	void SetRotation(glm::vec3& rot);

	float* GetScale();
	void SetScale(glm::vec3& scl);

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
	glm::vec3 scale;

	float aspectRatio;
	float FOV;
	float zNear;
	float zFar;

	glm::vec3 target;
};

