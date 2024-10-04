#pragma once
#include <resource-list.h>

class ImageList : public ResourceList<VkImage> {
    public:
    virtual void destroy(std::list<VkImage>::iterator it, VkDevice device) {
        vkDestroyImage(device, *it, nullptr);
        ResourceList::destroy(it);
    };
};