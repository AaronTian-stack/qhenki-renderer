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
    for (auto &texture : textures)
    {
        texture->destroy();
    }
}

void Model::draw(vk::CommandBuffer commandBuffer)
{
    for (auto &mesh : meshes)
    {
        mesh->draw(commandBuffer);
    }
}

std::vector<vk::DescriptorImageInfo> Model::getDescriptorImageInfo()
{
    std::vector<vk::DescriptorImageInfo> descriptorImageInfo;
    descriptorImageInfo.reserve(textures.size());
    for (auto &texture : textures)
    {
        descriptorImageInfo.push_back(texture->getDescriptorInfo());
    }
    return descriptorImageInfo;
}
