#pragma once
#include "resource-list.h"

class FramebufferList : public ResourceList<VkFramebuffer> {
    public:
    virtual void destroy(std::list<VkFramebuffer>::iterator it, VkDevice device) {
        vkDestroyFramebuffer(device, *it, nullptr);
        ResourceList::destroy(it);
    };
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};