#include "mesh.h"
#include <fstream>
#include <iostream>
#include "vertex-index-search-tree.h"

Mesh::Mesh()
{

}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> Mesh::getVertexAndIndexData()
{
    std::vector<Vertex> vertexData;
    std::vector<uint32_t> indexData;

    VertexIndexSearchTree viSearchTree;
    
    for (uint32_t i = 0U ; i < shape.mesh.indices.size() ; i++) {
        tinyobj::index_t index = shape.mesh.indices[i];
        int* insertionIndex = viSearchTree.insertVertexIndices(index.vertex_index, index.texcoord_index, index.normal_index);
        if (*insertionIndex == -1) {
            glm::vec3 position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            glm::vec3 normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };
            glm::vec2 texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };
            Vertex vertex = {
                position,
                normal,
                glm::vec3(0.0f),
                texCoord
            };
            vertexData.push_back(vertex);
            *insertionIndex = vertexData.size() - 1;
        }
        indexData.push_back(*insertionIndex);
    }
    return std::pair<std::vector<Vertex>, std::vector<uint32_t>>(vertexData, indexData);
}
