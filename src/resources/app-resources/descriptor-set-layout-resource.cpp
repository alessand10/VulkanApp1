#include "vulkan-app.h"
#include "descriptor-set-layout-resource.h"

void AppDescriptorSetLayout::init(VulkanApp *app, std::vector<DescriptorItem> descriptorItems)
{
    uint32_t index = 0U;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};
    for (DescriptorItem descriptorItem : descriptorItems) {
        VkDescriptorType descriptorType = descriptorItem.descriptorType;
        if (layoutBindings.size() > 0 && layoutBindings[index - 1].descriptorType == descriptorType && false)
            layoutBindings[index - 1].descriptorCount++;
        else {
             /**
             * 
             * binding: Set to 0, this corresponds to the binding number of the UBO that we will be binding in the shader.
             * descriptorCount: We are only binding and creating 1 descriptor in this case. 
             * descriptorType: The type of descriptors that this descriptor set consists of (set for UBOs).
             * stageFlags: Set to the vertex shader since we will be accessing the UBO in the vertex shader.
             * pImmutableSamplers: Used if this descriptor set is a sampler resource (nullptr because UBO not sampler).
             */
            layoutBindings.push_back(VkDescriptorSetLayoutBinding{});
            layoutBindings.back().binding = index;
            layoutBindings.back().descriptorCount = 1U;
            layoutBindings.back().stageFlags = descriptorItem.shaderStage;
            layoutBindings.back().descriptorType = descriptorType;
            layoutBindings.back().pImmutableSamplers = nullptr;
        }
        index++;
    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.bindingCount = layoutBindings.size();
    createInfo.pBindings = layoutBindings.data();
    createInfo.flags = 0U;


    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    THROW(vkCreateDescriptorSetLayout(app->logicalDevice.get(), &createInfo, nullptr, &descriptorSetLayout), "Failed to create descriptor set layout");
    
    AppResource::init(app, app->resources.descriptorSetLayouts.create(descriptorSetLayout));
}

void AppDescriptorSetLayout::destroy()
{
    getApp()->resources.descriptorSetLayouts.destroy(getIterator(), getApp()->logicalDevice.get());
}

static VkDescriptorType getDescriptorTypeFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE : {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

static VkShaderStageFlags getDescriptorStageFlagFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER : {
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
    }
}