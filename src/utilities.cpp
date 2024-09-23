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

    uint32_t width = 0, height = 0;

    int pixelFormat = TJPF_RGBA;
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    std::vector<char> jpegImage = readJPEG(path, imageMemoryRequirements.alignment, &width, &height);

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
    memcpy(mappedImageMemory, jpegImage.data(), stagingImageMemoryRequirements.size);
    vkUnmapMemory(device, stagingImageMemory.get());

    app->resourceManager.pushStagingImage(stagingImage, returnImageBundle.image);
    app->resourceManager.transitionImageLayout(returnImageBundle.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Destroy the staging image
    app->resourceManager.destroyImage(stagingImage);

    return returnImageBundle;
}

std::vector<char> readFile(const std::string filename)
{
    std::vector<char> fileData;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    bool open = file.is_open();
    if (open) {
        size_t fileSize = (size_t)file.tellg();
        fileData.resize(fileSize);
        file.seekg(0);
        file.read(fileData.data(), fileSize);
        return fileData;
    }
}

std::vector<char> readJPEG(const std::string filename, int alignment, uint32_t* width, uint32_t* height)
{
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    
    std::vector<char> jpegFile = readFile(filename);
    const unsigned char* jpegData = static_cast<const unsigned char*>(static_cast<void*>(jpegFile.data()));

    int pixelFormat = TJPF_RGBA;

    // Retrieve image parameters
    tj3DecompressHeader(turboJpegHandle, jpegData, jpegFile.size());

    // Returns the image height and width in pixels, respectively
    int imageHeight = tj3Get(turboJpegHandle, TJPARAM_JPEGHEIGHT);
    int imageWidth = tj3Get(turboJpegHandle, TJPARAM_JPEGWIDTH);
    *width = static_cast<uint32_t>(imageWidth);
    *height = static_cast<uint32_t>(imageHeight);


    // Returns the number of bits used per sample
    int precision = tj3Get(turboJpegHandle, TJPARAM_PRECISION);

    int samplesPerPixel = tjPixelSize[pixelFormat];

    // Determine the number of padded pixels needed per row to satisfy memory alignment requirements
    int paddingPixelsPerRow = alignment == 0 ? 0 : (imageWidth % alignment);

    // Determine the number of samples per row, aka pitch
    int pitch = (imageWidth + paddingPixelsPerRow) * samplesPerPixel;

    // Create a vector big enough for the image
    int outputBufferSize = imageHeight * pitch;
    std::vector<char> outputImage(outputBufferSize);

    tj3Decompress8(turboJpegHandle, jpegData, jpegFile.size(), static_cast<unsigned char*>(static_cast<void*>(outputImage.data())), pitch, pixelFormat);

    return outputImage;
}
