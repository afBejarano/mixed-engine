//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#include <Camera.h>

Camera::Camera() {
	projection = glm::mat4();
	view = glm::mat4();
}

void Camera::Perspective(const float fovy, const float aspectRatio, const float near, const float far) {
	projection = glm::perspective<float>(fovy, aspectRatio, near, far);
}
void Camera::LookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up) {
	view = glm::lookAt(eye,at,up);
}
