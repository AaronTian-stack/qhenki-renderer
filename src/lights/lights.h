#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct LightingParameters
{
    glm::mat4 cubeMapRotation;
    float iblIntensity = 1.f;
    float emissionMultiplier = 1.f;
    int pointLightCount = 0;
    int sphereLightCount = 0;
    int tubeLightCount = 0;
    int rectangleLightCount = 0;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct SphereLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};

struct SphereLightShader
{
    glm::vec3 position;
    glm::vec3 color;
    float radius;
};

struct TubeLight
{
    glm::vec3 position; // end points are in x direction from this
    float length;
    glm::vec3 color;
    float intensity;
    float radius;
    glm::quat rotation;
    glm::vec3 eulerAngle; // TODO: eventually remove this. only ui elements should have euler angle (like godot dirty system)
};

struct TubeLightShader
{
    glm::vec3 position1;
    glm::vec3 position2;
    glm::vec3 color;
    float radius;
};

struct RectangleLight
{
    glm::vec3 position; // forward facing z direction
    glm::vec3 color;
    float intensity;
    glm::quat rotation;
    glm::vec3 eulerAngle; // TODO: eventually remove this. only ui elements should have euler angle (like godot dirty system)
    glm::vec2 size;
};

struct RectangleLightShader
{
    glm::vec3 position;
    glm::vec3 up; // not normalized, takes into account size
    glm::vec3 right; // not normalized, takes into account size
    glm::vec3 color;
};