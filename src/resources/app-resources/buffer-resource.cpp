#include "app-base.h"
#include "buffer-resource.h"
#include "device-resource.h"
#include "device-memory-resource.h"

static VkBufferCreateInfo getBufferCreateInfoFromTemplate(AppBufferTemplate t, uint32_t size) { 
    switch(t) {
        case AppBufferTemplate::UNIFORM_BUFFER :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           size,
           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           size,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           size,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           size,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           size,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        default:
            return {};
    }
};

void AppBuffer::init(AppBase* appBase, size_t size, AppBufferTemplate appBufferTemplate)
{
   this->appBufferTemplate = appBufferTemplate;
   this->size = size;

   // Populate the create info struct
   VkBufferCreateInfo createInfo{getBufferCreateInfoFromTemplate(appBufferTemplate, size)};

   // Attempt to create the buffer
   VkBuffer buffer = VK_NULL_HANDLE;
   THROW(vkCreateBuffer(appBase->getDevice(), &createInfo, nullptr, &buffer), "Failed to create buffer"); 

   // Add the buffer to the list of managed buffer, returning an iterator
   AppResource::init(appBase, appBase->resources.buffers.create(buffer));
}

void AppBuffer::bindToMemory(AppDeviceMemory *bufferMemory)
{
   VkDevice device = appBase->getDevice();
   VkBuffer buffer = get();
   VkDeviceMemory memory = bufferMemory->get();
   this->bufferMemory = bufferMemory;

   THROW(vkBindBufferMemory(device, buffer, memory, 0U), "Failed to bind image to memory");
}

void AppBuffer::copyBuffer(AppBuffer &src, AppBuffer &dst, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
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

   AppBase* resources = src.appBase;

   VkFence transferCompleteFence;
   vkCreateFence(resources->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

   vkQueueSubmit(resources->queues.graphicsQueue, 1U, &submitInfo, transferCompleteFence);

   vkWaitForFences(resources->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

   vkDeviceWaitIdle(resources->logicalDevice.get());
   
   vkDestroyFence(resources->logicalDevice.get(), transferCompleteFence, nullptr);
}

void AppBuffer::destroy()
{
   appBase->resources.buffers.destroy(getIterator(), appBase->logicalDevice.get());
}
