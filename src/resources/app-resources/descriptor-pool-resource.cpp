#include "app-base.h"
#include "descriptor-pool-resource.h"
#include "descriptor-set-layout-resource.h"

void AppDescriptorPool::init(AppBase* appBase, uint32_t maxSetsCount, std::map<VkDescriptorType, uint32_t> descriptorTypeCounts)
{
    this->maxSetsCount = maxSetsCount;
    this->descriptorTypeCounts = descriptorTypeCounts;

    std::vector<VkDescriptorPoolSize> poolSizes = {};
    for (auto descriptorTypeCount = descriptorTypeCounts.begin() ; descriptorTypeCount != descriptorTypeCounts.end() ; descriptorTypeCount++) {
        VkDescriptorType type = descriptorTypeCount->first;
        uint32_t count = descriptorTypeCount->second;
        poolSizes.push_back(VkDescriptorPoolSize{type, count});
    }

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = maxSetsCount;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    THROW(vkCreateDescriptorPool(appBase->getDevice(), &createInfo, nullptr, &descriptorPool), "Failed to create descriptor pool");
    AppResource::init(appBase, appBase->resources.descriptorPools.create(descriptorPool));
}

VkDescriptorSet AppDescriptorPool::allocateDescriptorSet(AppDescriptorSetLayout* descriptorSetLayout)
{
    if (!this->isInitialized) throw std::runtime_error("Failed to allocate descriptor set, descriptor pool not initialized");

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.pSetLayouts = descriptorSetLayout->getRef();
    allocInfo.descriptorSetCount = 1U;
    allocInfo.descriptorPool = this->get();

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    THROW(vkAllocateDescriptorSets(appBase->getDevice(), &allocInfo, &descriptorSet), "Failed to allocate descriptor set");
    
    return descriptorSet;
}

void AppDescriptorPool::destroy()
{
    appBase->resources.descriptorPools.destroy(getIterator(), appBase->getDevice());
}