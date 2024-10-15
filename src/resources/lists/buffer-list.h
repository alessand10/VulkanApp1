#pragma once
#include "resource-list.h"

class BufferList : public ResourceList<VkBuffer> {
    public:
    virtual void destroy(std::list<VkBuffer>::iterator it, VkDevice device) {
        vkDestroyBuffer(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) {while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};