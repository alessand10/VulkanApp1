#pragma once
#include "app-resource.h"

class AppCommandPool : public AppResource<VkCommandPool>{
    uint32_t queueFamilyIndex;
    public:
    void init(class VulkanApp* app, uint32_t queueFamilyIndex);

    VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel level);

    void destroy();
};