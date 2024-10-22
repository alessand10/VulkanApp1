#include "render-utilities.h"
#include "framebuffer-resource.h"
#include "app-base.h"
#include "render-pass-resource.h"
#include "vulkan/vulkan.hpp"


void appBeginRenderPass(AppRenderPass *renderPass, AppFramebuffer *framebuffer, VkCommandBuffer commandBuffer)
{
    AppBase* appBase = framebuffer->getAppBase();
    VkClearValue clearValues[2]{};
    clearValues[0].color = {0.f, 0.f, 0.f, 1.f};
    clearValues[1].depthStencil = {1.f, 0U};
    VkRenderPassBeginInfo renderPassBeginInfo {};
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.framebuffer = framebuffer->get();
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass->get();
    renderPassBeginInfo.renderArea.extent = {appBase->viewportSettings.width, appBase->viewportSettings.height};
    renderPassBeginInfo.renderArea.offset = {static_cast<int>(appBase->viewportSettings.x), static_cast<int>(appBase->viewportSettings.y)};

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void drawMesh(Mesh *mesh, VkCommandBuffer commandBuffer)
{
    
    vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1U, mesh->getIndexOffset(), mesh->getVertexOffset(), 0U);
}
