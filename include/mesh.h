#pragma once
#include <vector>
#include <cstdint>
#include "vertex.h"

/**
 * 
 */ 
struct Mesh {

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<Face> faces;

    public:
    Mesh();
    void importOBJ(const char* path);
    uint32_t getIndexCount();
    uint32_t getVertexCount();

    
};