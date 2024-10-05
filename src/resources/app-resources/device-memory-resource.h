#pragma once
#include "app-resource.h"
#include "buffer-resource.h"
#include "image-resource.h"

class AppDeviceMemory : public AppResource<VkDeviceMemory> {
    public:
    void init(class VulkanApp* app, AppBuffer buffer);
    void init(class VulkanApp* app, AppImage image);
    
    void destroy();
};