#pragma once

#include "GLFW/glfw3.h"
#include "glm/vec3.hpp"
#include "glm/glm.hpp"

struct CameraOptions
{
    float fov = 45.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    float speed{};
    float zoom{};
};

struct CameraFields
{
    glm::vec3 position;
    glm::vec3 target;
    float theta;
    float phi = 90.f;
    float targetDistance = 2.f;
};

class Camera
{
private:
    const glm::vec3 worldUp = {0.f, 1.f, 0.f};
    glm::vec3 forward, right, up;
    CameraFields regular, smooth;
    void recalculateFields();

public:
    CameraOptions options;
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), CameraOptions options = CameraOptions());
    void update();
    void zoom(float yOffset);
    void lerp(float delta);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
};

glm::vec3 sphericalToCartesian(float theta, float phi, float radius);
