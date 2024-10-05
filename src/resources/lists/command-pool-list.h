#pragma once
#include "resource-list.h"

class CommandPoolList : public ResourceList<VkCommandPool> {
    public:
    virtual void destroy(std::list<VkCommandPool>::iterator it, VkDevice device) {
        vkDestroyCommandPool(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};