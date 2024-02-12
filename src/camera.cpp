#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "inputprocesser.h"

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, target, worldUp);
}

Camera::Camera(glm::vec3 position, CameraOptions options) : position(position), options(options), phi(90.f), targetDistance(5.f)
{}

void Camera::update(float dt)
{
    auto offset = InputProcesser::deltaMouse * InputProcesser::SENSITIVITY;
    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
    {
        theta += offset.x;
        phi += offset.y;
    }
    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
        forward = glm::normalize(target - position);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
        target -= right * offset.x * 0.05f;
        target -= up * offset.y * 0.05f;
    }

    float x = glm::sin(glm::radians(phi)) * glm::sin(glm::radians(theta));
    float z = glm::sin(glm::radians(phi)) * glm::cos(glm::radians(theta));
    float y = glm::cos(glm::radians(phi));

    position = glm::vec3(x, y, z) * targetDistance + target;
}
