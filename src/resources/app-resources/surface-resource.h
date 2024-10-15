#pragma once
#include "app-resource.h"

class AppSurface : public AppResource<VkSurfaceKHR> {
    public:
    void init(class AppBase* appBase, struct GLFWwindow* window);
    void destroy();
};