#include "vulkan-app.h"
#include "pipeline-layout-resource.h"

void AppPipelineLayout::init(VulkanApp *app, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges)
{
    // Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); 
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); 
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size(); 
    pipelineLayoutInfo.pPushConstantRanges =  pushConstantRanges.size() == 0 ? nullptr : pushConstantRanges.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    THROW(vkCreatePipelineLayout(app->logicalDevice.get(), &pipelineLayoutInfo, NULL, &pipelineLayout), "Failed to create pipeline layout")

    AppResource::init(app, app->resources.pipelineLayouts.create(pipelineLayout));
}

void AppPipelineLayout::destroy()
{
    getApp()->resources.pipelineLayouts.destroy(getIterator(), getApp()->logicalDevice.get());
}
