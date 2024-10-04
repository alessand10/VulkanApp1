#pragma once
#include "app-resource.h"

class AppSemaphore : public AppResource<VkSemaphore> {
    public:
    void init(VulkanApp* app);
    void destroy() { getApp()->resources.semaphores.destroy(getIterator(), getApp()->logicalDevice.get()); }
};