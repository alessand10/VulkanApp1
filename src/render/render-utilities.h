#pragma once
#include "vulkan/vulkan.hpp"

void appBeginRenderPass(class AppRenderPass* renderPass, class AppFramebuffer* framebuffer, VkCommandBuffer commandBuffer);

void drawMesh(class Mesh* mesh, VkCommandBuffer commandBuffer);