#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<char> Shader::readFile(const std::string &filename)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        file.open(filename, std::ios::ate | std::ios::binary);
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR: SHADER: " << filename << " FILE_NOT_SUCCESFULLY_READ" << std::endl;
        std::runtime_error("File not successfully read");
    }
    return {};
}

vk::ShaderModule Shader::createShaderModule(const std::string &filePath)
{
    std::vector<char> code = readFile(filePath);
    vk::ShaderModuleCreateInfo createInfo(
    vk::ShaderModuleCreateFlags(),
            code.size(),
            reinterpret_cast<const uint32_t *>(code.data())
    );
    return device.createShaderModule(createInfo);
}

Shader::Shader(vk::Device device, const char* vertShaderPath, const char* fragShaderPath) : Destroyable(device)
{
    vertShaderModule = createShaderModule(vertShaderPath);
    fragShaderModule = createShaderModule(fragShaderPath);

    auto vertShaderStageInfo = vertexStageInfo(vertShaderModule);
    auto fragShaderStageInfo = fragmentStageInfo(fragShaderModule);

    shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
}

vk::PipelineShaderStageCreateInfo Shader::vertexStageInfo(const vk::ShaderModule &vertShaderModule)
{
    return {
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eVertex, // vertex shader
        vertShaderModule,
        "main" // entry point, could combine multiple shaders into one module. however not possible with GLSL
    };
}

vk::PipelineShaderStageCreateInfo Shader::fragmentStageInfo(vk::ShaderModule const &fragShaderModule)
{
    return {
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eFragment,
        fragShaderModule,
        "main"
    };
}

void Shader::destroy()
{
    device.destroyShaderModule(fragShaderModule);
    device.destroyShaderModule(vertShaderModule);
}
