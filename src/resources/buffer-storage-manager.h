#pragma once
#include "memory-list-tree.h"
#include "device-memory-resource.h"
/**
 * A general buffer storage manager that can be used to reserve and free space within a buffer.
 * This manager does not actually perform memory operations, but rather keeps track of the memory
 */
template <typename T>
struct BufferStorageManager {
    MemoryListTree memoryListTree;
    AppDeviceMemory* bufferMemory;
    public:

    void init(AppDeviceMemory* bufferMemory) {
        this->bufferMemory = bufferMemory;
        memoryListTree.init(bufferMemory->getSize());
    }

    std::list<MemoryBlockNode>::iterator reserveMemory(std::vector<T> elements) {
        // Find a free block in memory to store the vertices
        auto memoryBlock = memoryListTree.reserve(elements.size() * sizeof(T));

        return memoryBlock;
    }

    void freeMemory(std::list<MemoryBlockNode>::iterator it) {
        memoryListTree.free(it);
    }

};