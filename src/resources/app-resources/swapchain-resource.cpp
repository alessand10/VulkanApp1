#include "app-base.h"
#include "swapchain-resource.h"

void AppSwapchain::init(AppBase* appBase, AppSurface appSurface, uint32_t width, uint32_t height)
{
    AppSwapchain appSwapchain{};

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(appBase->physicalDevice, appBase->surface.get(), &surfaceCapabilities);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = appSurface.get();
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainCreateInfo.clipped = VK_FALSE;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.imageArrayLayers = 1U;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainCreateInfo.imageExtent.height = height;
    swapchainCreateInfo.imageExtent.width = width;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.pNext = NULL;


    VkSwapchainKHR swapchain;
    THROW(vkCreateSwapchainKHR(appBase->getDevice(), &swapchainCreateInfo, NULL, &swapchain), "Failed to create swapchain");
    AppResource::init(appBase, appBase->resources.swapchains.create(swapchain));
    
    uint32_t swapchainImageCount = 0;

    // Retrieve the image count
    THROW(vkGetSwapchainImagesKHR(appBase->getDevice(), get(), &swapchainImageCount, NULL), "Failed to retrieve swapchain images");

    imageCount = swapchainImageCount;
}

std::vector<VkImage> AppSwapchain::getImages()
{
    uint32_t swapchainImageCount = 0;

    // Get the count initially
    THROW(vkGetSwapchainImagesKHR(appBase->getDevice(), get(), &swapchainImageCount, NULL), "Failed to retrieve swapchain images");
    std::vector<VkImage> swapchainImages(swapchainImageCount);

    // Now populate the sized vector
    THROW(vkGetSwapchainImagesKHR(appBase->getDevice(), get(), &swapchainImageCount, swapchainImages.data()), "Failed to retrieve swapchain images");

    return swapchainImages;
}

uint32_t AppSwapchain::getImageCount()
{
    return imageCount;
}

void AppSwapchain::destroy()
{
    appBase->resources.swapchains.destroy(getIterator(), appBase->getDevice());
}
