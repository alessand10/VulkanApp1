#include "app-base.h"
#include "semaphore-resource.h"

void AppSemaphore::init(AppBase* appBase)
{
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    THROW(vkCreateSemaphore(appBase->getDevice(), &createInfo, nullptr, &semaphore), "Failed to create semaphore");
    AppResource::init(appBase, appBase->resources.semaphores.create(semaphore));
}

void AppSemaphore::destroy()
{
    appBase->resources.semaphores.destroy(getIterator(), appBase->getDevice());
}
