#pragma once
#include "app-resource.h"

class AppSurface : public AppResource<VkSurfaceKHR> {
    public:
    void init(VulkanApp* app, struct GLFWwindow* window);
    void destroy() { getApp()->resources.surfaces.destroy(getIterator(), getApp()->logicalDevice.get()); }
};