#pragma once
#include "resource-list.h"

class FenceList : public ResourceList<VkFence> {
    public:
    virtual void destroy(std::list<VkFence>::iterator it, VkDevice device) {
        vkDestroyFence(device, *it, nullptr);
        ResourceList::destroy(it);
    }
};