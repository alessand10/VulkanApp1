#pragma once
#include "app-resource.h"

class AppSurface : public AppResource<VkSurfaceKHR> {
    public:
    void init(class VulkanApp* app, struct GLFWwindow* window);
    void destroy();
};