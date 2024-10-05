#pragma once
#include "app-resource.h"

class AppDevice : public AppResource<VkDevice> {
    public:
    void init(class VulkanApp* app, VkPhysicalDevice physicalDevice, std::vector<const char*> layers = {}, std::vector<const char*> extensions = {});
    
    void destroy();
};