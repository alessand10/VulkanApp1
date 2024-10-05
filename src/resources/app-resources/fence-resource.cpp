#include "vulkan-app.h"
#include "fence-resource.h"

void AppFence::init(VulkanApp *app, VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = flags;

    VkFence fence = VK_NULL_HANDLE;
    THROW(vkCreateFence(app->logicalDevice.get(), &createInfo, nullptr, &fence), "Failed to create fence");

    AppResource::init(app, app->resources.fences.create(fence));
}

void AppFence::destroy()
{
    getApp()->resources.fences.destroy(getIterator(), getApp()->logicalDevice.get()); 
}
