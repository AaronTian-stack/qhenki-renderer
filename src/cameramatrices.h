#include "glm/detail/type_mat4x4.hpp"

struct CameraMatrices
{
    alignas(16) glm::vec4 position;
    alignas(16) glm::mat4 viewProj;
};