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
};