#pragma once
#include "resource-structs.h"

class WindowManager {
    uint32_t viewportHeight = 1080U;
    uint32_t viewportWidth = 1920U;

    /* Vulkan objects*/
    struct GLFWwindow* window;

    // The Vulkan surface that we render onto
    AppSurface surface;

    // The swapchain 
    AppSwapchain swapchain;

    void createWindow();
};