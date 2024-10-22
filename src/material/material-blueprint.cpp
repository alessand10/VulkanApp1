#include "material-blueprint.h"
#include "pipeline-resource.h"
#include "descriptor-set-layout-resource.h"
#include "descriptor-pool-resource.h"
#include "resource-utilities.h"
#include "material-input.h"
#include "app-base.h"
#include "image/image.h"

void MaterialBlueprint::init(AppBase* appBase, AppPipeline* pipeline) 
{
    // Set the pipeline
    this->pipeline = pipeline;
    
    std::vector<DescriptorItem> descriptorItems = {};

    // Add image array descriptor if there are any input texture images
    if (materialInput->getInputImages().size() != 0){
        descriptorItems.push_back(DescriptorItem{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});
    }

    AppDescriptorSetLayout descriptorSetLayout;
    descriptorSetLayout.init(appBase, descriptorItems);

    
}

Material MaterialBlueprint::createMaterial(MaterialInput* materialInput)
{ 
    // A valid pipeline must exist to create a material, fetch the app base
    AppBase* appBase = pipeline->getAppBase();

    std::vector<MaterialInputImage> materialInputImages =  materialInput->getInputImages();
    uint32_t width = materialInputImages[0].image->getWidth();
    uint32_t height = materialInputImages[0].image->getHeight();

    Material material{};
    // Allocate a descriptor set for the material

    VkDescriptorSet materialDescriptorSet = descriptorPool->allocateDescriptorSet(descriptorSetLayout);
    material.setDescriptorSet(materialDescriptorSet);

    // Create the image array for all images
    AppImageBundle imageBundle = createImageAll(appBase, width, height, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE, materialInputImages.size());

    // Copy the image data to the VkImage array
    for (uint32_t i = 0; i < materialInputImages.size(); i++) {
        
    }

    createImageAll(appBase, mat)

    for (InputTextureImage textureImage : materialInput->inputTextureImageOrder) {
        //updateDescriptor(textureImage.imageBundle->imageView, material.getDescriptorSet(), 0U,)
    }

    return material;
}
