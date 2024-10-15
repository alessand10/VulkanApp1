#pragma once
#include "resource-list.h" 

class DescriptorSetLayoutList : public ResourceList<VkDescriptorSetLayout> {
    public:
    virtual void destroy(std::list<VkDescriptorSetLayout>::iterator it, VkDevice device) {
        vkDestroyDescriptorSetLayout(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) {while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};