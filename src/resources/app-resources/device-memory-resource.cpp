#include "vulkan-app.h"
#include "device-memory-resource.h"
#include "buffer-resource.h"
#include "image-resource.h"

uint32_t getSuitableMemoryTypeIndex(VulkanApp* app, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags propertyFlags)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memoryProperties);
    
    // A bitmask where (1 << i) is set iff memoryProperties.memoryType[i] is supported given the requirements
    for (uint32_t index = 0 ; index < memoryProperties.memoryTypeCount ; index++) {
        // Is memory at index 'index' supported? (indicated by 1 in bitmask), does the memory at that index support all of the desired property flags? 
        if ((memoryRequirements.memoryTypeBits & (1 << index)) && ((memoryProperties.memoryTypes[index].propertyFlags & propertyFlags) == propertyFlags)) {
            return index;
        }
    }

    throw std::runtime_error("Unable to find a suitable memory type");
}

void AppDeviceMemory::init(VulkanApp *app, AppBuffer buffer)
{
    AppDeviceMemory returnDeviceMemory{};
    
    VkMemoryPropertyFlags memoryPropertyFlags = 0U;

    switch (buffer.getTemplate()) {
        case AppBufferTemplate::UNIFORM_BUFFER :
        case AppBufferTemplate::VERTEX_BUFFER_STAGING :
        case AppBufferTemplate::INDEX_BUFFER_STAGING :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case AppBufferTemplate::VERTEX_BUFFER_DEVICE :
        case AppBufferTemplate::INDEX_BUFFER_DEVICE :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
    };

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(app->logicalDevice.get(), buffer.get(), &bufferMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(app, bufferMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;

    VkDeviceMemory deviceMemory;
    THROW(vkAllocateMemory(app->logicalDevice.get(), &memoryAllocateInfo, nullptr, &deviceMemory), "Failed to allocate buffer memory");
    
    AppResource::init(app, app->resources.deviceMemorySet.create(deviceMemory));
}

void AppDeviceMemory::init(VulkanApp *app, AppImage image)
{
    VkMemoryPropertyFlags memoryPropertyFlags = 0U;

    switch (image.getTemplate()) {
        case AppImageTemplate::DEPTH_STENCIL :
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;

        case AppImageTemplate::STAGING_IMAGE_TEXTURE : {
            memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        }
    }

    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), image.get(), &imageMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(app, imageMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = imageMemoryRequirements.size;

    VkDeviceMemory deviceMemory;
    THROW(vkAllocateMemory(app->logicalDevice.get(), &allocInfo, nullptr, &deviceMemory), "Failed to allocate image memory");

    AppResource::init(app, app->resources.deviceMemorySet.create(deviceMemory));
}

void AppDeviceMemory::destroy()
{
    getApp()->resources.deviceMemorySet.destroy(getIterator(), getApp()->logicalDevice.get()); 
}
