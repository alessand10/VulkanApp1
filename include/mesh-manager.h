#pragma once
#include <map>
#include <vector>
#include <stdint.h>
#include <vertex.h>
#include "mesh.h"
/**
 * @class MeshManager
 * @brief Manages 3D mesh storage
 * 
 * The MeshManager class is the interface through which the app user 
 * adds, removes and configures meshes. 
 */
class MeshManager {
    public:
    struct MeshInsertion {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t vertexCount;
        uint32_t indexCount;
    };

    private:
    class VulkanApp* app;

    std::vector<Mesh> meshes;
    std::map<uint32_t, MeshInsertion> insertedMeshData = {};
    std::vector<Vertex> vertexData;
    std::vector<uint32_t> indexData;
    
    void insertMesh(Mesh* mesh);

    public:
    MeshManager();
    void init(class VulkanApp* app) {this->app = app;};
    void importMeshFromOBJ(const char* path);
    Vertex* getVertexData() {return vertexData.data();}
    uint32_t getVertexDataSize() {return sizeof(Vertex) * vertexData.size();};
    u_int32_t* getIndexData() {return indexData.data();}
    uint32_t getIndexDataSize() {return sizeof(uint32_t) * indexData.size();};

    uint32_t getMeshCount();
    MeshInsertion getMeshInsertionAtIndex(uint32_t index);
};