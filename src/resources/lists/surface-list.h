#pragma once
#include "resource-list.h"

class SurfaceList : public ResourceList<VkSurfaceKHR> {
    public:
    virtual void destroy(std::list<VkSurfaceKHR>::iterator it, VkInstance instance) {
        vkDestroySurfaceKHR(instance, *it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll(VkInstance instance) { while (!resourceList.empty()) destroy(resourceList.begin(), instance);}
};