#include "vulkan-app.h"
#include "instance-resource.h"
#include "GLFW/glfw3.h"

void AppInstance::init(VulkanApp *app, const char* appName, bool enableValidationLayers)
{

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkApplicationInfo appCreateInfo{};
    appCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appCreateInfo.pApplicationName = appName;
    appCreateInfo.applicationVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.pEngineName = "No Engine";
    appCreateInfo.engineVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    
    // Retrieve the vulkan extensions required by glfw
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  
    std::vector<const char*> extensions = {};
    for (int index = 0 ; index < glfwExtensionCount ; index++) extensions.push_back(glfwExtensions[index]);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appCreateInfo;
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledExtensionCount = extensions.size();
    instanceCreateInfo.pNext = NULL;
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = NULL;
    }


    VkInstance instance = VK_NULL_HANDLE;
    THROW(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Failed to create Vulkan instance");

    AppResource::init(app, app->resources.instances.create(instance));
}

void AppInstance::destroy()
{
    getApp()->resources.instances.destroy(getIterator());
}
