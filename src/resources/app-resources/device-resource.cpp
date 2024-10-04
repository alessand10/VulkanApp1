#include "device-resource.h"

void AppDevice::init(VulkanApp *app)
{
    //... Do some work to create the device
    // TO DO: Implement device creation
    //
    AppResource::init(app, app->resources.devices.create(VkDevice{}));
}
