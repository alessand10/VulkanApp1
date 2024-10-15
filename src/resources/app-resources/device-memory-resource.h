#pragma once
#include "app-resource.h"
#include "buffer-resource.h"
#include "image-resource.h"

class AppDeviceMemory : public AppResource<VkDeviceMemory> {
    uint32_t size;
    public:
    void init(class AppBase* appBase, AppBuffer buffer);
    void init(class AppBase* appBase, AppImage image);
    uint32_t getSize() { return size; }
    
    void destroy();
};