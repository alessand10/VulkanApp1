#pragma once
#include "resource-list.h"

class SemaphoreList : public ResourceList<VkSemaphore> {
    public:
    virtual void destroy(std::list<VkSemaphore>::iterator it, VkDevice device) {
        vkDestroySemaphore(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};