#include "mesh.h"
#include <fstream>
#include <iostream>
#include "material.h"

void Mesh::setMaterial(Material *material)
{
    this->material = material;
}
