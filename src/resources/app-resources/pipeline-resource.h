#pragma once
#include "app-resource.h"
#include "shader-module-resource.h"
#include "pipeline-layout-resource.h"
#include "render-pass-resource.h"

class AppPipeline : public AppResource<VkPipeline> {
    public:
    void init(class VulkanApp* app, std::vector<AppShaderModule> shaderModules, AppPipelineLayout pipelineLayout, AppRenderPass renderPass);
    void destroy();
};