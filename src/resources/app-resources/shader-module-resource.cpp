#include "app-base.h"
#include "shader-module-resource.h"

void AppShaderModule::init(AppBase* appBase, std::vector<char> bytecode, VkShaderStageFlagBits shaderStageFlags)
{
    this->shaderStageFlags = shaderStageFlags;
    
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.codeSize = bytecode.size();
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    VkShaderModule shaderModule;
    THROW(vkCreateShaderModule(appBase->getDevice(), &shaderModuleCreateInfo, NULL, &shaderModule), "Failed to create shader module");

    AppResource::init(appBase, appBase->resources.shaderModules.create(shaderModule));
}

void AppShaderModule::destroy()
{
    appBase->resources.shaderModules.destroy(getIterator(), appBase->getDevice());
}
