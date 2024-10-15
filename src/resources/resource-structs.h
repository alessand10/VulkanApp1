#pragma once

#include <vulkan/vulkan.hpp>
#include <list>
#include "memory-list-tree.h"
#include "vulkan-app.h"
#include "device-memory-resource.h"


template <typename T>
struct StagingBufferManager {
    MemoryListTree memoryListTree;
    AppDeviceMemory* bufferMemory;
    public:

    void init(AppDeviceMemory* bufferMemory) {
        this->bufferMemory = bufferMemory;
        memoryListTree.init(bufferMemory->getSize());
    }

    std::list<MemoryBlockNode>::iterator addElements(std::vector<T> elements) {
        VkDevice device = bufferMemory->getAppBase()->getDevice();
        // Find a free block in memory to store the vertices
        auto memoryBlock = memoryListTree.reserve(elements.size() * sizeof(T));
        
        // Map the staging buffer memory
        void* mappedBufferMemory = nullptr;
        vkMapMemory(device, bufferMemory->get(), memoryBlock->byteOffset, memoryBlock->byteSize, 0, &mappedBufferMemory);

        // Copy the elements to the staging buffer
        memcpy(mappedBufferMemory, elements.data(), elements.size() * sizeof(T));

        // Unmap memory
        vkUnmapMemory(device, bufferMemory->get());

        return memoryBlock;
    }

    void freeMemory(std::list<MemoryBlockNode>::iterator it) {
        memoryListTree.free(it);
    }

};