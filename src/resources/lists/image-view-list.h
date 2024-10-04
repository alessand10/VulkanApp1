#pragma once
#include "resource-list.h"

class ImageViewList : public ResourceList<VkImageView> {
    public:
    virtual void destroy(std::list<VkImageView>::iterator it, VkDevice device) {
        vkDestroyImageView(device, *it, nullptr);
        ResourceList::destroy(it);
    }
};