#pragma once
#include "app-resource.h"

class PipelineLayout : public AppResource<VkPipelineLayout> {
    public:
    void init(VulkanApp* app, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges);
    
    void destroy() { getApp()->resources.pipelineLayouts.destroy(getIterator(), getApp()->logicalDevice.get()); }
};