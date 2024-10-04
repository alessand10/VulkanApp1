#pragma once
#include "app-resource.h"

class AppPipeline : public AppResource<VkPipeline> {
    public:
    void init(VulkanApp* app, std::vector<AppShaderModule> shaderModules, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);
    void destroy() { getApp()->resources.pipelines.destroy(getIterator(), getApp()->logicalDevice.get()); }
};