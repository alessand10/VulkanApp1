#pragma once
#include "resource-list.h"

class ImageViewList : public ResourceList<VkImageView> {
    public:
    virtual void destroy(std::list<VkImageView>::iterator it, VkDevice device) {
        vkDestroyImageView(device, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};