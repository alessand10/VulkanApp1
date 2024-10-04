#pragma once
#include "app-resource.h"

class AppDevice : public AppResource<VkDevice> {
    public:
    void init(VulkanApp* app);
    
    void destroy() { getApp()->resources.devices.destroy(getIterator()); }
};