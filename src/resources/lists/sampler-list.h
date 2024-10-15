#pragma once
#include "resource-list.h"

class SamplerList : public ResourceList<VkSampler> {
    public:
    virtual void destroy(std::list<VkSampler>::iterator it, VkDevice device) {
        vkDestroySampler(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) { while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};