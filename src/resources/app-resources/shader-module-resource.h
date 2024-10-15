#pragma once
#include "app-resource.h"

class AppShaderModule : public AppResource<VkShaderModule> {
    VkShaderStageFlagBits shaderStageFlags;
    public:
    void init(class AppBase* appBase, std::vector<char> bytecode, VkShaderStageFlagBits shaderStageFlags);
    VkShaderStageFlagBits getShaderStage() { return shaderStageFlags; }
    void destroy();
};