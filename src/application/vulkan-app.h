#pragma once
#include "glm/glm.hpp"
#include <map>
#include "resource-utilities.h"
#include "geometry-utilities.h"
#include <chrono>
#include "app-base.h"


class VulkanApp : public AppBase {
    std::chrono::high_resolution_clock::time_point lastRenderTime;
    std::chrono::high_resolution_clock highResClock;
    
    public:
    std::chrono::duration<double> deltaTime;

    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

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