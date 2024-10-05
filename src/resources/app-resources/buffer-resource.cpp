#include "vulkan-app.h"
#include "buffer-resource.h"
#include "device-resource.h"


static VkBufferCreateInfo getBufferCreateInfoFromTemplate(AppBufferTemplate t, uint32_t size) { 
    switch(t) {
        case AppBufferTemplate::UNIFORM_BUFFER :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           size,
           0U,
           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           size,
           0U,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           size,
           0U,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           size,
           0U,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           size,
           0U,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        default:
            return {};
    }
};

void AppBuffer::init(VulkanApp *app, size_t size, AppBufferTemplate appBufferTemplate)
{
   this->appBufferTemplate = appBufferTemplate;
   this->size = size;

   // Populate the create info struct
   VkBufferCreateInfo createInfo{getBufferCreateInfoFromTemplate(appBufferTemplate, size)};

   // Attempt to create the buffer
   VkBuffer buffer = VK_NULL_HANDLE;
   THROW(vkCreateBuffer(app->logicalDevice.get(), &createInfo, nullptr, &buffer), "Failed to create buffer"); 

   // Add the buffer to the list of managed buffer, returning an iterator
   AppResource::init(app, app->resources.buffers.create(buffer));
}

void AppBuffer::copyBuffer(AppBuffer &src, AppBuffer &dst, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
   VulkanApp* app = src.getApp();
   VkSubmitInfo submitInfo{};
   submitInfo.pCommandBuffers = &(commandBuffer);
   submitInfo.commandBufferCount = 1U;
   submitInfo.pNext = nullptr;
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.pSignalSemaphores = nullptr;
   submitInfo.signalSemaphoreCount = 0U;
   submitInfo.pWaitSemaphores = nullptr;
   submitInfo.pWaitDstStageMask = nullptr;
   submitInfo.waitSemaphoreCount = 0u;

   VkCommandBufferBeginInfo beginInfo{};
   beginInfo.pNext = nullptr;
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.pInheritanceInfo = nullptr;
   beginInfo.flags = 0U;

   VkBufferCopy bufferCopy{};
   //                  {x, y, z}
   bufferCopy.dstOffset = dstOffset;
   bufferCopy.srcOffset = srcOffset;
   bufferCopy.size = size;

   vkBeginCommandBuffer(commandBuffer, &beginInfo);
   vkCmdCopyBuffer(commandBuffer, src.get(), dst.get(), 1U, &bufferCopy);
   vkEndCommandBuffer(commandBuffer);

   VkFenceCreateInfo transferCompleteFenceInfo{};
   transferCompleteFenceInfo.pNext = nullptr;
   transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

   VkFence transferCompleteFence;
   vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

   vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

   vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

   vkDeviceWaitIdle(app->logicalDevice.get());
   
   vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);
}

void AppBuffer::destroy()
{
   getApp()->resources.buffers.destroy(getIterator(), getApp()->logicalDevice.get());
}
