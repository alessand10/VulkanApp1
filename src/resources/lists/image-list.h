#pragma once
#include <resource-list.h>

class ImageList : public ResourceList<VkImage> {
    public:
    virtual void destroy(std::list<VkImage>::iterator it, VkDevice device) {
        vkDestroyImage(device, *it, nullptr);
        ResourceList::destroy(it);
    };
    void destroyAll(VkDevice device) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, device);}
};