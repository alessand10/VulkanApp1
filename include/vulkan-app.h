#pragma once
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "camera.h"
#include <map>
#include "mesh-manager.h"
#include "resource-manager.h"
#include <chrono>

// A constant used around Vulkan operations to throw exceptions when the operations fail
#define THROW(x,msg) if (x != VK_SUCCESS) throw std::runtime_error(msg);

// Used when pulling CMAKE compile definitions
#define STRING(x) #x
#define XSTRING(x) STRING(x)

class VulkanApp {
    std::chrono::high_resolution_clock::time_point lastRenderTime;
    std::chrono::high_resolution_clock highResClock;
    
    public:
    std::chrono::duration<double> deltaTime;
    MeshManager meshManager;
    ResourceManager resourceManager;

    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    QueueFamilyIndices queueFamilyIndices;

    uint32_t windowWidth = 1920U;
    uint32_t windowHeight = 1080U;

    AppCamera appCamera;

    uint32_t supportedVertexCount = 100u;
    uint32_t supportedIndexCount = 200u;

    /* Vulkan objects*/
    struct GLFWwindow* window;
    AppInstance instance;
    VkPhysicalDevice physicalDevice;
    AppDevice logicalDevice;
    AppSurface surface;
    AppSwapchain swapchain;

    // Queues to submit particular commands
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    uint32_t maxFramesInFlight = 0u;

    void enumeratePhysicalDevice();

    /* App helper methods */
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

    /* App inititalization methods */
    void setupDebugMessenger();
    void createWindow();
    void tickTimer();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

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

    
    public:
    void init();
    void drawFrame(float deltaTime);
    void renderLoop();
    void cleanup();

};