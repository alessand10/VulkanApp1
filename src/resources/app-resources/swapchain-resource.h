#pragma once
#include "app-resource.h"
#include "image-resource.h"
#include "image-view-resource.h"
#include "surface-resource.h"

class AppSwapchain : public AppResource<VkSwapchainKHR> {
    uint32_t imageCount;
    public:
    void init(class AppBase* appBase, AppSurface appSurface, uint32_t width, uint32_t height);
    
    /**
     * @brief Retrieves all images in the swapchain
     * 
     * @note The images are returned as VkImages because they are not managed by the app
     * 
     * @return A vector containing all images in the swapchain
     */
    std::vector<VkImage> getImages();
    uint32_t getImageCount();
    
    void destroy();
};
