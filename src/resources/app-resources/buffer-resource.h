#pragma once
#include "app-resource.h"


enum class AppBufferTemplate {
    UNIFORM_BUFFER,
    VERTEX_BUFFER_DEVICE,
    INDEX_BUFFER_DEVICE,
    VERTEX_BUFFER_STAGING,
    INDEX_BUFFER_STAGING
};

class AppBuffer : public AppResource<VkBuffer> {
    protected:
    AppBufferTemplate appBufferTemplate = AppBufferTemplate::UNIFORM_BUFFER;
    size_t size;
    public:
    void init(class VulkanApp* app, size_t size, AppBufferTemplate appBufferTemplate);
    AppBufferTemplate getTemplate() { return appBufferTemplate; }

    void bindToMemory(class AppDeviceMemory *bufferMemory);

    static void copyBuffer(AppBuffer &src, AppBuffer &dst, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

    void destroy();
};