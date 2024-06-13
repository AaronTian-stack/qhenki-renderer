#pragma once

#include "GLFW/glfw3.h"
#include "glm/vec3.hpp"
#include "glm/glm.hpp"

struct CameraOptions
{
    float nearClip = 0.1f;
    float farClip = 1000.0f;
};

struct CameraFields
{
    glm::vec3 position;
    glm::vec3 target;
    float theta;
    float phi = 90.f;
    float targetDistance = 2.f;
    float fov = 45.0f;
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
    Camera(CameraOptions options = CameraOptions());
    void update();
    void zoom(float yOffset);
    void adjustFOV(float offset);
    void lerp(float delta);

    float getFOV() const;
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getForwardVector() const;

    void setTarget(glm::vec3 position);
    void setThetaPhi(float theta, float phi);
    void setTargetDistance(float distance);
    void simpleReset();

    friend class CameraMenu;
};

glm::vec3 sphericalToCartesian(float theta, float phi, float radius);
