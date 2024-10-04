#pragma once
#include "resource-list.h"

class ShaderModuleList : public ResourceList<VkShaderModule> {
    public:
    virtual void destroy(std::list<VkShaderModule>::iterator it, VkDevice device) {
        vkDestroyShaderModule(device, *it, nullptr);
        ResourceList::destroy(it);
    }
};