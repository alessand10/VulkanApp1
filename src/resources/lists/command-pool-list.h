#pragma once
#include "resource-list.h"

class CommandPoolList : public ResourceList<VkCommandPool> {
    public:
    virtual void destroy(std::list<VkCommandPool>::iterator it, VkDevice device) {
        vkDestroyCommandPool(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) {while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};