#pragma once
#include "resource-list.h"

class PipelineLayoutList : public ResourceList<VkPipelineLayout> {
    public:
    virtual void destroy(std::list<VkPipelineLayout>::iterator it, VkDevice device) {
        vkDestroyPipelineLayout(device, *it, nullptr);
        ResourceList::destroy(it);
    }
};