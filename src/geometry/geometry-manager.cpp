#include "geometry-manager.h"
#include <fstream>
#include <iostream>
#include <string>
#include "geometry-utilities.h"
#include <sstream>
#include "string-utilities.h"

void GeometryManager::insertMesh(Mesh *mesh, VkCommandBuffer commandBuffer)
{
    uint32_t vertexOffset = vertexData.size();
    uint32_t indexOffset = indexData.size();
    uint32_t vertexCount = 0U;
    uint32_t indexCount = 0U;
    for (Face face : mesh->faces) {
        uint32_t iProcOrder[] = {0, 2, 1};
        glm::vec3 tangent = computeTangentBitangent(
            mesh->positions[face.posIndices[iProcOrder[0]]],
            mesh->texCoords[face.texCoordIndices[iProcOrder[0]]],
            mesh->positions[face.posIndices[iProcOrder[1]]],
            mesh->texCoords[face.texCoordIndices[iProcOrder[1]]],
            mesh->positions[face.posIndices[iProcOrder[2]]],
            mesh->texCoords[face.texCoordIndices[iProcOrder[2]]]
        )[0];

        for (uint32_t i = 0u ; i < 3 ; i++){
            vertexData.push_back({
                mesh->positions[face.posIndices[iProcOrder[i]]],
                mesh->normals[face.normalIndices[iProcOrder[i]]],
                tangent,
                mesh->texCoords[face.texCoordIndices[iProcOrder[i]]]
            });
            indexData.push_back(vertexCount);
            vertexCount++;
            indexCount++;
        }
    }

    // app->resourceManager.copyDataToStagingMemory(stagingVertexBuffer.deviceMemory, getVertexData(), getVertexDataSize());
    // app->resourceManager.pushStagingBuffer(stagingVertexBuffer.buffer, deviceVertexBuffer.buffer, commandBuffer);

    // app->resourceManager.copyDataToStagingMemory(stagingIndexBuffer.deviceMemory, getIndexData(), getIndexDataSize());
    // app->resourceManager.pushStagingBuffer(stagingIndexBuffer.buffer, deviceIndexBuffer.buffer, commandBuffer);

    insertedMeshData[meshes.size() - 1] = MeshInsertion{vertexOffset, indexOffset, vertexCount, indexCount};
}

GeometryManager::GeometryManager()
{
    vertexData = {};
    indexData = {};
}

// void GeometryManager::setVertexBuffers(AppBufferBundle stagingVertexBuffer, AppBufferBundle deviceVertexBuffer)
// {
//     this->stagingVertexBuffer = stagingVertexBuffer;
//     this->deviceVertexBuffer = deviceVertexBuffer;
// }

// void GeometryManager::setIndexBuffers(AppBufferBundle stagingIndexBuffer, AppBufferBundle deviceIndexBuffer)
// {
//     this->stagingIndexBuffer = stagingIndexBuffer;
//     this->deviceIndexBuffer = deviceIndexBuffer;
// }

void GeometryManager::importMeshFromOBJ(const char *path, VkCommandBuffer commandBuffer)
{
    meshes.push_back(Mesh{});
    Mesh* mesh = &(meshes[meshes.size() - 1]);
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string lineType = "";
            iss >> lineType;
            // Line is a vertex, extract xyz coordinate
            if (lineType == "v") {
                float x,y,z;
                if (!(iss >> x >> y >> z)) std::cout << "Corrupt vertex entry";
                mesh->positions.push_back(glm::vec3{x,y,z});
            }
            else if (lineType == "vt") {
                float u,v;
                if (!(iss >> u >> v)) std::cout << "Corrupt texcoord entry";
                mesh->texCoords.push_back(glm::vec2{u, v});
            }
            else if (lineType == "vn") {
                float x,y,z;
                if (!(iss >> x >> y >> z)) std::cout << "Corrupt normal entry";
                mesh->normals.push_back(glm::vec3{x, y, z});
            }
            else if (lineType == "f") {
                std::string indexGroup;
                Face face;
                while (iss >> indexGroup) {
                    std::vector<std::string> indices = splitString(indexGroup, "/");
                    for (uint32_t index = 0u ; index < indices.size() ; index++) {
                        uint32_t indexUL = std::stoul(indices[index]) - 1U;
                        if (index == 0u) face.posIndices.push_back(indexUL);
                        else if (index == 1u) face.texCoordIndices.push_back(indexUL);
                        else if (index == 2u) face.normalIndices.push_back(indexUL);
                    }
                }
                mesh->faces.push_back(face);
            }
        }
    }
    else {
        std::cout << "Failed to open obj file, check that the path is correct from the build directory and that the file \
        exists." << std::endl;
    }
    insertMesh(mesh, commandBuffer);
}

uint32_t GeometryManager::getMeshCount()
{
    return insertedMeshData.size();
}

GeometryManager::MeshInsertion GeometryManager::getMeshInsertionAtIndex(uint32_t index)
{
    return insertedMeshData[index];
}
