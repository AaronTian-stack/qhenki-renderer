#pragma once

class Disposable
{
protected:
    VkDevice deviceForDispose = VK_NULL_HANDLE;

public:
    Disposable() {}
    Disposable(VkDevice device) : deviceForDispose(device) {}
    virtual void dispose() = 0;
};
