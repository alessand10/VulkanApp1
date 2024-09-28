#pragma once
#include <vector>
#include <string>
#include "turbojpeg.h"
#include "vulkan-app.h"

std::vector<std::string> splitString(std::string& s, const std::string& delimiter);

AppImageBundle createAndLoadVulkanImage(const char* path, class VulkanApp* app);

void loadJPEGImage(const char* path, AppImage image, uint32_t targetLayer, class VulkanApp* app);

// Creates an image with 6 array layers, ready to be rendered and used as a cubemap
//AppImage createCubeMap(float planeWidth, float planeHeight, uint32_t imageWidth, uint32_t imageHeight)

// Renders to a cubemap using specified parameters
// void renderCubeMap(AppImage &image, float3 worldLocation, float4 upVector, float4 forwardVector)

// Creates a 2D tangent map, which contains local/object space tangent vectors for the model.
// The tangent vectors point in the horizontal direction (U+) of the texture map in object space
// AppImage createTangentMap()

std::vector<char> readFile(const std::string filename);

/**
 * @brief Reads in and decompresses a JPEG image file
 * 
 * @param filename The JPEG image file
 * @param alignment The number of bytes to align to, set to -1 to have the application compute this value
 * @return std::vector<char> 
 */
std::vector<char> readJPEG(const std::string filename, int alignment, uint32_t* width = nullptr, uint32_t* height = nullptr);

