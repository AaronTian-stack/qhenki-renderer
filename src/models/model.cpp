#include "model.h"

Model::Model()
{

}

void Model::destroy()
{
    for (auto &mesh : meshes)
    {
        mesh->destroy();
    }
}

void Model::draw(vk::CommandBuffer commandBuffer)
{
    for (auto &mesh : meshes)
    {
        mesh->draw(commandBuffer);
    }
}
