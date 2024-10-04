#pragma once
#include "app-resource.h"

class AppInstance : AppResource<VkInstance> {
    public:
    void init(VulkanApp* app, const char* appName, bool enableValidationLayers);
    
    void destroy() { getApp()->resources.instances.destroy(getIterator()); }
};