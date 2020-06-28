#include "Camera.h"

Camera::Camera() 
	: upVector(0.0f, 1.0f, 0.0f), position(0.0f, 0.0f, 0.0f), fov(radians(90.0f)), yaw(0.0f), pitch(0.0f), roll(0.0f), near(0.01f), far(100.0f), ratio(1.0f) {

}

mat4 Camera::getView() {
	vec3 viewDirection(0.0f, 0.0f, -1.0f);
	viewDirection = vec3(rotate(mat4(1.0f), (float)pitch, vec3(1.0f, 0.0f, 0.0f)) * vec4(viewDirection, 0.0f));
	viewDirection = vec3(rotate(mat4(1.0f), (float)yaw,   vec3(0.0f, 1.0f, 0.0f)) * vec4(viewDirection, 0.0f));
	viewDirection = vec3(rotate(mat4(1.0f), (float)roll,  vec3(0.0f, 0.0f, 1.0f)) * vec4(viewDirection, 0.0f));
	return lookAt(position, position + viewDirection, upVector);
}

mat4 Camera::getProjection() {
	return perspective(fov, ratio, near, far);
}

void Camera::steer(double rot) {
	yaw += rot;
	if (fabs(yaw) > radians(360.0f))
		yaw = fmod(yaw, radians(360.0f));
}

void Camera::tilt(double rot) {
	pitch += rot;
	if (fabs(pitch) > radians(89.0f))
		pitch -= rot;
}

void Camera::move(vec3 movement) {
	position += vec3(rotate(mat4(1.0f), (float)yaw, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), (float)pitch, vec3(1.0f, 0.0f, 0.0f)) * vec4(movement, 0.0f));
}

void Camera::setPosition(vec3 position) {
	this->position = position;
}

void Camera::setFov(double fov) {
	this->fov = fov;
}

void Camera::setNearFar(double near, double far) {
	this->near = near;
	this->far = far;
}

void Camera::setRatio(double width, double height) {
	this->ratio = width / height;
}

void Camera::setRatio(double ratio) {
	this->ratio = ratio;
}

vec3 Camera::getPosition() {
	return position;
}