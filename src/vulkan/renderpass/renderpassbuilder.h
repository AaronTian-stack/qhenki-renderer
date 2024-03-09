#include "../destroyable.h"
#include "renderpass.h"
#include "../../smartpointer.h"

class RenderPassBuilder : public Destroyable
{
private:
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::AttachmentReference> attachmentRefs;

    std::vector<std::vector<vk::AttachmentReference>> refsVector;
    std::vector<vk::SubpassDescription> subPasses;
    //std::vector<vk::SubpassDependency> dependencies;

public:

    void addColorAttachment(vk::Format format);
    void addDepthAttachment(vk::Format format);
    void addAttachment(vk::AttachmentDescription *attachment, vk::ImageLayout layout);

    void addSubPass(const std::vector<uint32_t> &indices, int depthIndex = -1);
    //void addDependency(vk::SubpassDependency dependency);

    uPtr<RenderPass> buildRenderPass();
    void reset();
    void destroy() override;
};
