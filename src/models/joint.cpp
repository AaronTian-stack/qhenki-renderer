#include "joint.h"

Joint::Joint()
{

}

Joint::Joint(const glm::mat4 &inverseBindMatrix) : inverseBindMatrix(inverseBindMatrix)
{

}
