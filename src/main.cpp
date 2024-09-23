#include <iostream>
#include "GLFW/glfw3.h"
#include "vulkan-app.h"
#include <mesh.h>

int main() {
    VulkanApp app;
    app.init();

    app.meshManager.importMeshFromOBJ("C:/CodeProjects/VulkanApp1/mesh/cube1.obj");

    app.renderLoop();
    app.cleanup();
    
    return 0;
}