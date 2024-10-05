#pragma once
#include "app-resource.h"

class AppFence : AppResource<VkFence> {
    public:
    void init(class VulkanApp* app, VkFenceCreateFlags flags = 0U);

    void destroy();
};