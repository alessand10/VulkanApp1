#include "mesh.h"
#include <fstream>
#include <iostream>

Mesh::Mesh()
{
    positions = {};
    texCoords = {};
    normals = {};
    faces = {};
}

void Mesh::importOBJ(const char *path)
{

}
