#pragma once
#include "geometry-manager.h"
#include "resources.h"
#include "GLFW/glfw3.h"
#include "camera.h"
#include "instance-resource.h"
#include "device-resource.h"
#include "surface-resource.h"

// A constant used around Vulkan operations to throw exceptions when the operations fail
#define THROW(x,msg) if (x != VK_SUCCESS) throw std::runtime_error(std::string(msg) + std::string(" - Failed with code: ") + std::to_string(x));

// Used when pulling CMAKE compile definitions
#define STRING(x) #x
#define XSTRING(x) STRING(x)

struct ViewportSettings {
    uint32_t width;
    uint32_t height;
    float minDepth;
    float maxDepth;
    float nearPlane;
    float farPlane;
    uint32_t x;
    uint32_t y;
};

struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};

struct Queues {
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
};

class AppBase {
    public:
    Resources resources;
    GeometryManager geometryManager;
    QueueFamilyIndices queueFamilyIndices;
    ViewportSettings viewportSettings;
    VkPhysicalDevice physicalDevice;
    GLFWwindow* window;
    AppCamera appCamera;
    Queues queues;
    AppInstance instance;
    AppSurface surface;
    AppDevice logicalDevice;

    VkDevice getDevice() { return logicalDevice.get(); }
    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkSurfaceKHR getSurface() { return surface.get(); }
    VkInstance getInstance() { return instance.get(); }
};