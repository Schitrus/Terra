#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

using namespace glm;

class Camera
{
	vec3 upVector;

	vec3 position;

	double yaw, pitch, roll;

	double fov; // Field of View

	double near, far;

	double ratio;

public:
	Camera();

	mat4 getView();
	mat4 getView(vec3 position);

	mat4 getProjection();

	void steer(double rot);

	void tilt(double tilt);

	void move(vec3 movement);

	void setFov(double fov);

	void setNearFar(double near, double far);

	void setRatio(double width, double height);
	void setRatio(double ratio);

	void setPosition(vec3 position);

	vec3 getPosition();

private:
};

