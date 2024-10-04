#pragma once
#include "vulkan/vulkan.hpp"
#include "resource-structs.h"
#include <map>
#include <list>

/**
 * @class ResourceManager
 * @brief Manages the creation and destruction of Vulkan resources.
 * 
 * The resource manager class provides an extensive list of operations to create and destroy
 * application resources.
 */

class ResourceManager {
    private:
    std::list<VkInstance> instances;

    std::list<VkSurfaceKHR> surfaces;

    std::list<VkDevice> devices;

    std::list<VkDeviceMemory> deviceMemorySet;

    std::list<VkImage> images;

    std::list<VkImageView> imageViews;
    
    public:
    AppImage createImage();
    void destroyImage(AppImage image, VkDevice device);

    AppImageView createImageView();
    void destroyImageView(AppImageView imageView, VkDevice device);

    AppBuffer createBuffer();
    void destroyBuffer(AppBuffer, VkDevice device);



    void destroy();
};