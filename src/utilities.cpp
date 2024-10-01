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
AppImageBundle createAndLoadVulkanImage(const char* path, VkCommandBuffer commandBuffer, VulkanApp* app) {
    VkDevice device = app->logicalDevice.get();

    AppImageBundle returnImageBundle {};
    
    // Create a dummy image to fetch image requirements from (particularly, the alignment)
    AppImage dummyImage = app->resourceManager.createImage(1U, 1U, AppImageTemplate::STAGING_IMAGE_TEXTURE);
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

    // Create an image bundle to be used for the return image
    returnImageBundle.image = app->resourceManager.createImage(width, height, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE);

    // Initialize the actual staging image
    stagingImage = app->resourceManager.createImage(width, height, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), stagingImage.get(), &stagingImageMemoryRequirements);

    returnImageBundle.deviceMemory = app->resourceManager.allocateImageMemory(returnImageBundle.image);
    stagingImageMemory = app->resourceManager.allocateImageMemory(stagingImage);

    app->resourceManager.bindImageToMemory(returnImageBundle.image, returnImageBundle.deviceMemory);
    app->resourceManager.bindImageToMemory(stagingImage, stagingImageMemory);

    returnImageBundle.imageView = app->resourceManager.createImageView(returnImageBundle.image);

    void* mappedImageMemory = nullptr;

    // Copy the image into the staging image resource
    app->resourceManager.copyDataToStagingMemory(stagingImageMemory, jpegImage.data(), stagingImageMemoryRequirements.size);

    // Push the staging image contents to the device-local image
    app->resourceManager.pushStagingImage(stagingImage, returnImageBundle.image, commandBuffer);

    // Transition the image to be used as a shader resource
    app->resourceManager.transitionImageLayout(returnImageBundle.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);

    // Destroy the staging image
    app->resourceManager.destroyImage(stagingImage);

    return returnImageBundle;
}

void loadJPEGImage(const char *path, AppImage image, VkCommandBuffer commandBuffer, uint32_t targetLayer, VulkanApp *app)
{
    VkDevice device = app->logicalDevice.get();
    
    // Create a dummy staging image to fetch image requirements from (particularly, the alignment)
    AppImage dummyImage = app->resourceManager.createImage(1U, 1U, AppImageTemplate::STAGING_IMAGE_TEXTURE);
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
    stagingImage = app->resourceManager.createImage(width, height, AppImageTemplate::STAGING_IMAGE_TEXTURE);
    
    VkMemoryRequirements stagingImageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice.get(), stagingImage.get(), &stagingImageMemoryRequirements);

    stagingImageMemory = app->resourceManager.allocateImageMemory(stagingImage);

    app->resourceManager.bindImageToMemory(stagingImage, stagingImageMemory);

    void* mappedImageMemory = nullptr;

    // Copy the image into the staging image resource
    app->resourceManager.copyDataToStagingMemory(stagingImageMemory, jpegImage.data(), stagingImageMemoryRequirements.size);

    // Push the staging image contents to the device-local image
    app->resourceManager.pushStagingImage(stagingImage, image, commandBuffer, targetLayer);

    // Transition the image to be used as a shader resource
    app->resourceManager.transitionImageLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer, targetLayer);

    // Destroy the staging image
    app->resourceManager.destroyImage(stagingImage);
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
    if (jpegFile.size() == 0) throw std::runtime_error("Failed to open jpeg image");
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

std::vector<glm::vec3> computeTangentBitangent(glm::vec3 p1, glm::vec2 p1UV, glm::vec3 p2, glm::vec2 p2UV, glm::vec3 p3, glm::vec2 p3UV)
{
    /**
     * Approach: For a triangle formed from the points p1, p2, and p3, we can compute the tangent vector
     * T and bi-tangent vector B by recognizing that the edge vectors from p1 to p2 (p2 - p1) and p1 to p3
     * (p3 - p1) can be formed by a linear combination of the tangent and bitangent vectors. 
     * The +U direction in 2D space corresponds to the tangent vector, and the +V direction in 2D space corresponds
     * to the bitangent vector. 
     * 
     * Edge 1 has UV vectors euv1 = (u1, v1) and edge 2 has UV vectors euv2 = (u2, v2). If we apply this to the
     * equivalent 3D vectors, we get that Edge 1 = u1 * T + v1 * B and edge 2 = u2 * T + v2 * B.
     * 
     * We compute tangent vectors per triangle
     */

    // Compute the uv differences in uv space
    glm::vec2 uv1 = p2UV - p1UV, uv2 = p3UV - p1UV;

    glm::vec3 edge1 = p2 - p1, edge2 = p3 - p1;

    // compute the inverse of the uv difference matrix

    float mDet = (1.f / (uv1.x * uv2.y - uv1.y * uv2.x));

    glm::mat2 mAdjoint = {
        {uv2.y, -uv1.y},
        {-uv2.x, uv1.x}
    };

    glm::mat2 mInv = mDet * mAdjoint;

    glm::vec3 tangent = {mInv[0][0] * edge1 + mInv[0][1] * edge2};
    tangent = glm::normalize(tangent);
    glm::vec3 bitangent = {mInv[1][0] * edge1 + mInv[1][1] * edge2};
    bitangent = glm::normalize(bitangent);

    return std::vector<glm::vec3> {tangent, bitangent};
}
