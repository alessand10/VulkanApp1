#pragma once
#include "resource-list.h"

class DeviceMemoryList : public ResourceList<VkDeviceMemory> {
    public:
    virtual void destroy(std::list<VkDeviceMemory>::iterator it, VkDevice device) {
        vkFreeMemory(device, *it, nullptr);
        ResourceList::destroy(it);
    };
};