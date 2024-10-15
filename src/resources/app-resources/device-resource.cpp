#include "app-base.h"
#include "device-resource.h"

void AppDevice::init(AppBase* appBase, VkPhysicalDevice physicalDevice, std::vector<const char*> layers, std::vector<const char*> extensions)
{
    VkDevice device = VK_NULL_HANDLE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    
    float queuePriority = 1.f;

    std::vector<uint32_t> uniqueQueueFamilies = {};

    const std::vector<uint32_t> queueFamilyIndices = {
        appBase->queueFamilyIndices.graphics,
        appBase->queueFamilyIndices.compute,
        appBase->queueFamilyIndices.transfer
    };

    // The queue create info structures, we create one per unique queue family index
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

    // Populate the queue create info portion of the structure
    for (auto it = queueFamilyIndices.begin() ; it != queueFamilyIndices.end() ; ++it) {
        if (std::find(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end(), *it) == uniqueQueueFamilies.end()) {
            uniqueQueueFamilies.push_back(*it);
            queueCreateInfos.push_back(
                VkDeviceQueueCreateInfo {
                    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    nullptr, // pNext
                    0U, // flags
                    *it, // Queue family
                    1U, // Count, only create 1 queue
                    &queuePriority // Priority
                }
            );
        }
    }

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDevice logicalDevice;
    THROW(vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice), "Failed to create logical device");

    AppResource::init(appBase, appBase->resources.devices.create(logicalDevice));
}

void AppDevice::destroy()
{
    appBase->resources.devices.destroy(getIterator());
}
