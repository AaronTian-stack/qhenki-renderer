#include "model.h"
#include <iostream>

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
    for (auto &skin : skins)
    {
        for (auto &jointBuffer : skin.jointBuffers)
        {
            jointBuffer->destroy();
        }
    }
}

void Model::skin(vk::CommandBuffer commandBuffer)
{

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

void Model::updateAnimation(float time, int frame)
{
    if (!animations.empty())
    {
        auto &animation = animations[0];
        for (auto &channel : animation.channels)
        {
            auto &sampler = animation.samplers[channel.sampler];
            auto &node = channel.node;

            // cast sampler input to floats
            auto &inputBuffer = animationRawData[sampler.input];
            assert(inputBuffer.size() % sizeof(float) == 0);
            const auto size = inputBuffer.size() * sizeof(unsigned char) / sizeof(float);
            auto inputTimes = reinterpret_cast<float*>(inputBuffer.data());

            // pretty sure gltf animations always start at 0
            const auto newTime = fmod(time, inputTimes[size - 1]);

            int timeIndex = 0;
            for (int i = 0; i < size - 1; i++)
            {
                if (newTime >= inputTimes[i] && newTime < inputTimes[i + 1])
                {
                    timeIndex = i;
                    break;
                }
            }

            auto &outputBuffer = animationRawData[sampler.output];

            float interpolationValue = (newTime - inputTimes[timeIndex]) / (inputTimes[timeIndex + 1] - inputTimes[timeIndex]);

            if (channel.path == TargetPath::TRANSLATION)
            {
                assert(outputBuffer.size() % sizeof(glm::vec3) == 0);
                auto translations = reinterpret_cast<glm::vec3*>(outputBuffer.data());
                node->setTranslation(glm::mix(translations[timeIndex], translations[timeIndex + 1], interpolationValue));
            }
            else if (channel.path == TargetPath::ROTATION)
            {
                assert(outputBuffer.size() % sizeof(glm::quat) == 0);
                auto rotations = reinterpret_cast<glm::quat*>(outputBuffer.data());
                node->setRotation(glm::slerp(rotations[timeIndex], rotations[timeIndex + 1], interpolationValue));
            }
            else if (channel.path == TargetPath::SCALE)
            {
                assert(outputBuffer.size() % sizeof(glm::vec3) == 0);
                auto scales = reinterpret_cast<glm::vec3*>(outputBuffer.data());
                node->setScale(glm::mix(scales[timeIndex], scales[timeIndex + 1], interpolationValue));
            }
        }
    }
    for (auto &skin : skins)
    {
        // update the transformation of the joint and write to SSBO
        auto inverseBindMatrices = static_cast<glm::mat4*>(skin.jointBuffers[frame]->getPointer());
        for (int i = 0; i < skin.nodeBindMatrices.size(); i++)
        {
            auto &nodeBindMatrix = skin.nodeBindMatrices[i];
            auto &jointNode = nodeBindMatrix.node;
            auto &inverseBindMatrix = nodeBindMatrix.inverseBindMatrix;

            // update joint matrix
            inverseBindMatrices[i] = jointNode->getWorldTransform() * inverseBindMatrix;
        }
    }
}

Node *Model::getRoot()
{
    return root.get();
}
