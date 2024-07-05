#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
    glm::vec3 translate;
    glm::quat rotation;
    glm::vec3 scale = glm::vec3(1.f);
};