#include "resource-utilities.h"
#include "vulkan-app.h"
#include "turbojpeg.h"
#include "GLFW/glfw3.h"
#include "file-utilities.h"

void loadJPEGImage(const char *path, AppImage image, VkCommandBuffer commandBuffer, uint32_t targetLayer, VulkanApp *app)
{
    VkDevice device = app->logicalDevice.get();
    
    // Create a dummy staging image to fetch image requirements from (particularly, the alignment)
    AppImage dummyImage = createImage(app, 1U, 1U, AppImageTemplate::STAGING_IMAGE_TEXTURE);
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
    stagingImage = createImage(app, width, height, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), stagingImage.get(), &stagingImageMemoryRequirements);

    stagingImageMemory = allocateImageMemory(stagingImage);

    bindImageToMemory(stagingImage, stagingImageMemory);

    void* mappedImageMemory = nullptr;

    // Copy the image into the staging image resource
    copyDataToStagingMemory(stagingImageMemory, jpegImage.data(), stagingImageMemoryRequirements.size);

    // Push the staging image contents to the device-local image
    pushStagingImage(stagingImage, image, commandBuffer, targetLayer);

    // Transition the image to be used as a shader resource
    transitionImageLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer, targetLayer);

    // Destroy the staging image
    destroyImage(stagingImage);
}

void renderCubeMap(AppImage imageArray)
{
    // 
}



AppImage addExistingImage(VulkanApp* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers, VkImageLayout layout, VkImage image)
{
    AppImage returnImage{};
    returnImage.imageCreationTemplate = appImageTemplate;
    returnImage.layerCount = layers;
    returnImage.width = width;
    returnImage.imageLayout = layout;
    returnImage.height = height;
    returnImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    returnImage.setRef(app->resources.images.create(image));

    return returnImage;
}

AppImageView createImageView(VulkanApp* app, AppImage &image, uint32_t layerCount, uint32_t baseLayer)
{

    VkImageViewCreateInfo createInfo{ getImageViewCreateInfoFromTemplate(image.imageCreationTemplate) };
    createInfo.image = image.get();
    createInfo.subresourceRange.layerCount = layerCount;
    createInfo.subresourceRange.baseArrayLayer = baseLayer;

    // If this view will encompass more than one layer, give it the array type
    if (layerCount > 1) {
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }

    VkImageView imageView;
    THROW(vkCreateImageView(app->logicalDevice.get(), &createInfo, nullptr, &imageView), "Failed to create image view");

    AppImageView returnView(app);
    returnView.imageCreationTemplate = image.imageCreationTemplate;
    returnView.setRef(app->resources.imageViews.create(imageView));

    return returnView;
}

AppImageBundle createImageAll(VulkanApp* app, uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{

    AppImageBundle bundle {};
    bundle.image = createImage(app, width, height, appImageTemplate, layerCount);
    bundle.deviceMemory = allocateImageMemory(app, bundle.image);
    bindImageToMemory(app, bundle.image, bundle.deviceMemory);
    bundle.imageView = createImageView(app,bundle.image, layerCount);
    return bundle;
}


void pushStagingImage(AppImage &stagingImage, AppImage &deviceLocalImage, VkCommandBuffer commandBuffer, uint32_t deviceLocalLayer)
{
    transitionImageLayout(stagingImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
    transitionImageLayout(deviceLocalImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer, deviceLocalLayer);

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
    vkMapMemory(app->logicalDevice.get(), stagingMemory.get(), 0U, size, 0U, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(app->logicalDevice.get(), stagingMemory.get());
}


void updateDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, AppSampler sampler)
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
    descriptorWriteImg.descriptorType = getDescriptorTypeFromTemplate(itemTemplate);
    descriptorWriteImg.dstSet = set;
    descriptorWriteImg.dstBinding = binding;
    descriptorWriteImg.dstArrayElement = 0;
    descriptorWriteImg.pImageInfo = &imageInfo;
    descriptorWriteImg.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(imageView.getApp()->logicalDevice.get(), 1U, &descriptorWriteImg, 0U, nullptr);
}

void updateDescriptor(VkBuffer buffer, VkDescriptorSet set, uint32_t size, uint32_t binding, AppDescriptorItemTemplate itemTemplate)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWriteBuffer{};
    descriptorWriteBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteBuffer.pNext = nullptr;
    descriptorWriteBuffer.descriptorCount = 1;
    descriptorWriteBuffer.pBufferInfo = &bufferInfo;
    descriptorWriteBuffer.descriptorType = getDescriptorTypeFromTemplate(itemTemplate);
    descriptorWriteBuffer.dstSet = set;
    descriptorWriteBuffer.dstBinding = binding;
    descriptorWriteBuffer.dstArrayElement = 0;
    descriptorWriteBuffer.pImageInfo = nullptr;
    descriptorWriteBuffer.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(app->logicalDevice.get(), 1U, &descriptorWriteBuffer, 0U, nullptr);
}


void destroySwapchain(AppSwapchain swapchain)
{
    // Destroy the created image views, and delete the images (which will themselves be destroyed when destroying the swapchain)
    for (uint32_t index = 0 ; index < swapchain.swapchainImages.size() ; index++){
        vkDestroyImageView(app->logicalDevice.get(), swapchain.swapchainImageViews[index].get(), nullptr);
        imageViews.erase(swapchain.swapchainImageViews[index].getRef());
        images.erase(swapchain.swapchainImages[index].getRef());

        vkDestroyFramebuffer(app->logicalDevice.get(), swapchain.framebuffers[index].get(), nullptr);
        frameBuffers.erase(swapchain.framebuffers[index].getRef());

    }

    // Destroy the swapchain Vulkan resource
    vkDestroySwapchainKHR(app->logicalDevice.get(), swapchain.get(), nullptr);

    // Remove this swapchain from the list
    swapchains.erase(swapchain.getRef());
}


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

static VkDescriptorType getDescriptorTypeFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE : {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

static VkShaderStageFlags getDescriptorStageFlagFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER : {
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
    }
}