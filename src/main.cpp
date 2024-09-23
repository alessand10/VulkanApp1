#include <iostream>
#include "GLFW/glfw3.h"
#include "vulkan-app.h"
#include <mesh.h>
#include <filesystem>
#include <fstream>

#define STRING(x) #x
#define XSTRING(x) STRING(x)

int main() {
    std::filesystem::current_path(XSTRING(SOURCE_ROOT));
    VulkanApp app;
    app.init();

    app.meshManager.importMeshFromOBJ("../mesh/cube1.obj");

    app.renderLoop();
    app.cleanup();
    
    return 0;
}