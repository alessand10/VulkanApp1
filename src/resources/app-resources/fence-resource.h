#pragma once
#include "app-resource.h"

class AppFence : public AppResource<VkFence> {
    public:
    void init(class AppBase* appBase, VkFenceCreateFlags flags = 0U);

    void destroy();
};