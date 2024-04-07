#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "inputprocesser.h"
#include <iostream>

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(smooth.position, smooth.target, worldUp);
}

Camera::Camera(CameraOptions options) : options(options)
{}

void Camera::update()
{
    forward = glm::normalize(regular.target - regular.position);
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));

    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
    {
        auto offset = InputProcesser::deltaMouse * InputProcesser::SENSITIVITY_ROTATE;
        regular.theta += offset.x;
        regular.phi += offset.y;
        regular.phi = glm::clamp(regular.phi, 0.f, 180.0f);
    }
    if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
        auto offset = InputProcesser::deltaMouse * InputProcesser::SENSITIVITY_TRANSLATE;
        regular.target -= right * offset.x ;
        regular.target -= up * offset.y;
    }
    recalculateFields();
}

void Camera::zoom(float yOffset)
{
    regular.targetDistance -= yOffset * 0.2f;
    regular.targetDistance = glm::clamp(regular.targetDistance, 1.0f, 100.0f);
    if (regular.targetDistance <= 1.0f)
    {
        // move the camera forward
        regular.target += forward * 0.1f;
    }
    recalculateFields();
}

void Camera::recalculateFields()
{
    regular.position = sphericalToCartesian(regular.theta, regular.phi, regular.targetDistance) + regular.target;
}

void Camera::lerp(float delta)
{
    float a = 10.f * delta;
    smooth.target = glm::mix(smooth.target, regular.target, a);
    smooth.phi = glm::mix(smooth.phi, regular.phi, a);
    smooth.theta = glm::mix(smooth.theta, regular.theta, a);
    smooth.targetDistance = glm::mix(smooth.targetDistance, regular.targetDistance, a);

    smooth.position = sphericalToCartesian(smooth.theta, smooth.phi, smooth.targetDistance) + smooth.target;
}

glm::vec3 Camera::getPosition() const
{
    return smooth.position;
}

void Camera::setTarget(glm::vec3 position)
{
    regular.target = position;
}

void Camera::setThetaPhi(float theta, float phi)
{
    regular.theta = theta;
    regular.phi = phi;
}

void Camera::setTargetDistance(float distance)
{
    regular.targetDistance = distance;
}

void Camera::simpleReset()
{
    regular.target = {0.f, 0.f, 0.f};
    regular.theta = 0.f;
    regular.phi = 90.f;
    regular.targetDistance = 2.f;
    recalculateFields();
}

glm::vec4 Camera::getForwardVector() const
{
    return {smooth.target - smooth.position, 1.0f};
}

glm::vec3 sphericalToCartesian(float theta, float phi, float radius)
{
    float x = radius * glm::sin(glm::radians(phi)) * glm::sin(glm::radians(theta));
    float z = radius * glm::sin(glm::radians(phi)) * glm::cos(glm::radians(theta));
    float y = radius * glm::cos(glm::radians(phi));
    return glm::vec3(x, y, z) * radius;
}