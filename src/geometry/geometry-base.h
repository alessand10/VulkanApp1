#pragma once 
#include "memory-block-node.h"
#include "glm/glm.hpp"
#include "app-config.h"

class GeometryBase {
    public:

    struct VertexIndices {
        uint32_t positionIndex;
        uint32_t texCoordIndex;
        uint32_t normalIndex;
    };
    
    protected:
    std::list<MemoryBlockNode>::iterator vertexBufferBlock;
    std::list<MemoryBlockNode>::iterator indexBufferBlock;
    
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::vec2> texCoords = {};
    std::vector<VertexIndices> vertexIndices = {};

    std::string shapeName;

    public:
    uint32_t getVertexCount();
    uint32_t getIndexCount();
    uint32_t getVertexOffset();
    uint32_t getIndexOffset();
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> getVertexAndIndexData();
    void setVertexBufferBlock(std::list<MemoryBlockNode>::iterator vertexBufferBlock);
    void setIndexBufferBlock(std::list<MemoryBlockNode>::iterator indexBufferBlock);

    void setShapeName(std::string name);
    void addVertexIndex(uint32_t positionIndex, uint32_t texCoordIndex, uint32_t normalIndex);
    void addPosition(glm::vec3 position);
    std::vector<glm::vec3> getPositions();
    void addNormal(glm::vec3 normal);
    std::vector<glm::vec3> getNormals();
    void addTexCoord(glm::vec2 texCoord);
    std::vector<glm::vec2> getTexCoords();
};