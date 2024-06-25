#include <glm/detail/type_mat4x4.hpp>

struct CameraMatrices
{
    glm::vec3 position;
    glm::vec3 forward;
    glm::mat4 viewProj;
    glm::mat4 inverseViewProj;
    glm::mat4 view;
    glm::mat4 proj;
};