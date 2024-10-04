#pragma once
#include "resource-list.h" 

class DescriptorSetLayoutList : public ResourceList<VkDescriptorSetLayout> {
    public:
    virtual void destroy(std::list<VkDescriptorSetLayout>::iterator it, VkDevice device) {
        vkDestroyDescriptorSetLayout(device, *it, nullptr);
        ResourceList::destroy(it);
    }  
};