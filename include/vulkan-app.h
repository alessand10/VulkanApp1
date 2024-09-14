#pragma once
#include "vulkan/vulkan.hpp"
#include "glm.hpp"
#include "camera.h"
#include <map>
#include "mesh-manager.h"
#include "resource-manager.h"
#include <chrono>

#define THROW(x,msg) if (x != VK_SUCCESS) throw std::runtime_error(msg);

class VulkanApp {
    std::chrono::high_resolution_clock::time_point lastRenderTime;
    std::chrono::high_resolution_clock highResClock;
    
    public:
    std::chrono::duration<double> deltaTime;
    MeshManager meshManager;
    ResourceManager resourceManager;

    AppImageBundle albedo;
    AppImageBundle normal;

    bool enableValidationLayers = true;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    struct VSUniformBuffer {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    };

    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
    };

    QueueFamilyIndices queueIndices;

    uint32_t windowWidth = 1080u;
    uint32_t windowHeight = 720u;

    AppCamera appCamera;

    uint32_t supportedVertexCount = 100u;
    uint32_t supportedIndexCount = 200u;

    /* Vulkan objects*/
    struct GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSurfaceKHR surface;
    AppSwapchain swapchain;
    AppImageBundle depthStencilImage;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    AppShaderModule vertexShaderModule;
    AppShaderModule fragmentShaderModule;
    VkPipeline colorGraphicsPipeline;
    VkPipeline depthGraphicsPipeline;
    AppCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Queues to submit particular commands
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    AppBufferBundle stagingVertexBuffer;
    AppBufferBundle deviceVertexBuffer;

    AppBufferBundle stagingIndexBuffer;
    AppBufferBundle deviceIndexBuffer;

    std::vector<AppBufferBundle> uniformBuffersVS;
    VkDescriptorSetLayout pipelineDescriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSetsPerFrame;

    AppSampler sampler;

    // Signal when an image is available
    VkSemaphore imageAvailableSemaphore;

    // Signal when rendering is complete
    VkSemaphore renderingFinishedSemaphore;

    // Fence is used to block execution until rendering of previous frame
    VkFence inFlightFence;

    uint32_t maxFramesInFlight = 0u;

    std::vector<void*> mappedUBOs = {};

    /* App helper methods */
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

    /* App inititalization methods */
    void setupDebugMessenger();
    void initVulkanLoader();
    void createWindow();
    void createInstance();
    void selectPhysicalDevice();
    void setQueueIndices();
    void createLogicalDeviceAndQueues();
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