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

    MemoryListTree vertexBufferMlt;
    MemoryListTree indexBufferMlt;

    std::vector<Mesh> meshes;

    bool buffersInitialized = false;
    
    void insertMesh(Mesh* mesh, VkCommandBuffer commandBuffer);

    public:
    void init(class VulkanApp* app, uint32_t vertexCount, uint32_t indexCount) {
        this->app = app;
        vertexBufferMlt.init(vertexCount);
        indexBufferMlt.init(indexCount);
    };

    Mesh* getMesh(uint32_t index) {
        return &meshes[index];
    }

    int importOBJ(const char* path, VkCommandBuffer commandBuffer);

};