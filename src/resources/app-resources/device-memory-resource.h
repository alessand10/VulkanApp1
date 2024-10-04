#pragma once
#include "app-resource.h"
#include "buffer-resource.h"

class AppDeviceMemory : public AppResource<VkDeviceMemory> {
    public:
    void init(VulkanApp* app, AppBuffer buffer);
    void init(VulkanApp* app, AppImage image);
    
    void destroy() { getApp()->resources.deviceMemorySet.destroy(getIterator(), getApp()->logicalDevice.get()); }
};