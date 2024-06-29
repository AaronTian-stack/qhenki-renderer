#pragma once

#include "transform.h"

class Joint
{
private:
    Transform transform;
    glm::mat4 inverseBindMatrix;
public:
    Joint();
    Joint(const glm::mat4 &inverseBindMatrix);
};
