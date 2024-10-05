#pragma once
#include "resource-list.h" 

class DescriptorSetLayoutList : public ResourceList<VkDescriptorSetLayout> {
    public:
    virtual void destroy(std::list<VkDescriptorSetLayout>::iterator it, VkDevice device) {
        vkDestroyDescriptorSetLayout(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};