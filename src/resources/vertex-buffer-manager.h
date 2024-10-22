#pragma once
#include "buffer-storage-manager.h"
#include "app-config.h"
#include "geometry-base.h"

class VIBufferManager {
    BufferStorageManager<Vertex> vbStorageManager;
    BufferStorageManager<uint32_t> ibStorageManager;
    AppBufferBundle stagingVertexBuffer;
    AppBufferBundle stagingIndexBuffer;
    AppBufferBundle vertexBuffer;
    AppBufferBundle indexBuffer;
    
    public:
    void init(AppBufferBundle vertexBuffer, AppBufferBundle indexBuffer, AppBufferBundle stagingVertexBuffer, AppBufferBundle stagingIndexBuffer) {
        vbStorageManager.init(&vertexBuffer.deviceMemory);
        ibStorageManager.init(&indexBuffer.deviceMemory);
        this->stagingVertexBuffer = stagingVertexBuffer;
        this->stagingIndexBuffer = stagingIndexBuffer;
        this->vertexBuffer = vertexBuffer;
        this->indexBuffer = indexBuffer;
    }

    void addGeometry(GeometryBase* geometry, VkCommandBuffer commandBuffer) {
        std::pair<std::vector<Vertex>, std::vector<uint32_t>> vertexIndexData = geometry->getVertexAndIndexData();
        geometry->setVertexBufferBlock(vbStorageManager.reserveMemory(vertexIndexData.first));
        geometry->setIndexBufferBlock(ibStorageManager.reserveMemory(vertexIndexData.second));

        // Copy the data to the staging buffers
        copyDataToStagingMemory(stagingVertexBuffer.deviceMemory, vertexIndexData.first.data(), vertexIndexData.first.size() * sizeof(Vertex));
        copyDataToStagingMemory(stagingIndexBuffer.deviceMemory, vertexIndexData.second.data(), vertexIndexData.second.size() * sizeof(uint32_t));

        // Copy the data to the vertex and index buffers
        AppBuffer::copyBuffer(stagingVertexBuffer.buffer, vertexBuffer.buffer, commandBuffer, geometry->getVertexCount() * sizeof(Vertex), 0U, geometry->getVertexOffset() * sizeof(Vertex));
        AppBuffer::copyBuffer(stagingIndexBuffer.buffer, indexBuffer.buffer, commandBuffer, geometry->getIndexCount() * sizeof(uint32_t), 0U, geometry->getIndexOffset() * sizeof(uint32_t));
    }

    void freeMemory(std::list<MemoryBlockNode>::iterator it) {
        vbStorageManager.freeMemory(it);
    }
};