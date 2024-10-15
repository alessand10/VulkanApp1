#pragma once
#include "resource-structs.h"
#include <map>
#include "sampler-resource.h"
#include "image-view-resource.h"
#include "buffer-resource.h"
#include "swapchain-resource.h"
#include "command-pool-resource.h"
#include "instance-resource.h"
#include "device-resource.h"
#include "shader-module-resource.h"
#include "descriptor-pool-resource.h"
#include "descriptor-set-layout-resource.h"
#include "render-pass-resource.h"
#include "framebuffer-resource.h"
#include "pipeline-layout-resource.h"
#include "pipeline-resource.h"
#include "semaphore-resource.h"
#include "fence-resource.h"
#include "device-memory-resource.h"

void loadJPEGImage(AppBase* appBase, const char* path, AppImage image, VkCommandBuffer commandBuffer, uint32_t targetLayer);

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

struct AppImageBundle {
    AppImage image;
    AppImageView imageView;
    AppDeviceMemory deviceMemory;
};

struct AppBufferBundle {
    AppBuffer buffer;
    AppDeviceMemory deviceMemory;
};

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
AppImageBundle createImageAll(class AppBase* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount = 1U);
AppBufferBundle createBufferAll(class AppBase* app, AppBufferTemplate bufferTemplate, size_t size);

/**
 * @brief Updates an image descriptor for a particular descriptor set
 */
void updateDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, VkDescriptorType descriptorType, AppSampler sampler = AppSampler{});
void updateDescriptor(AppBuffer buffer, VkDescriptorSet set, uint32_t size,  uint32_t binding, VkDescriptorType descriptorType);


