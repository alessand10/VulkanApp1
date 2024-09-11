#pragma once
#include "vulkan/vulkan.hpp"
#include "resource-structs.h"
#include <map>

/**
 * @class ResourceManager
 * @brief Manages the creation and destruction of Vulkan resources.
 */

class ResourceManager {
    private:
    
    public:
    class VulkanApp* app;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkDeviceMemory> imageMemory;

    void init(class VulkanApp* appRef);
    uint32_t getSuitableMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags propertyFlags);

    /**
     * @brief Creates an app-managed image using one of the defined templates
     * 
     * @note The created image is automatically destroyed when the app terminates.
     * 
     * @param width The width of the image
     * @param height The height of the image
     * @param appImageTemplate The template to create the image from
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    AppImage2D createImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers = 1U);

    /**
     * @brief Creates an app-managed image view based on the image & template passed in
     * 
     * @note The created image view is automatically destroyed when the app terminates.
     * @note The created view is returned via the image structure argument.
     * 
     * @param image The app image object to create a view for
     * @param layerCount The number of array layers that this image view will encompass
     * @param baseLayer The starting array layer that this image view will encompass
     */
    void createImageView(AppImage2D &image, uint32_t layerCount = 1U, uint32_t baseLayer = 0U);

    /**
     * @brief Allocates memory for an app image
     * 
     * @note The allocated image memory is automatically freed when the app terminates.
     * @note Memory allocation flags are set based on the image creation template.

     * @param image The app image object to allocate memory for
     */
    void allocateImageMemory(AppImage2D &image);
    void bindImageToMemory(AppImage2D &image);


    /**
     * @brief Creates an app-managed image, image memory, and image view. The image is also bound to its memory.
     * 
     * @note The created image, image view and memory are destroyed/freed automatically
     * 
     * @param width The width of the image
     * @param height The height of the image
     * @param appImageTemplate The template to create the image from
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    AppImage2D createImageAll(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount = 1U);

    /**
     * @brief Transitions an image from its current layout to a new layout
     * 
     * @param image The image to transition the layout of
     * @param newLayout The layout to transition the new image to
     */
    void transitionImageLayout(AppImage2D &image, VkImageLayout newLayout);

    /**
     * @brief Create an app-managed image view based on the image & template passed in
     * 
     * @note The returned image will be in the layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, it can be
     * transitioned for further use with the transitionImageLayout() method.
     * 
     * @param stagingImage The staging image to copy data from
     * @param deviceLocalImage The device-local image to copy data to
     */
    void pushStagingImage(AppImage2D &stagingImage, AppImage2D &deviceLocalImage);

    std::vector<VkBuffer> buffers = {};
    std::vector<VkDeviceMemory> bufferMemory = {};

    AppBuffer createBuffer(AppBufferTemplate bufferTemplate, size_t size);
    void allocateBufferMemory(AppBuffer &appBuffer);
    void bindBufferToMemory(AppBuffer &appBuffer);
    AppBuffer createBufferAll(AppBufferTemplate bufferTemplate, size_t size);
    void pushStagingBuffer(AppBuffer &stagingBuffer, AppBuffer &deviceLocalBuffer);
    void copyDataToStagingBuffer(AppBuffer &stagingBuffer, void* memory, size_t size);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDescriptorPool> descriptorPools;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<AppDescriptorItemTemplate> descriptorItems);
    VkDescriptorPool createDescriptorPool(uint32_t maxSetsCount, std::map<AppDescriptorItemTemplate, uint32_t> descriptorTypeCounts);
    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
    void updateImageDescriptor(AppImage2D image, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, VkSampler sampler = nullptr);
    void updateBufferDescriptor(VkDescriptorSet set, VkBuffer buffer, uint32_t size,  uint32_t binding, AppDescriptorItemTemplate itemTemplate);

    void destroy();

};