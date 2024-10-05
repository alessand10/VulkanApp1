#include "vulkan-app.h"
#include "shader-module-resource.h"

void AppShaderModule::init(VulkanApp *app, std::vector<char> bytecode, VkShaderStageFlagBits shaderStageFlags)
{
    this->shaderStageFlags = shaderStageFlags;
    
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.codeSize = bytecode.size();
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    VkShaderModule shaderModule;
    THROW(vkCreateShaderModule(app->logicalDevice.get(), &shaderModuleCreateInfo, NULL, &shaderModule), "Failed to create shader module");

    AppResource::init(app, app->resources.shaderModules.create(shaderModule));
}

void AppShaderModule::destroy()
{
    getApp()->resources.shaderModules.destroy(getIterator(), getApp()->logicalDevice.get());
}
