#pragma once
#include "app-resource.h"

class AppPipelineLayout : public AppResource<VkPipelineLayout> {
    public:
    void init(class VulkanApp* app, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges);
    
    void destroy();
};