#pragma once
#include <vector>
#include <string>
#include "turbojpeg.h"
#include "vulkan/vulkan.hpp"

std::vector<std::string> splitString(std::string& s, const std::string& delimiter);

void createAndLoadVulkanImage(const char* path, class VulkanApp* app, class AppImage2D &image);

// Creates an image with 6 array layers, ready to be rendered and used as a cubemap
//AppImage2D createCubeMap(float planeWidth, float planeHeight, uint32_t imageWidth, uint32_t imageHeight)

// Renders to a cubemap using specified parameters
// void renderCubeMap(AppImage2D &image, float3 worldLocation, float4 upVector, float4 forwardVector)

// Creates a 2D tangent map, which contains local/object space tangent vectors for the model.
// The tangent vectors point in the horizontal direction (U+) of the texture map in object space
// AppImage2D createTangentMap()


