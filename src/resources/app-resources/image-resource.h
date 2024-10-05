#pragma once
#include "app-resource.h"

enum class AppImageTemplate {
    PREWRITTEN_SAMPLED_TEXTURE = 0U,
    STAGING_IMAGE_TEXTURE = 1U,
    DEVICE_WRITE_SAMPLED_TEXTURE = 2U,
    DEPTH_STENCIL,
    SWAPCHAIN_FORMAT
};


class AppImage : public AppResource<VkImage> {
    AppImageTemplate imageCreationTemplate;
    uint32_t layerCount;
    uint32_t width;
    uint32_t height;
    VkImageLayout layout;
    public:
    AppImageTemplate getTemplate() { return imageCreationTemplate; }

    /**
     * @brief Initializes an app-managed image using one of the defined templates
     * 
     * @note The created image is automatically destroyed when the app terminates.
     * 
     * @param app The application object
     * @param appImageTemplate The template to create the image from
     * @param width The width of the image
     * @param height The height of the image
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    void init(class VulkanApp* app, AppImageTemplate appImageTemplate, uint32_t height, uint32_t width, uint32_t layerCount = 1U, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);

    /**
     * @brief Initializes an app-managed image using an existing Vulkan image
     * 
     * @note The created image is automatically destroyed when the app terminates.
     * 
     * @param app The application object
     * @param image The existing Vulkan image to use
     * @param appImageTemplate The template that matches this image
     * @param width The width of the image
     * @param height The height of the image
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    void init(class VulkanApp* app, VkImage image, AppImageTemplate appImageTemplate, uint32_t height, uint32_t width, uint32_t layerCount, VkImageLayout layout);

    /**
     * @brief Transitions an image from its current layout to a new layout
     * 
     * The application must have a command buffer to transition an image's layout.
     * 
     * @param newLayout The layout to transition the new image to
     * @param commandBuffer The command buffer to execute the layout transition on
     * @param targetLayer The layer to transition the layout of
     * @param layerCount The number of layers to transition the layout of
     */
    void transitionLayout(VkImageLayout newLayout, VkCommandBuffer commandBuffer, uint32_t targetLayer = 0U, uint32_t layerCount = 1U);

    void bindToMemory(class AppDeviceMemory *imageMemory);

    static void copyImage(AppImage &src, AppImage &dst, VkCommandBuffer commandBuffer, uint32_t srcLayer, uint32_t dstLayer, uint32_t layerCount, VkImageAspectFlags srcAspect = VK_IMAGE_ASPECT_COLOR_BIT, VkImageAspectFlags dstAspect = VK_IMAGE_ASPECT_COLOR_BIT);

    void destroy();
};