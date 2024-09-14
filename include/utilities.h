#pragma once
#include <vector>
#include <string>
#include "turbojpeg.h"
#include "vulkan-app.h"

std::vector<std::string> splitString(std::string& s, const std::string& delimiter);

AppImageBundle createAndLoadVulkanImage(const char* path, class VulkanApp* app);

// Creates an image with 6 array layers, ready to be rendered and used as a cubemap
//AppImage createCubeMap(float planeWidth, float planeHeight, uint32_t imageWidth, uint32_t imageHeight)

// Renders to a cubemap using specified parameters
// void renderCubeMap(AppImage &image, float3 worldLocation, float4 upVector, float4 forwardVector)

// Creates a 2D tangent map, which contains local/object space tangent vectors for the model.
// The tangent vectors point in the horizontal direction (U+) of the texture map in object space
// AppImage createTangentMap()

std::vector<char> readFile(const std::string filename);

