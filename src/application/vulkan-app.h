#pragma once
#include "glm/glm.hpp"
#include "camera.h"
#include <map>
#include "resource-utilities.h"
#include "geometry-utilities.h"
#include "resources.h"
#include <chrono>
#include "viewport-settings.h"
#include "queues.h"

struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};

// A constant used around Vulkan operations to throw exceptions when the operations fail
#define THROW(x,msg) if (x != VK_SUCCESS) throw std::runtime_error(std::string(msg) + std::string(" - Failed with code: ") + std::to_string(x));

// Used when pulling CMAKE compile definitions
#define STRING(x) #x
#define XSTRING(x) STRING(x)

class VulkanApp {
    std::chrono::high_resolution_clock::time_point lastRenderTime;
    std::chrono::high_resolution_clock highResClock;
    
    public:
    std::chrono::duration<double> deltaTime;
    Resources resources;

    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    QueueFamilyIndices queueFamilyIndices;

    ViewportSettings viewportSettings;

    AppCamera appCamera;

    /* Vulkan objects*/
    GLFWwindow* window;
    AppInstance instance;
    VkPhysicalDevice physicalDevice;
    AppDevice logicalDevice;
    AppSurface surface;
    AppSwapchain swapchain;

    // Queues to submit particular commands
    Queues queues;

    void enumeratePhysicalDevice();
    void enumerateQueueFamilies();
    void getQueues();

    /* App helper methods */
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

    /* App inititalization methods */
    void setupDebugMessenger();
    void createWindow();
    void tickTimer();

    /**
     * Input related methods
     */
    double xposOld, yposOld;
    bool wDown = false, aDown = false, sDown = false, dDown = false, eDown = false, qDown = false;
    static void glfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void appCursorPositionCallback(double xpos, double ypos);
    void appKeyCallback(int key, int scancode, int action, int mods);
    void processKeyActions();

    void internalInit();
    void internalLoop();
    
    public:
    void userInit();
    void drawFrame(float deltaTime);
    void userTick(double deltaTime);
    void cleanup();

};