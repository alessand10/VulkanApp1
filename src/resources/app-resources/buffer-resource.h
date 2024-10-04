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
    void init(VulkanApp* app, size_t size, AppBufferTemplate appBufferTemplate);
    AppBufferTemplate getTemplate() { return appBufferTemplate; }

    static void copyBuffer(AppBuffer &src, AppBuffer &dst, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

    void destroy() { getApp()->resources.buffers.destroy(getIterator(), getApp()->logicalDevice.get()); }
};