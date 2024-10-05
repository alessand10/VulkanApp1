#include "vulkan-app.h"
#include "filesystem"

VulkanApp app;

// AppImageBundle albedo;
// AppImageBundle normal;

// struct FragmentPushConst {
//     uint32_t textureIndex = 0u;
// };

// struct VSUniformBuffer {
//     glm::mat4 worldMatrix;
//     glm::mat4 viewMatrix;
//     glm::mat4 projMatrix;
// };

// AppImageBundle depthStencilImage;
// VkPipelineLayout pipelineLayout;
// VkRenderPass renderPass;
// AppShaderModule vertexShaderModule;
// AppShaderModule fragmentShaderModule;
// VkPipeline graphicsPipeline;
// AppCommandPool commandPool;
// VkCommandBuffer commandBuffer;

// AppBufferBundle stagingVertexBuffer;
// AppBufferBundle deviceVertexBuffer;

// AppBufferBundle stagingIndexBuffer;
// AppBufferBundle deviceIndexBuffer;

// std::vector<AppBufferBundle> uniformBuffersVS;
// VkDescriptorSetLayout pipelineDescriptorSetLayout;
// VkDescriptorPool descriptorPool;
// std::vector<VkDescriptorSet> descriptorSetsPerFrame;

// AppSampler sampler;

// // Signal when an image is available
// VkSemaphore imageAvailableSemaphore;

// // Signal when rendering is complete
// VkSemaphore renderingFinishedSemaphore;

// // Fence is used to block execution until rendering of previous frame
// VkFence inFlightFence;

// std::vector<void*> mappedUBOs = {};

// /**
//  * Initialize the Vulkan application and all desired resources
//  */
// void onInit() {
//     // Create an app image bundle for the albedo and normal textures
//     albedo = createImageAll(&app, 2048U, 2048U, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE, 2U);
//     normal = createImageAll(&app, 2048U, 2048U, AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE, 2U);

    
//     commandPool.init(&app, app.queueFamilyIndices.graphics);
    
//     commandBuffer = commandPool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

//     // Load the brick wall texture into layer 0 of the albedo and normal, respectively
//     loadJPEGImage(&app, "../images/alley-brick-wall_albedo.jpg", albedo.image, commandBuffer, 0U);
//     loadJPEGImage(&app, "../images/alley-brick-wall_normal-dx.jpg", normal.image, commandBuffer, 0U);

//     loadJPEGImage(&app, "../images/new-brick-wall-albedo.jpeg", albedo.image, commandBuffer, 1U);
//     loadJPEGImage(&app, "../images/new-brick-wall-normal.jpeg", normal.image, commandBuffer, 1U);

//     // Create the vertex and index staging buffers
//     stagingVertexBuffer = createBufferAll(&app, AppBufferTemplate::VERTEX_BUFFER_STAGING, sizeof(Vertex) * supportedVertexCount);
//     deviceVertexBuffer = createBufferAll(&app, AppBufferTemplate::VERTEX_BUFFER_DEVICE, sizeof(Vertex) * supportedVertexCount);
//     stagingIndexBuffer = createBufferAll(&app, AppBufferTemplate::INDEX_BUFFER_STAGING, sizeof(uint32_t) * supportedIndexCount);
//     deviceIndexBuffer = createBufferAll(&app, AppBufferTemplate::INDEX_BUFFER_DEVICE, sizeof(uint32_t) * supportedIndexCount);

//     // Provide the mesh manager will the created buffers
//     meshManager.setVertexBuffers(stagingVertexBuffer, deviceVertexBuffer);
//     meshManager.setIndexBuffers(stagingIndexBuffer, deviceIndexBuffer);

//     swapchain = resourceManager.createSwapchain();
//     depthStencilImage = resourceManager.createImageAll(windowWidth, windowHeight, AppImageTemplate::DEPTH_STENCIL);

//     // Create a descriptor pool capable of storing the uniform buffer for each frame in flight, the albedo and the normal
//     descriptorPool = resourceManager.createDescriptorPool(maxFramesInFlight, {
//         {AppDescriptorItemTemplate::VS_UNIFORM_BUFFER, maxFramesInFlight},
//         {AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, 2}
//     });

//     pipelineDescriptorSetLayout = resourceManager.createDescriptorSetLayout({
//         AppDescriptorItemTemplate::VS_UNIFORM_BUFFER,
//         AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER,
//         AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER
//     });

//     // Create the application render pass
//     renderPass = resourceManager.createRenderPass(
//         // Attachments
//         { 
//             {AppAttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT},
//             {AppAttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT}
//         },
//         // Subpasses
//         {
//             // subpass 1
//             AppSubpass {
//                 {
//                     // Color and depth stencil attachments at the 0th and 1st index, respectively
//                     // This must match the framebuffer views
//                     AppSubpassAttachmentRef{0U, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
//                     AppSubpassAttachmentRef{1U, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
//                 }
//             }
//         },
//         {
//             // subpass dependency
//             {VK_SUBPASS_EXTERNAL, 0U, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0U, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0U}
//         }
//     );

//     sampler = resourceManager.createSampler(AppSamplerTemplate::DEFAULT);

//     for (uint32_t frame = 0u; frame < maxFramesInFlight ; frame++) {
//         // Create a uniform buffer for all frames in flight
//         uniformBuffersVS.push_back(resourceManager.createBufferAll(AppBufferTemplate::UNIFORM_BUFFER, sizeof(VSUniformBuffer)));

//         // Allocate the descriptor sets for each frame
//         descriptorSetsPerFrame.push_back(resourceManager.allocateDescriptorSet(pipelineDescriptorSetLayout, descriptorPool));

//         // Update all descriptors associated with the descriptor set for this frame
//         resourceManager.updateBufferDescriptor(descriptorSetsPerFrame[frame], uniformBuffersVS[frame].buffer.get(), sizeof(VSUniformBuffer), 0U, AppDescriptorItemTemplate::VS_UNIFORM_BUFFER);
//         resourceManager.updateImageDescriptor(albedo.imageView, descriptorSetsPerFrame[frame], 1U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);
//         resourceManager.updateImageDescriptor(normal.imageView, descriptorSetsPerFrame[frame], 2U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);

//         // Map the memory for the Uniform Buffer Objects
//         mappedUBOs.push_back({});
//         vkMapMemory(logicalDevice.get(), uniformBuffersVS[frame].deviceMemory.get(), 0u, sizeof(VSUniformBuffer), 0, &(mappedUBOs[frame]));

//         // Create a framebuffer for the resources of this frame
//         swapchain.framebuffers.push_back(resourceManager.createFramebuffer(renderPass, {
//             swapchain.swapchainImageViews[frame].get(),
//             depthStencilImage.imageView.get()
//         }));
//     }

//     // Create the shader modules that will be used
//     vertexShaderModule = resourceManager.createShaderModule("../shaders/build/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//     fragmentShaderModule = resourceManager.createShaderModule("../shaders/build/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

//     // Create the pipeline layout and pipeline
//     pipelineLayout = resourceManager.createPipelineLayout(
//         // Specify descriptor sets
//         {
//             pipelineDescriptorSetLayout
//         }, 
//         // Specify push constant ranges
//         {
//             {
//                 VK_SHADER_STAGE_FRAGMENT_BIT, // Accessible shader stage
//                 0U, // Offset
//                 sizeof(FragmentPushConst) // Size
//             }
//         }
//     );
//     graphicsPipeline = resourceManager.createGraphicsPipeline(
//         {vertexShaderModule, fragmentShaderModule},
//         pipelineLayout,
//         renderPass
//     );

//     // Create some sync primitives that we'll use during rendering
//     inFlightFence = resourceManager.createFence(true);
//     renderingFinishedSemaphore = resourceManager.createSemaphore();
//     imageAvailableSemaphore = resourceManager.createSemaphore();

//     meshManager.importMeshFromOBJ("../mesh/cube.obj", commandBuffer);
// }

// /**
//  * Callback that runs when a new frame has been acquired from the swapchain, and we can now render to it
//  */
// void onRender() {

// }

// /**
//  * Writes the command buffer to be submitted, using a multithreaded approach
//  */
// void writeCommandBuffer() {

// }


int main() {
    std::filesystem::current_path(XSTRING(SOURCE_ROOT));
//     app.init();
//     app.renderLoop();
//     app.cleanup();
    
//     return 0;
}