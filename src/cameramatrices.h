#include "glm/detail/type_mat4x4.hpp"

struct CameraMatrices
{
    alignas(16) glm::vec4 position;
    alignas(16) glm::vec4 forward;
    alignas(16) glm::mat4 viewProj;
    alignas(16) glm::mat4 inverseViewProj;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};