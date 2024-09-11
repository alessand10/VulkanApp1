#include <iostream>
#include "GLFW/glfw3.h"
#include "vulkan-app.h"
#include <mesh.h>

int main() {
    VulkanApp app;
    app.init();

    app.meshManager.importMeshFromOBJ("../mesh/cube1.obj");

    app.renderLoop();
    app.cleanup();
    
    return 0;
}