#pragma once
#include "resource-list.h"

class RenderPassList : public ResourceList<VkRenderPass> {
    public:
    virtual void destroy(std::list<VkRenderPass>::iterator it, VkDevice device) {
        vkDestroyRenderPass(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};