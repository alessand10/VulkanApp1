#include "vulkan-app.h"
#include "device-resource.h"

void AppDevice::init(VulkanApp *app, VkPhysicalDevice physicalDevice)
{
    //... Do some work to create the device
    // TO DO: Implement device creation
    //
    AppResource::init(app, app->resources.devices.create(VkDevice{}));
}

void AppDevice::destroy()
{
    getApp()->resources.devices.destroy(getIterator());
}
