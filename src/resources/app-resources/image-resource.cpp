#include "vulkan-app.h"
#include "image-resource.h"
#include "device-resource.h"
#include "device-memory-resource.h"


VkImageCreateInfo getImageCreateInfoFromTemplate(AppImageTemplate t, uint32_t height, uint32_t width, uint32_t layerCount) { 
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {height, width, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            layerCount, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::STAGING_IMAGE_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {height, width, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            layerCount, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_LINEAR, //tiling
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {height, width, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            layerCount, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::DEPTH_STENCIL : 
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_D32_SFLOAT_S8_UINT, //format
            {height, width, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            layerCount, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        default: 
        return {};
    };
};

void AppImage::init(VulkanApp *app, AppImageTemplate appImageTemplate, uint32_t height, uint32_t width, uint32_t layerCount, VkImageLayout layout)
{
    this->imageCreationTemplate = appImageTemplate;
    this->height = height;
    this->width = width;
    this->layout = layout;
    this->layerCount = layerCount;

    VkImageCreateInfo createInfo{ getImageCreateInfoFromTemplate(appImageTemplate, height, width, layerCount) };

    // Attempt to create the image
    VkImage image = VK_NULL_HANDLE;
    THROW(vkCreateImage(app->logicalDevice.get(), &createInfo, nullptr, &image), "Failed to create image");

    AppResource::init(app, app->resources.images.create(image));
}

void AppImage::init(VulkanApp *app, VkImage image, AppImageTemplate appImageTemplate, uint32_t height, uint32_t width, uint32_t layerCount, VkImageLayout layout)
{
    this->imageCreationTemplate = appImageTemplate;
    this->height = height;
    this->width = width;
    this->layout = layout;
    this->layerCount = layerCount;

    AppResource::init(app, app->resources.images.create(image));
}

void AppImage::transitionLayout(VkImageLayout newLayout, VkCommandBuffer commandBuffer, uint32_t targetLayer, uint32_t layerCount)
{
    VkImageMemoryBarrier layoutTransitionBarrier{};
    layoutTransitionBarrier.pNext = nullptr;
    layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //                                {aspect mask, mip level, mip level count, array layer, array layer count}
    layoutTransitionBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 1U, targetLayer, layerCount};
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.srcAccessMask = VK_ACCESS_NONE; // We are not waiting for anything to occur prior to this barrier
    layoutTransitionBarrier.dstAccessMask = VK_ACCESS_NONE; // Nothing is awaiting this barrier
    layoutTransitionBarrier.image = this->get();
    layoutTransitionBarrier.oldLayout = this->layout;
    layoutTransitionBarrier.newLayout = newLayout;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0U, 0U, nullptr, 0U, nullptr, 1U, &layoutTransitionBarrier);
    vkEndCommandBuffer(commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = &(commandBuffer);
    submitInfo.commandBufferCount = 1U;
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0u;

    vkQueueSubmit(app->queues.graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice.get());
    
    vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);

    this->layout = newLayout;
}

void AppImage::bindToMemory(AppDeviceMemory* imageMemory)
{
    VkDevice device = this->getApp()->logicalDevice.get();
    VkImage img = get();
    VkDeviceMemory memory = imageMemory->get();

    THROW(vkBindImageMemory(device, img, memory, 0U), "Failed to bind image to memory");
}

void AppImage::copyImage(AppImage &src, AppImage &dst, VkCommandBuffer commandBuffer, uint32_t srcLayer, uint32_t dstLayer, uint32_t layerCount, VkImageAspectFlags srcAspect, VkImageAspectFlags dstAspect)
{
    VulkanApp* app = src.app;
    // Throw an error if the source and destination images are not in a valid layout for transfer
    if (!(src.layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL || src.layout == VK_IMAGE_LAYOUT_GENERAL)) {
        throw std::runtime_error("Source image must be in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL layout");
    }
    if (!(dst.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || dst.layout == VK_IMAGE_LAYOUT_GENERAL)) {
        throw std::runtime_error("Destination image must be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL layout");
    }

    // Create the command buffer submit struct
    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = &(commandBuffer);
    submitInfo.commandBufferCount = 1U;
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0u;

    // Create the command buffer begin info struct
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;

    // Create the image copy struct
    VkImageCopy imgCopy{};
    //                  {x, y, z}
    imgCopy.srcOffset = {0U, 0U, 0U};
    imgCopy.dstOffset = {0U, 0U, 0U};
    imgCopy.extent.width = src.width;
    imgCopy.extent.height = src.height;
    imgCopy.extent.depth = 1U; 
    //                       {Aspect, Mip level, Array layer, Layer count}
    imgCopy.srcSubresource = {srcAspect, 0U, srcLayer, layerCount};
    imgCopy.dstSubresource = {dstAspect, 0U, dstLayer, layerCount};

    // Write the command buffer copy operation
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdCopyImage(commandBuffer, src.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &imgCopy);
    vkEndCommandBuffer(commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    vkQueueSubmit(app->queues.graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice.get());
    
    vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);
}

void AppImage::destroy()
{
    getApp()->resources.images.destroy(getIterator(), getApp()->logicalDevice.get());
}
