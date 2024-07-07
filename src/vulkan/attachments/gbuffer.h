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
    vk::Extent2D resolution;
    std::array<Attachment*, (unsigned int)GBufferAttachmentType::END> attachmentMap{};
    std::unordered_map<Attachment*, GBufferAttachmentType> attachmentIndexMap;

public:
    GBuffer(vk::Device device, vk::Extent2D extent);

    Attachment* getAttachment(GBufferAttachmentType type);
    vk::Extent2D getResolution() { return resolution; }

    void setAttachment(GBufferAttachmentType type, uPtr<Attachment> &attachment, bool own);
    void createFrameBuffer(vk::RenderPass renderPass);
};
