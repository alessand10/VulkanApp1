#include "semaphore-resource.h"

void AppSemaphore::init(VulkanApp *app)
{
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    THROW(vkCreateSemaphore(app->logicalDevice.get(), &createInfo, nullptr, &semaphore), "Failed to create semaphore");
    AppResource::init(app, app->resources.semaphores.create(semaphore));
}