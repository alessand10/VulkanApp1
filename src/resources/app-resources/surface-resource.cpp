#include "app-base.h"
#include "surface-resource.h"
#include "GLFW/glfw3.h"

void AppSurface::init(AppBase* appBase, GLFWwindow *window)
{
    VkSurfaceKHR surface;
    THROW(glfwCreateWindowSurface(appBase->getInstance(), window, nullptr, &surface), "Failed to create window surface");
    
    AppResource::init(appBase, appBase->resources.surfaces.create(surface));
}

void AppSurface::destroy()
{
    appBase->resources.surfaces.destroy(getIterator(), appBase->getInstance());
}
