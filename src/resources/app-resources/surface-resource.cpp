#include "vulkan-app.h"
#include "surface-resource.h"
#include "GLFW/glfw3.h"

void AppSurface::init(VulkanApp *app, GLFWwindow *window)
{
    VkSurfaceKHR surface;
    THROW(glfwCreateWindowSurface(app->instance.get(), window, nullptr, &surface), "Failed to create window surface");
    
    AppResource::init(app, app->resources.surfaces.create(surface));
}

void AppSurface::destroy()
{
    getApp()->resources.surfaces.destroy(getIterator(), getApp()->instance.get());
}
