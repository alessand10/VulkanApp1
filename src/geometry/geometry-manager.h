#pragma once
#include <map>
#include <vector>
#include <stdint.h>
#include "vulkan/vulkan.hpp"
#include "mesh.h"
#include "memory-list-tree.h"

/**
 * @class MeshManager
 * @brief Manages 3D mesh storage
 * 
 * The MeshManager class is the interface through which the app user 
 * adds, removes and configures meshes. 
 */
class GeometryManager {
    public:

    private:
    class VulkanApp* app;

    std::vector<Mesh> meshes;

    bool buffersInitialized = false;


    public:
    void init(class VulkanApp* app) {
        this->app = app;
    };

    Mesh* getMesh(uint32_t index) {
        return &meshes[index];
    }

    int importOBJ(const char* path, VkCommandBuffer commandBuffer);

};