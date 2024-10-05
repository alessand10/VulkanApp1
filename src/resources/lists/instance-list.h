#pragma once
#include "resource-list.h"

class InstanceList : public ResourceList<VkInstance> {
    public:
    virtual void destroy(std::list<VkInstance>::iterator it) {
        vkDestroyInstance(*it, nullptr);
        ResourceList::destroy(it);
    }
    void destroyAll() {for (auto it = resourceList.begin(); it != resourceList.end(); it++) destroy(it);}
};