#pragma once
#include "resource-list.h"

class InstanceList : public ResourceList<VkInstance> {
    public:
    virtual void destroy(std::list<VkInstance>::iterator it) {
        vkDestroyInstance(*it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll() { while (!resourceList.empty()) destroy(resourceList.begin());}
};