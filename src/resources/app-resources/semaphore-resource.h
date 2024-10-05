#pragma once
#include "app-resource.h"

class AppSemaphore : public AppResource<VkSemaphore> {
    public:
    void init(class VulkanApp* app);
    void destroy();
};