#pragma once
#include "resource-list.h" 

class DescriptorPoolList : public ResourceList<VkDescriptorPool> {
    public:
    virtual void destroy(std::list<VkDescriptorPool>::iterator it, VkDevice device) {
        vkDestroyDescriptorPool(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
    void destroyAll(VkDevice device) {while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};