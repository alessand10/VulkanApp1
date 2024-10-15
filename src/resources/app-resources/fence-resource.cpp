#include "app-base.h"
#include "fence-resource.h"

void AppFence::init(AppBase* appBase, VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = flags;

    VkFence fence = VK_NULL_HANDLE;
    THROW(vkCreateFence(appBase->getDevice(), &createInfo, nullptr, &fence), "Failed to create fence");

    AppResource::init(appBase, appBase->resources.fences.create(fence));
}

void AppFence::destroy()
{
   appBase->resources.fences.destroy(getIterator(), appBase->getDevice()); 
}
