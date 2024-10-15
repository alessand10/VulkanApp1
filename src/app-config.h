#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"

static uint32_t supportedVertexCount = 200U;
static uint32_t supportedIndexCount = 200U;

struct FragmentPushConst {
    uint32_t textureIndex = 0u;
};

struct VSUniformBuffer {
    glm::mat4 worldMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
};

struct Face {
    std::vector<uint32_t> posIndices = {};
    std::vector<uint32_t> texCoordIndices = {};
    std::vector<uint32_t> normalIndices = {};
    std::vector<Face> triangulate() {
        return std::vector<Face> {Face{posIndices, texCoordIndices, normalIndices}};
    }
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        return {
            // Position
            {
                0U, // Location, the shader input location (layout(location = 0) in vec3 position)
                0U, // Binding, the binding number of the vertex buffer from which this data is coming,
                VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the position,
                0U // Offset, this is the first attribute so use an offset of 0
            },

            // Normal
            {
                1U, // Location, the shader input location (layout(location = 1) in vec3 normal)
                0U, // Binding, the binding number of the vertex buffer from which this data is coming,
                VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the normal,
                12U // Offset, we use 12 bytes since position is made up of 3 x 4-byte values
            },
            // Tangent
            {
                2U, // Location, the shader input location (layout(location = 1) in vec3 normal)
                0U, // Binding, the binding number of the vertex buffer from which this data is coming,
                VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the normal,
                24U // Offset, we use 12 bytes since position is made up of 3 x 4-byte values
            },

            // Texcoord
            {
                3U, // Location, the shader input location (layout(location = 2) in vec2 texCoord)
                0U, // Binding, the binding number of the vertex buffer from which this data is coming,
                VK_FORMAT_R32G32_SFLOAT, // Format, we use a Float2 for the texture coordinate,
                36U // Offset, we use 24 bytes since position & normal take up 6 x 4-byte values
            }
        };
    }
};