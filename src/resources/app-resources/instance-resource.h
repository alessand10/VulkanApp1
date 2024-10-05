#pragma once
#include "app-resource.h"

class AppInstance : public AppResource<VkInstance> {
    public:
    void init(class VulkanApp* app, const char* appName, bool enableValidationLayers);
    
    void destroy();
};