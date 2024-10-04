#pragma once
#include "app-resource.h"

class AppCommandPool : public AppResource<VkCommandPool>{
    uint32_t queueFamilyIndex;
    public:
    void init(VulkanApp* app, uint32_t queueFamilyIndex);

    VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel level);

    void destroy() { getApp()->resources.commandPools.destroy(getIterator(), getApp()->logicalDevice.get()); }
};