#include "vulkan-app.h"
#include "render-pass-resource.h"


static AttachmentType getAttachmentTypeFromTemplate(AttachmentTemplate t) {
    switch (t) {
        case AttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT: 
        case AttachmentTemplate::APP_TEXTURE_COLOR_ATTACHMENT: 
            return AttachmentType::COLOR;
        case AttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT:
            return AttachmentType::DEPTH_STENCIL;
        default: return AttachmentType::COLOR;
    }
}

static VkAttachmentDescription getAttachmentDescriptionFromTemplate(AttachmentTemplate t) {
    /**
     * flags;
     * format;
     * samples;
     * loadOp;
     * storeOp;
     * stencilLoadOp;
     * stencilStoreOp;
     * initialLayout;
     * finalLayout;
     */
    switch(t) {
        case AttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_B8G8R8A8_SRGB,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            };
        }
        case AttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };
        }
        case AttachmentTemplate::APP_TEXTURE_COLOR_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }
        default : 
        return {};
    }
}

void AppRenderPass::init(VulkanApp *app, std::vector<AttachmentTemplate> attachments, std::vector<AppSubpass> subpasses, std::vector<VkSubpassDependency> subpassDependencies)
{
    VkRenderPassCreateInfo renderPassInfo{};
    std::vector<VkAttachmentDescription> attachmentDescriptions = {};
    std::vector<VkSubpassDescription> subpassDescriptions = {};

    // Each std::vector<VkAttachmentReference> element is for a separate subpass
    std::vector<std::vector<VkAttachmentReference>> colorAttachmentReferences = {};
    std::vector<std::vector<VkAttachmentReference>> depthStencilAttachmentReferences = {};
    
    for (AttachmentTemplate attachment : attachments) {
        attachmentDescriptions.push_back(getAttachmentDescriptionFromTemplate(attachment));
    }

    for (AppSubpass subpass : subpasses) {
        colorAttachmentReferences.push_back(std::vector<VkAttachmentReference>{});
        depthStencilAttachmentReferences.push_back(std::vector<VkAttachmentReference>{});
        subpassDescriptions.push_back(VkSubpassDescription{});

        for (AppSubpassAttachmentRef ref : subpass.attachmentRefs) {
            AttachmentType attachmentType = getAttachmentTypeFromTemplate(attachments[ref.attachmentIndex]);
            if (attachmentType == AttachmentType::COLOR) {
                colorAttachmentReferences.back().push_back(VkAttachmentReference{});
                colorAttachmentReferences.back().back().attachment = ref.attachmentIndex;
                colorAttachmentReferences.back().back().layout = ref.imageLayout;
            }
            else if (attachmentType == AttachmentType::DEPTH_STENCIL) {
                depthStencilAttachmentReferences.back().push_back(VkAttachmentReference{});
                depthStencilAttachmentReferences.back().back().attachment = ref.attachmentIndex;
                depthStencilAttachmentReferences.back().back().layout = ref.imageLayout;
            }
        }

        subpassDescriptions.back().colorAttachmentCount = colorAttachmentReferences.size();
        subpassDescriptions.back().pColorAttachments = colorAttachmentReferences.back().size() == 0 ? nullptr : colorAttachmentReferences.back().data();
        subpassDescriptions.back().flags = 0U;
        subpassDescriptions.back().pDepthStencilAttachment = depthStencilAttachmentReferences.back().size() == 0 ? nullptr : &(depthStencilAttachmentReferences.back()[0]);

    }

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    renderPassInfo.flags = 0U;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    THROW(vkCreateRenderPass(app->logicalDevice.get(), &renderPassInfo, nullptr, &renderPass), "Failed to create render pass");
    AppResource::init(app, app->resources.renderPasses.create(renderPass));
}

void AppRenderPass::destroy()
{
    getApp()->resources.renderPasses.destroy(getIterator(), getApp()->logicalDevice.get());
}