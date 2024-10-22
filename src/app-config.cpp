#include "vulkan-app.h"
#include "file-utilities.h"
#include "resource-utilities.h"
#include "filesystem"
#include <iostream>
#include "app-config.h"
#include "render-utilities.h"
#include "vertex-buffer-manager.h"
#include "material-input.h"
#include "material-blueprint.h"
#include "image/image.h"
#include "image/image-loader.h"

AppImageBundle albedo;
AppImageBundle normal;

AppSwapchain swapchain;
AppImageBundle depthStencilImage;
AppPipelineLayout pipelineLayout;
AppRenderPass renderPass;
AppShaderModule vertexShaderModule;
AppShaderModule fragmentShaderModule;
AppPipeline graphicsPipeline;
AppCommandPool commandPool;
VkCommandBuffer commandBuffer;

AppBufferBundle stagingVertexBuffer;
AppBufferBundle deviceVertexBuffer;
AppBufferBundle stagingIndexBuffer;
AppBufferBundle deviceIndexBuffer;
VIBufferManager viBufferManager;

std::vector<AppBufferBundle> uniformBuffersVS;
AppDescriptorSetLayout descriptorSetLayout;
AppDescriptorPool descriptorPool;
std::vector<VkDescriptorSet> descriptorSetsPerFrame;

AppSampler sampler;

// Signal when an image is available
AppSemaphore imageAvailableSemaphore;

// Signal when rendering is complete
AppSemaphore renderingFinishedSemaphore;

// Fence is used to block execution until rendering of previous frame
AppFence inFlightFence;

std::vector<void*> mappedUBOs = {};

std::vector<AppFramebuffer> framebuffers = {};
std::vector<AppImageView> swapchainImageViews = {};

VSUniformBuffer uniformBuffer{};


void createSwapchainAndResources(VulkanApp* app) {
    swapchain.init(app, app->surface, app->viewportSettings.width, app->viewportSettings.height);

    std::vector<VkImage> swapchainImages = swapchain.getImages();

    // Resize the framebuffer and swapchain image view vectors
    framebuffers.clear();
    framebuffers.resize(swapchainImages.size());
    swapchainImageViews.clear();
    swapchainImageViews.resize(swapchainImages.size());

    for (uint32_t frame = 0U ; frame < swapchain.getImageCount() ; frame++){
        swapchainImageViews[frame].init(app, swapchainImages[frame], AppImageTemplate::SWAPCHAIN_FORMAT, 1U, 0U);

        // Create a framebuffer for the resources of this frame
        framebuffers[frame].init(app, &renderPass, {
            &swapchainImageViews[frame],
            &depthStencilImage.imageView
        });
    }
}

void destroySwapchainAndResources(VulkanApp* app) {
    for (uint32_t frame = 0U ; frame < swapchain.getImageCount() ; frame++) {
        framebuffers[frame].destroy();
        swapchainImageViews[frame].destroy();
    }
    swapchain.destroy();
    framebuffers.clear();
    swapchainImageViews.clear();
}

void VulkanApp::userInit() {
    // Create the depth stencil image, view and memory
    depthStencilImage = createImageAll(this, this->viewportSettings.width, this->viewportSettings.height, AppImageTemplate::DEPTH_STENCIL);

    renderPass.init(this,
        // Attachments
        { 
            AttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT,
            AttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT
        },
        // Subpasses
        {
            // subpass 1
            AppSubpass {
                {
                    // Color and depth stencil attachments at the 0th and 1st index, respectively
                    // This must match the framebuffer views
                    AppSubpassAttachmentRef{0U, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                    AppSubpassAttachmentRef{1U, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
                }
            }
        },
        {
            // subpass dependency
            {VK_SUBPASS_EXTERNAL, 0U, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0U, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0U}
        }
    );

    createSwapchainAndResources(this);


    // Create a command pool for graphics family command buffers
    commandPool.init(this, this->queueFamilyIndices.graphics);

    // Allocate a command buffer from the command pool
    commandBuffer = commandPool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Create the vertex and index staging buffers
    stagingVertexBuffer = createBufferAll(this, AppBufferTemplate::VERTEX_BUFFER_STAGING, sizeof(Vertex) * 200);
    deviceVertexBuffer = createBufferAll(this, AppBufferTemplate::VERTEX_BUFFER_DEVICE, sizeof(Vertex) * supportedVertexCount);
    stagingIndexBuffer = createBufferAll(this, AppBufferTemplate::INDEX_BUFFER_STAGING, sizeof(uint32_t) * 200);
    deviceIndexBuffer = createBufferAll(this, AppBufferTemplate::INDEX_BUFFER_DEVICE, sizeof(uint32_t) * supportedIndexCount);

    // Initialize the staging buffer managers
    viBufferManager.init(deviceVertexBuffer, deviceIndexBuffer, stagingVertexBuffer, stagingIndexBuffer);

    // Create a descriptor pool capable of storing the uniform buffer for each frame in flight, the albedo and the normal
    descriptorPool.init(this, swapchain.getImageCount(), {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain.getImageCount()},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
    });
    
    // Create the descriptor set layout
    descriptorSetLayout.init(this, {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
    });

    sampler.init(this, AppSamplerTemplate::DEFAULT);

    // Create an app image bundle for the albedo and normal textures
    albedo = createImageAll(this, 2048U, 2048U, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE, 2U);
    normal = createImageAll(this, 2048U, 2048U, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE, 2U);

    for (uint32_t frame = 0u; frame < swapchain.getImageCount() ; frame++) {
        // Create a uniform buffer for all frames in flight
        uniformBuffersVS.push_back(createBufferAll(this, AppBufferTemplate::UNIFORM_BUFFER, sizeof(VSUniformBuffer)));

        // Allocate the descriptor sets for each frame
        descriptorSetsPerFrame.push_back(descriptorPool.allocateDescriptorSet(&descriptorSetLayout));

        // Update all descriptors associated with the descriptor set for this frame
        updateDescriptor(uniformBuffersVS[frame].buffer, descriptorSetsPerFrame[frame], sizeof(VSUniformBuffer), 0U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        updateDescriptor(albedo.imageView, descriptorSetsPerFrame[frame], 1U, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler);
        updateDescriptor(normal.imageView, descriptorSetsPerFrame[frame], 2U, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler);

        // Map the memory for the Uniform Buffer Objects
        mappedUBOs.push_back({});
        vkMapMemory(logicalDevice.get(), uniformBuffersVS[frame].deviceMemory.get(), 0u, sizeof(VSUniformBuffer), 0, &(mappedUBOs[frame]));
    }

    std::vector<char> vertexShaderByteCode = readFile("../shaders/build/vert.spv");
    std::vector<char> fragmentShaderByteCode = readFile("../shaders/build/frag.spv");

    // Create the shader modules that will be used
    vertexShaderModule.init(this, vertexShaderByteCode, VK_SHADER_STAGE_VERTEX_BIT);
    fragmentShaderModule.init(this, fragmentShaderByteCode, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create the pipeline layout and pipeline
    pipelineLayout.init(this,
        // Specify descriptor sets
        {
            descriptorSetLayout.get()
        }, 
        // Specify push constant ranges
        {
            {
                VK_SHADER_STAGE_FRAGMENT_BIT, // Accessible shader stage
                0U, // Offset
                sizeof(FragmentPushConst) // Size
            }
        }
    );
    graphicsPipeline.init(this, 
        {vertexShaderModule, fragmentShaderModule},
        pipelineLayout,
        renderPass
    );

    // Create some sync primitives that we'll use during rendering
    renderingFinishedSemaphore.init(this);
    imageAvailableSemaphore.init(this);
    inFlightFence.init(this, VK_FENCE_CREATE_SIGNALED_BIT);

    

    geometryManager.importOBJ("../mesh/cube.obj", commandBuffer);
    geometryManager.importOBJ("../mesh/cube1.obj", commandBuffer);

      // Load the brick wall texture into layer 0 of the albedo and normal, respectively
    loadImage(this, "../images/alley-brick-wall_albedo.jpg", albedo.image, commandBuffer, 0U);
    loadImage(this, "../images/alley-brick-wall_normal-dx.jpg", normal.image, commandBuffer, 0U);

    loadImage(this, "../images/new-brick-wall-albedo.jpeg", albedo.image, commandBuffer, 1U);
    loadImage(this, "../images/new-brick-wall-normal.jpeg", normal.image, commandBuffer, 1U);

    viBufferManager.addGeometry(geometryManager.getMesh(0), commandBuffer);
    viBufferManager.addGeometry(geometryManager.getMesh(1), commandBuffer);

    Image brickWallAlbedo = ImageLoader::loadJPEGFromFile("../images/alley-brick-wall_albedo.jpg", 0U);
    Image brickWallNormal = ImageLoader::loadJPEGFromFile("../images/alley-brick-wall_normal.jpg", 0U);

    MaterialBlueprint pbrMaterialBlueprint;
    //pbrMaterialBlueprint.init(graphicsPipeline.get());

    struct PBRMaterialInput : public MaterialInput {
        MaterialInputImage albedo {"Albedo"};
        MaterialInputImage normal {"Normal"};
        
        PBRMaterialInput() {
            inputTextureImages = {albedo, normal};
        };
    };

    PBRMaterialInput brickWall;
    brickWall.albedo.image = &brickWallAlbedo;
    brickWall.normal.image = &brickWallNormal;

    Material matBrickWall = pbrMaterialBlueprint.createMaterial(&brickWall);

    geometryManager.getMesh(0)->setMaterial(&matBrickWall);

}

/**
 * Writes the command buffer to be submitted, using a multithreaded approach
 */
void writeCommandBuffer(uint32_t frame, AppBase* appBase) {
    ViewportSettings viewportSettings = appBase->viewportSettings;

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.get());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.get(), 0U, 1U, &descriptorSetsPerFrame[frame], 0U, nullptr);

    VkDeviceSize vertexBufferOffsets = 0U;
    vkCmdBindVertexBuffers(commandBuffer, 0U, 1U, deviceVertexBuffer.buffer.getRef(), &vertexBufferOffsets);
    vkCmdBindIndexBuffer(commandBuffer, deviceIndexBuffer.buffer.get(), 0U, VK_INDEX_TYPE_UINT32);

    appBeginRenderPass(&renderPass, &framebuffers[frame], commandBuffer);

    // Draw mesh 1
    FragmentPushConst pushConst{0U};
    vkCmdPushConstants(commandBuffer, pipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0U, sizeof(FragmentPushConst), &pushConst);
    drawMesh(appBase->geometryManager.getMesh(0U), commandBuffer);
    
    // Draw mesh 2
    pushConst.textureIndex = 1U;
    vkCmdPushConstants(commandBuffer, pipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0U, sizeof(FragmentPushConst), &pushConst);
    drawMesh(appBase->geometryManager.getMesh(1U), commandBuffer);
    
    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);
}



void VulkanApp::userTick(double deltaTime) {

    // Acquire the index of an available image to draw to
    uint32_t freeFrameIndex;

    // Wait for the in-flight fence to become signalled (last submitted queue has completed)
    vkWaitForFences(logicalDevice.get(), 1U, inFlightFence.getRef(), true, UINT64_MAX);

    vkAcquireNextImageKHR(logicalDevice.get(), swapchain.get(), UINT64_MAX, imageAvailableSemaphore.get(), VK_NULL_HANDLE, &freeFrameIndex);

    vkResetFences(logicalDevice.get(), 1U, inFlightFence.getRef());

    uniformBuffer.worldMatrix = glm::identity<glm::mat4>();
    uniformBuffer.projMatrix = appCamera.getProjMatrix();
    uniformBuffer.viewMatrix = appCamera.getViewMatrix();
    memcpy(mappedUBOs[freeFrameIndex], &uniformBuffer, sizeof(VSUniformBuffer));

    // Write the command buffer
    vkResetCommandBuffer(commandBuffer, 0U);
    writeCommandBuffer(freeFrameIndex, this);

    // Indicates that the color attachment output stage must wait for the imageAvailableSemaphore
    VkPipelineStageFlags waitSemaphoreStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore.get()};
    VkSubmitInfo submitInfo{};
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.waitSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores = renderingFinishedSemaphore.getRef();
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pWaitDstStageMask = waitSemaphoreStages;

    // Submit the recorded command buffer
    THROW(vkQueueSubmit(queues.graphicsQueue, 1U, &submitInfo, inFlightFence.get()), "Failed to submit queue");

    VkPresentInfoKHR presentInfo{};
    presentInfo.pNext = nullptr;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = swapchain.getRef();
    presentInfo.pWaitSemaphores = renderingFinishedSemaphore.getRef();
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pImageIndices = &freeFrameIndex;
    presentInfo.swapchainCount = 1U;

    THROW(vkQueuePresentKHR(queues.graphicsQueue, &presentInfo), "Failed to present");
}


