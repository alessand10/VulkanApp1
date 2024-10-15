#pragma once
#include "resource-list.h"

class PipelineList : public ResourceList<VkPipeline> {
    public:
    virtual void destroy(std::list<VkPipeline>::iterator it, VkDevice device) {
        vkDestroyPipeline(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) { while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};