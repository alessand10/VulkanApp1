#include "app-base.h"
#include "pipeline-layout-resource.h"

void AppPipelineLayout::init(AppBase* appBase, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges)
{
    // Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); 
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); 
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size(); 
    pipelineLayoutInfo.pPushConstantRanges =  pushConstantRanges.size() == 0 ? nullptr : pushConstantRanges.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    THROW(vkCreatePipelineLayout(appBase->getDevice(), &pipelineLayoutInfo, NULL, &pipelineLayout), "Failed to create pipeline layout")

    AppResource::init(appBase, appBase->resources.pipelineLayouts.create(pipelineLayout));
}

void AppPipelineLayout::destroy()
{
    appBase->resources.pipelineLayouts.destroy(getIterator(), appBase->getDevice());
}
