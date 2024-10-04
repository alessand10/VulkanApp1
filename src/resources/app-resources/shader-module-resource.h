#pragma once
#include "app-resource.h"

class AppShaderModule : public AppResource<VkShaderModule> {
    VkShaderStageFlagBits shaderStageFlags;
    public:
    void init(VulkanApp* app, std::vector<char> bytecode, VkShaderStageFlagBits shaderStageFlags);
    void destroy() { getApp()->resources.shaderModules.destroy(getIterator(), getApp()->logicalDevice.get()); }
};