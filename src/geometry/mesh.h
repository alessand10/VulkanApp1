#pragma once
#include <vector>
#include <cstdint>
#include "app-config.h"
#include "tiny_obj_loader.h"
#include "memory-block-node.h"
#include <list>

/**
 * 
 */ 
struct Mesh {
    std::list<MemoryBlockNode>::iterator vertexBufferBlock;
    std::list<MemoryBlockNode>::iterator indexBufferBlock;

    public:
    tinyobj::attrib_t attrib;
    tinyobj::shape_t shape;

    Mesh();
    
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> getVertexAndIndexData();
};