#include <iostream>
#include "GLFW/glfw3.h"
#include "vulkan-app.h"
#include <mesh.h>
#include <filesystem>
#include <fstream>


int main() {
    std::filesystem::current_path(XSTRING(SOURCE_ROOT));
    VulkanApp app;
    app.init();
    app.renderLoop();
    app.cleanup();
    
    return 0;
}