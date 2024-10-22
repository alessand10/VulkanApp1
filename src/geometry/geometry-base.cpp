#include "geometry-base.h"
#include "app-config.h"
#include "vertex-index-search-tree.h"
#include "geometry-utilities.h"

uint32_t GeometryBase::getVertexCount()
{
    return (*vertexBufferBlock).byteSize / sizeof(Vertex);
}

uint32_t GeometryBase::getIndexCount()
{
    return (*indexBufferBlock).byteSize / sizeof(uint32_t);
}

uint32_t GeometryBase::getVertexOffset()
{
    return (*vertexBufferBlock).byteOffset / sizeof(Vertex);
}

uint32_t GeometryBase::getIndexOffset()
{
    return (*indexBufferBlock).byteOffset / sizeof(uint32_t);
}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> GeometryBase::getVertexAndIndexData()
{
    std::vector<Vertex> vertexData;
    std::vector<uint32_t> indexData;

    VertexIndexSearchTree viSearchTree;
    
    // Iterate through the indices provided, 3 indices gives us one triangle (faces are triangulated on import)
    for (uint32_t i = 0U ; i < vertexIndices.size() ; i++) {
        GeometryBase::VertexIndices indices = vertexIndices[i];
        int* insertionIndex = viSearchTree.insertVertexIndices(indices.positionIndex, indices.texCoordIndex, indices.normalIndex);
        if (*insertionIndex == -1) {
            glm::vec3 position = positions[indices.positionIndex];
            glm::vec3 normal = normals[indices.normalIndex];
            glm::vec2 texCoord = texCoords[indices.texCoordIndex];
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

void GeometryBase::setVertexBufferBlock(std::list<MemoryBlockNode>::iterator vertexBufferBlock)
{
    this->vertexBufferBlock = vertexBufferBlock;
}

void GeometryBase::setIndexBufferBlock(std::list<MemoryBlockNode>::iterator indexBufferBlock)
{
    this->indexBufferBlock = indexBufferBlock;
}

void GeometryBase::setShapeName(std::string name)
{
    this->shapeName = name;   
}

void GeometryBase::addVertexIndex(uint32_t positionIndex, uint32_t texCoordIndex, uint32_t normalIndex)
{
    vertexIndices.push_back({positionIndex, texCoordIndex, normalIndex});
}

void GeometryBase::addPosition(glm::vec3 position)
{
    positions.push_back(position);
}

std::vector<glm::vec3> GeometryBase::getPositions()
{
    return positions;
}

void GeometryBase::addNormal(glm::vec3 normal)
{
    normals.push_back(normal);
}

std::vector<glm::vec3> GeometryBase::getNormals()
{
    return normals;
}

void GeometryBase::addTexCoord(glm::vec2 texCoord)
{
    texCoords.push_back(texCoord);
}

std::vector<glm::vec2> GeometryBase::getTexCoords()
{
    return texCoords;
}
