#pragma once
#include "app-resource.h"
#include "shader-module-resource.h"
#include "pipeline-layout-resource.h"
#include "render-pass-resource.h"

class AppPipeline : public AppResource<VkPipeline> {
    public:
    void init(class AppBase* appBase, std::vector<AppShaderModule> shaderModules, AppPipelineLayout pipelineLayout, AppRenderPass renderPass, uint32_t flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT);
    void destroy();
};