#pragma once
#include "resource-list.h"

class SurfaceList : public ResourceList<VkSurfaceKHR> {
    public:
    virtual void destroy(std::list<VkSurfaceKHR>::iterator it, VkInstance instance) {
        vkDestroySurfaceKHR(instance, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkInstance instance) {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it, instance);}
};