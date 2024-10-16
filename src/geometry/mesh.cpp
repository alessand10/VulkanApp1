#include "mesh.h"
#include <fstream>
#include <iostream>
#include "vertex-index-search-tree.h"
#include "geometry-utilities.h"

Mesh::Mesh()
{

}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> Mesh::getVertexAndIndexData()
{
    std::vector<Vertex> vertexData;
    std::vector<uint32_t> indexData;

    VertexIndexSearchTree viSearchTree;
    
    // Iterate through the indices provided, 3 indices gives us one triangle
    for (uint32_t i = 0U ; i < shape.mesh.indices.size() ; i++) {
        tinyobj::index_t index = shape.mesh.indices[i];
        int* insertionIndex = viSearchTree.insertVertexIndices(index.vertex_index, index.texcoord_index, index.normal_index);
        if (*insertionIndex == -1) {
            glm::vec3 position = {
                attrib.vertices[index.vertex_index + 0],
                attrib.vertices[index.vertex_index + 1],
                attrib.vertices[index.vertex_index + 2]
            };
            glm::vec3 normal = {
                attrib.normals[index.normal_index + 0],
                attrib.normals[index.normal_index + 1],
                attrib.normals[index.normal_index + 2]
            };
            glm::vec2 texCoord = {
                attrib.texcoords[index.texcoord_index + 0],
                attrib.texcoords[index.texcoord_index + 1]
            };
            Vertex vertex = {
                position,
                normal,
                glm::vec3(0.f), // Will be computed later
                texCoord
            };
            vertexData.push_back(vertex);
            *insertionIndex = vertexData.size() - 1;
        }
        indexData.push_back(*insertionIndex);

        // Compute the tangent vector for the past 3 vertices
        if ((i + 1) % 3 == 0) {
            // Get the indices of the last 3 vertices (index minus 2, index minus 1, index minus 0)
            uint32_t im2 = indexData[i - 2];
            uint32_t im1 = indexData[i - 1];
            uint32_t im0 = indexData[i];

            // Compute the tangent and bitangent vectors
            std::pair<glm::vec3, glm::vec3> tangentBitangent = computeTangentBitangent(
                vertexData[im2].position, vertexData[im2].texCoord,
                vertexData[im1].position, vertexData[im1].texCoord,
                vertexData[im0].position, vertexData[im0].texCoord
            );

            // Set the tangent vectors for the last 3 vertices
            vertexData[im2].tangent = tangentBitangent.first;
            vertexData[im1].tangent = tangentBitangent.first;
            vertexData[im0].tangent = tangentBitangent.first;
        }
    }
    return std::pair<std::vector<Vertex>, std::vector<uint32_t>>(vertexData, indexData);
}
