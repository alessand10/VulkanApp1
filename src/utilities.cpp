#include "utilities.h"
#include "vulkan-app.h"

std::vector<std::string> splitString(std::string &s, const std::string &delimiter){
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0UL, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

/**
 * Loads an image in R8G8B8 format
 */
void createAndLoadVulkanImage(const char* path, VulkanApp* app, AppImage2D &image) {
    VkDevice device = app->logicalDevice;
    
    AppImage2D dummyImage = app->resourceManager.createImage(1U, 1U, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    VkMemoryRequirements imageMemoryRequirements{};
    vkGetImageMemoryRequirements(app->logicalDevice, dummyImage.image, &imageMemoryRequirements);

    int width = 0, height = 0;

    int pixelFormat = TJPF_RGBA;
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    unsigned char* loadedImage = tj3LoadImage8(turboJpegHandle, path, &width, imageMemoryRequirements.alignment, &height, &pixelFormat);

    AppImage2D returnImage;
    AppImage2D stagingImage;

    returnImage = app->resourceManager.createImage(width, height, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE);
    stagingImage = app->resourceManager.createImage(width, height, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice, stagingImage.image, &stagingImageMemoryRequirements);

    app->resourceManager.allocateImageMemory(returnImage);
    app->resourceManager.allocateImageMemory(stagingImage);

    app->resourceManager.bindImageToMemory(returnImage);
    app->resourceManager.bindImageToMemory(stagingImage);

    app->resourceManager.createImageView(returnImage);

    void* mappedImageMemory = nullptr;

    // The image memory is then copied from the CPU to the staging image
    THROW(vkMapMemory(device, stagingImage.memory, 0U, stagingImageMemoryRequirements.size, 0U, &mappedImageMemory), "Failed to map staging image memory");
    memcpy(mappedImageMemory, loadedImage, stagingImageMemoryRequirements.size);
    vkUnmapMemory(device, stagingImage.memory);

    app->resourceManager.pushStagingImage(stagingImage, returnImage);
    app->resourceManager.transitionImageLayout(returnImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    image = returnImage;

    tj3Free(loadedImage);
}
