#include "resource-utilities.h"
#include "vulkan-app.h"
#include "turbojpeg.h"
#include "GLFW/glfw3.h"
#include "file-utilities.h"

void loadJPEGImage(VulkanApp *app, const char *path, AppImage image, VkCommandBuffer commandBuffer, uint32_t targetLayer)
{
    VkDevice device = app->logicalDevice.get();
    
    // Create a dummy staging image to fetch image requirements from (particularly, the alignment)
    AppImage dummyImage;
    dummyImage.init(app, AppImageTemplate::STAGING_IMAGE_TEXTURE, 1U, 1U);
    VkMemoryRequirements imageMemoryRequirements{};
    vkGetImageMemoryRequirements(app->logicalDevice.get(), dummyImage.get(), &imageMemoryRequirements);

    uint32_t width = 0, height = 0;

    // Specify the pixel format to retrieve the JPEG with
    int pixelFormat = TJPF_RGBA;
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);

    // Read in the jpeg image file
    std::vector<char> jpegImage = readJPEG(path, imageMemoryRequirements.alignment, &width, &height);

    // Define the staging image and memory that the CPU will write into
    AppImage stagingImage;
    AppDeviceMemory stagingImageMemory;

    // Initialize the actual staging image
    stagingImage.init(app, AppImageTemplate::STAGING_IMAGE_TEXTURE, width, height);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), stagingImage.get(), &stagingImageMemoryRequirements);

    stagingImageMemory.init(app, stagingImage);

    stagingImage.bindToMemory(&stagingImageMemory);

    void* mappedImageMemory = nullptr;

    // Copy the image into the staging image resource
    copyDataToStagingMemory(stagingImageMemory, jpegImage.data(), stagingImageMemoryRequirements.size);

    // Push the staging image contents to the device-local image
    stagingImage.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer, targetLayer);
    image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer, targetLayer);
    AppImage::copyImage(stagingImage, image, commandBuffer, 0U, targetLayer, 1U, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    // Transition the image to be used as a shader resource
    image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer, targetLayer);

    // Destroy the staging image
    stagingImage.destroy();
}

void renderCubeMap(AppImage imageArray)
{
    // 
}

AppImageBundle createImageAll(VulkanApp* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{
    AppImageBundle bundle {};
    bundle.image.init(app, appImageTemplate, width, height, layerCount);
    bundle.deviceMemory.init(app, bundle.image);
    bundle.image.bindToMemory(&bundle.deviceMemory);
    bundle.imageView.init(app, bundle.image, layerCount, 0U);
    return bundle;
}

AppBufferBundle createBufferAll(VulkanApp* app, AppBufferTemplate bufferTemplate, size_t size)
{
    AppBufferBundle bundle {};
    bundle.buffer.init(app, size, bufferTemplate);
    bundle.deviceMemory.init(app, bundle.buffer);
    bindBufferToMemory(bundle.buffer, bundle.deviceMemory);
    return bundle;
}

void bindBufferToMemory(AppBuffer &buffer, AppDeviceMemory &deviceMemory)
{
    THROW(vkBindBufferMemory(buffer.getApp()->logicalDevice.get(), buffer.get(), deviceMemory.get(), 0U), "Failed to bind buffer to memory");
}

AppBufferBundle createBufferAll(VulkanApp* app, AppBufferTemplate bufferTemplate, size_t size)
{
    AppBufferBundle bufferBundle {};
    bufferBundle.buffer.init(app, size, bufferTemplate);
    bufferBundle.deviceMemory.init(app, bufferBundle.buffer);
    bindBufferToMemory(bufferBundle.buffer, bufferBundle.deviceMemory);
    return bufferBundle;
}

void pushStagingBuffer(AppBuffer &stagingBuffer, AppBuffer &deviceLocalBuffer, VkCommandBuffer commandBuffer)
{
}

void copyDataToStagingMemory(AppDeviceMemory &stagingMemory, void *data, size_t size)
{
    void* mappedMemory = nullptr;
    vkMapMemory(stagingMemory.getApp()->logicalDevice.get(), stagingMemory.get(), 0U, size, 0U, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(stagingMemory.getApp()->logicalDevice.get(), stagingMemory.get());
}


void updateDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, VkDescriptorType descriptorType, AppSampler sampler)
{   
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = getImageLayoutFromTemplate(imageView.getTemplate());
    imageInfo.imageView = imageView.get();
    imageInfo.sampler = sampler.get();

    VkWriteDescriptorSet descriptorWriteImg{};
    descriptorWriteImg.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteImg.pNext = nullptr;
    descriptorWriteImg.descriptorCount = 1;
    descriptorWriteImg.pBufferInfo = nullptr;
    descriptorWriteImg.descriptorType = descriptorType;
    descriptorWriteImg.dstSet = set;
    descriptorWriteImg.dstBinding = binding;
    descriptorWriteImg.dstArrayElement = 0;
    descriptorWriteImg.pImageInfo = &imageInfo;
    descriptorWriteImg.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(imageView.getApp()->logicalDevice.get(), 1U, &descriptorWriteImg, 0U, nullptr);
}

void updateDescriptor(AppBuffer buffer, VkDescriptorSet set, uint32_t size, uint32_t binding, VkDescriptorType descriptorType)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer.get();
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWriteBuffer{};
    descriptorWriteBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteBuffer.pNext = nullptr;
    descriptorWriteBuffer.descriptorCount = 1;
    descriptorWriteBuffer.pBufferInfo = &bufferInfo;
    descriptorWriteBuffer.descriptorType = descriptorType;
    descriptorWriteBuffer.dstSet = set;
    descriptorWriteBuffer.dstBinding = binding;
    descriptorWriteBuffer.dstArrayElement = 0;
    descriptorWriteBuffer.pImageInfo = nullptr;
    descriptorWriteBuffer.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(buffer.getApp()->logicalDevice.get(), 1U, &descriptorWriteBuffer, 0U, nullptr);
}


// void destroySwapchain(AppSwapchain swapchain)
// {
//     // Destroy the created image views, and delete the images (which will themselves be destroyed when destroying the swapchain)
//     for (uint32_t index = 0 ; index < swapchain.swapchainImages.size() ; index++){
//         vkDestroyImageView(app->logicalDevice.get(), swapchain.swapchainImageViews[index].get(), nullptr);
//         imageViews.erase(swapchain.swapchainImageViews[index].getRef());
//         images.erase(swapchain.swapchainImages[index].getRef());

//         vkDestroyFramebuffer(app->logicalDevice.get(), swapchain.framebuffers[index].get(), nullptr);
//         frameBuffers.erase(swapchain.framebuffers[index].getRef());

//     }

//     // Destroy the swapchain Vulkan resource
//     vkDestroySwapchainKHR(app->logicalDevice.get(), swapchain.get(), nullptr);

//     // Remove this swapchain from the list
//     swapchains.erase(swapchain.getRef());
// }


static VkImageLayout getImageLayoutFromTemplate(AppImageTemplate t) {
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE : {
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        case AppImageTemplate::STAGING_IMAGE_TEXTURE : {
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE : {
            return VK_IMAGE_LAYOUT_GENERAL;
        }
        default:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}
