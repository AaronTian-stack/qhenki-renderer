#include "framebuffer.h"
#include <unordered_map>

enum class GBufferAttachmentType : unsigned int
{
    ALBEDO,
    NORMAL,
    METAL_ROUGHNESS_AO,
    EMISSIVE,
    DEPTH,
    OUTPUT,
    END
};

class GBuffer : public FrameBuffer
{
private:
    std::array<FrameBufferAttachment*, (unsigned int)GBufferAttachmentType::END> attachmentMap{};
    std::unordered_map<FrameBufferAttachment*, GBufferAttachmentType> attachmentIndexMap;

    std::array<vk::Framebuffer, (unsigned int)GBufferAttachmentType::END> individualFrameBuffers;

public:
    explicit GBuffer(vk::Device device);

    FrameBufferAttachment* getAttachment(GBufferAttachmentType type);
    void setIndividualFramebuffer(GBufferAttachmentType type, vk::Framebuffer framebuffer);
    vk::Framebuffer getIndividualFramebuffer(GBufferAttachmentType type);

    void setAttachment(GBufferAttachmentType type, const sPtr<FrameBufferAttachment> &attachment);
    void createFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent);
};
