#pragma once
#include "resource-list.h"

class DeviceList : public ResourceList<VkDevice> {
    public:
    virtual void destroy(std::list<VkDevice>::iterator it) {
        vkDestroyDevice(*it, nullptr);
        ResourceList::destroy(it);
    }  
};