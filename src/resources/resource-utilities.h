#pragma once
#include "resource-structs.h"
#include <map>
#include "sampler-resource.h"
#include "image-view-resource.h"
#include "buffer-resource.h"

void loadJPEGImage(const char* path, AppImage image, VkCommandBuffer commandBuffer, uint32_t targetLayer, class VulkanApp* app);

/**
 * @brief Renders a cube map to image array with 6 layers
 * 
 * @note Rendered as follows:
 * Layer 0: Positive X
 * Layer 1: Negative X
 * Layer 2: Positive Y
 * Layer 3: Negative Y
 * Layer 4: Positive Z
 * Layer 5: Negative Z
 */
void renderCubeMap(AppImage imageArray);



AppImage addExistingImage(VulkanApp* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers, VkImageLayout layout, VkImage image);

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
AppImageView createImageView(VulkanApp* app, AppImage &image, uint32_t layerCount = 1U, uint32_t baseLayer = 0U);

/**
 * @brief Allocates memory for an app image
 * 
 * @note The allocated image memory is automatically freed when the app terminates.
 * @note Memory allocation flags are set based on the image creation template.

    * @param image The app image object to allocate memory for
    */
AppDeviceMemory allocateImageMemory(VulkanApp* app, AppImage &image);
void bindImageToMemory(VulkanApp* app, AppImage &image, AppDeviceMemory &imageMemory);


/**
 * @brief Creates an app-managed image, image memory, and image view. The image is also bound to its memory.
 * 
 * @note The created image, image view and memory are destroyed/freed automatically
 * 
 * @param width The width of the image
 * @param height The height of the image
 * @param appImageTemplate The template to create the image from
 * @param layerCount The number of array layers to create for this image
 * 
 * @return The created image bundle (containing an image, view and memory)
 */
AppImageBundle createImageAll(VulkanApp* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount = 1U);



/**
 * @brief Pushes the content of a staging image (with a linear layout) to a device-local image.
 * 
 * @note The returned image will be in the layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, it can be
 * transitioned for further use with the transitionImageLayout() method.
 * 
 * @param stagingImage The staging image to copy data from
 * @param deviceLocalImage The device-local image to copy data to
 */
void pushStagingImage(AppImage &stagingImage, AppImage &deviceLocalImage, VkCommandBuffer commandBuffer, uint32_t deviceLocalLayer = 0);

std::list<VkBuffer> buffers = {};

AppBufferBundle createBufferAll(AppBufferTemplate bufferTemplate, size_t size);


/**
 * @brief Updates an image descriptor for a particular descriptor set
 */
void updateDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, AppSampler sampler = AppSampler{});
void updateDescriptor(VkBuffer buffer, VkDescriptorSet set, uint32_t size,  uint32_t binding, AppDescriptorItemTemplate itemTemplate);



void destroySwapchain(AppSwapchain swapchain);