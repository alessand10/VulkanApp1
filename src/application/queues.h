#pragma once
#include "vulkan/vulkan.hpp"

struct Queues {
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
};