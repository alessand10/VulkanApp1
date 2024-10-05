#pragma once
#include "resource-list.h"

class BufferList : public ResourceList<VkBuffer> {
    public:
    virtual void destroy(std::list<VkBuffer>::iterator it, VkDevice device) {
        vkDestroyBuffer(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};