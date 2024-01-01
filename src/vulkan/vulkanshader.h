#include "vulkan/vulkan.h"
#include "../disposable.h"
#include <vector>

class VulkanShader : public Disposable
{
private:
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    // vulkan wrapper for shader code
    VkShaderModule createShaderModule(const std::string &filePath);

    static VkPipelineShaderStageCreateInfo vertexStageInfo(const VkShaderModule &vertShaderModule);
    static VkPipelineShaderStageCreateInfo fragmentStageInfo(const VkShaderModule &fragShaderModule);

public:
    static std::vector<char> readFile(const std::string &filename);

    VulkanShader(VkDevice device, const char* vertShaderPath, const char* fragShaderPath);

    const std::array<VkPipelineShaderStageCreateInfo, 2> &getShaderStages() const { return shaderStages; }

    void dispose() override;
};
