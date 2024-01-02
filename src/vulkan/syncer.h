#include <vulkan/vulkan.h>
#include <unordered_map>
#include "../disposable.h"

class Syncer : public Disposable
{
private:
    std::unordered_map<const char*, VkSemaphore> semaphores;
    std::unordered_map<const char*, VkFence> fences;

public:
    void create(VkDevice device);
    VkSemaphore createSemaphore(const char* name);
    VkFence createFence(const char* name, bool startSignaled = false);

    VkSemaphore getSemaphore(const char* name);
    VkFence getFence(const char* name);

    void resetFence(const char* name);
    void waitForFence(const char* name);

    void dispose() override;
};
