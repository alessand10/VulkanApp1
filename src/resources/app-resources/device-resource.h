#pragma once
#include "app-resource.h"

class AppDevice : public AppResource<VkDevice> {
    public:
    void init(class VulkanApp* app, VkPhysicalDevice physicalDevice);
    
    void destroy();
};