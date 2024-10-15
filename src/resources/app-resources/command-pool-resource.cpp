#include "app-base.h"
#include "command-pool-resource.h"

void AppCommandPool::init(AppBase* appBase, uint32_t queueFamilyIndex)
{
    this->queueFamilyIndex = queueFamilyIndex;

    VkCommandPoolCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool;
    THROW(vkCreateCommandPool(appBase->getDevice(), &createInfo, nullptr, &commandPool), "Failed to create command pool");
    
    AppResource::init(appBase, appBase->resources.commandPools.create(commandPool));
}

VkCommandBuffer AppCommandPool::allocateCommandBuffer(VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocInfo{};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.level = level;
    allocInfo.commandPool = this->get();
    allocInfo.commandBufferCount = 1U;

    VkCommandBuffer buffer;
    THROW(vkAllocateCommandBuffers(appBase->getDevice(), &allocInfo, &buffer), "Failed to allocate command buffer");
    return buffer;
}

void AppCommandPool::destroy()
{
    appBase->resources.commandPools.destroy(getIterator(), appBase->getDevice()); 
}
