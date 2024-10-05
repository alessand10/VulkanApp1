#pragma once
#include "lists/buffer-list.h"
#include "lists/command-pool-list.h"
#include "lists/descriptor-pool-list.h"
#include "lists/descriptor-set-layout-list.h"
#include "lists/device-list.h"
#include "lists/device-memory-list.h"
#include "lists/fence-list.h"
#include "lists/image-list.h"
#include "lists/image-view-list.h"
#include "lists/instance-list.h"
#include "lists/pipeline-layout-list.h"
#include "lists/render-pass-list.h"
#include "lists/sampler-list.h"
#include "lists/semaphore-list.h"
#include "lists/shader-module-list.h"
#include "lists/surface-list.h"
#include "lists/swapchain-list.h"
#include "lists/pipeline-list.h"
#include "lists/framebuffer-list.h"

class Resources {
    public:
    InstanceList instances;
    SurfaceList surfaces;
    DeviceList devices;
    DeviceMemoryList deviceMemorySet;
    ImageList images;
    ImageViewList imageViews;
    BufferList buffers;
    SamplerList samplers;
    DescriptorSetLayoutList descriptorSetLayouts;
    DescriptorPoolList descriptorPools;
    CommandPoolList commandPools;
    FenceList fences;
    SemaphoreList semaphores;
    PipelineLayoutList pipelineLayouts;
    RenderPassList renderPasses;
    ShaderModuleList shaderModules;
    SwapchainList swapchains;
    PipelineList pipelines;
    FramebufferList framebuffers;

    void destroyAll(VkDevice device, VkInstance instance) {
        imageViews.destroyAll(device);
        images.destroyAll(device);
        buffers.destroyAll(device);
        deviceMemorySet.destroyAll(device);
        samplers.destroyAll(device);
        descriptorSetLayouts.destroyAll(device);
        descriptorPools.destroyAll(device);
        commandPools.destroyAll(device);
        fences.destroyAll(device);
        semaphores.destroyAll(device);
        pipelineLayouts.destroyAll(device);
        renderPasses.destroyAll(device);
        shaderModules.destroyAll(device);
        framebuffers.destroyAll(device);
        swapchains.destroyAll(device);
        surfaces.destroyAll(instance);
        pipelines.destroyAll(device);
        devices.destroyAll();
        instances.destroyAll();
    }
};