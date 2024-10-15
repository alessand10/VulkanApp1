#pragma once
#include "resource-list.h"

class ImageViewList : public ResourceList<VkImageView> {
    public:
    virtual void destroy(std::list<VkImageView>::iterator it, VkDevice device) {
        vkDestroyImageView(device, (*it), nullptr);
        ResourceList::destroy(it);
    }

    void destroyAll(VkDevice device) { while (!resourceList.empty()) destroy(resourceList.begin(), device);}
};