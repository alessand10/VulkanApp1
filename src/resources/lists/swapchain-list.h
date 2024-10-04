#pragma once
#include "resource-list.h"

class SwapchainList : public ResourceList<VkSwapchainKHR> {
    public:
    virtual void destroy(std::list<VkSwapchainKHR>::iterator it, VkDevice device) {
        vkDestroySwapchainKHR(device, *it, nullptr);
        ResourceList::destroy(it);
    }
};