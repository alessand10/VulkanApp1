#pragma once
#include "glm.hpp"
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Face {
    std::vector<uint32_t> posIndices = {};
    std::vector<uint32_t> texCoordIndices = {};
    std::vector<uint32_t> normalIndices = {};
    std::vector<Face> triangulate() {
        return std::vector<Face> {Face{posIndices, texCoordIndices, normalIndices}};
    }
};