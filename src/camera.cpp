#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "inputprocesser.h"
#include <iostream>

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(regular.position, regular.target, worldUp);
}

Camera::Camera(glm::vec3 position, CameraOptions options) : options(options)
{}

void Camera::update(float dt)
{
    auto offset = InputProcesser::deltaMouse * InputProcesser::SENSITIVITY;
    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
    {
        regular.theta += offset.x;
        regular.phi += offset.y;
    }
    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
        forward = glm::normalize(regular.target - regular.position);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
        regular.target -= right * offset.x * 0.05f;
        regular.target -= up * offset.y * 0.05f;
    }

    float x = glm::sin(glm::radians(regular.phi)) * glm::sin(glm::radians(regular.theta));
    float z = glm::sin(glm::radians(regular.phi)) * glm::cos(glm::radians(regular.theta));
    float y = glm::cos(glm::radians(regular.phi));

    regular.position = glm::vec3(x, y, z) * regular.targetDistance + regular.target;
}
