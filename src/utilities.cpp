#include "utilities.h"
#include "vulkan-app.h"
#include <fstream>

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
AppImageBundle createAndLoadVulkanImage(const char* path, VulkanApp* app) {
    VkDevice device = app->logicalDevice.get();

    AppImageBundle returnImageBundle {};
    
    AppImage dummyImage = app->resourceManager.createImage(1U, 1U, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    VkMemoryRequirements imageMemoryRequirements{};
    vkGetImageMemoryRequirements(app->logicalDevice.get(), dummyImage.get(), &imageMemoryRequirements);

    int width = 0, height = 0;

    int pixelFormat = TJPF_RGBA;
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    unsigned char* loadedImage = tj3LoadImage8(turboJpegHandle, path, &width, imageMemoryRequirements.alignment, &height, &pixelFormat);

    AppImage stagingImage;
    AppDeviceMemory stagingImageMemory;

    returnImageBundle.image = app->resourceManager.createImage(width, height, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE);
    stagingImage = app->resourceManager.createImage(width, height, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), stagingImage.get(), &stagingImageMemoryRequirements);

    returnImageBundle.deviceMemory = app->resourceManager.allocateImageMemory(returnImageBundle.image);
    stagingImageMemory = app->resourceManager.allocateImageMemory(stagingImage);

    app->resourceManager.bindImageToMemory(returnImageBundle.image, returnImageBundle.deviceMemory);
    app->resourceManager.bindImageToMemory(stagingImage, stagingImageMemory);

    returnImageBundle.imageView = app->resourceManager.createImageView(returnImageBundle.image);

    void* mappedImageMemory = nullptr;

    // The image memory is then copied from the CPU to the staging image
    THROW(vkMapMemory(device, stagingImageMemory.get(), 0U, stagingImageMemoryRequirements.size, 0U, &mappedImageMemory), "Failed to map staging image memory");
    memcpy(mappedImageMemory, loadedImage, stagingImageMemoryRequirements.size);
    vkUnmapMemory(device, stagingImageMemory.get());

    app->resourceManager.pushStagingImage(stagingImage, returnImageBundle.image);
    app->resourceManager.transitionImageLayout(returnImageBundle.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Destroy the staging image
    app->resourceManager.destroyImage(stagingImage);

    tj3Free(loadedImage);

    return returnImageBundle;
}

std::vector<char> readFile(const std::string filename)
{
    std::vector<char> fileData;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    bool open = file.is_open();
    size_t fileSize = (size_t)file.tellg();
    fileData.resize(fileSize);
    file.seekg(0);
    file.read(fileData.data(), fileSize);
    return fileData;
}
