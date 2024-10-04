#pragma once
#include "app-resource.h"

class AppFence : AppResource<VkFence> {
    public:
    void init(VulkanApp* app, VkFenceCreateFlags flags = 0U);

    void destroy() { getApp()->resources.fences.destroy(getIterator(), getApp()->logicalDevice.get()); }
};