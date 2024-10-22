#pragma once
#include <vector>
#include "glm/glm.hpp"


//void importMesh();

std::pair<glm::vec3, glm::vec3> computeTangentBitangent(glm::vec3 p1, glm::vec2 p1UV, glm::vec3 p2, glm::vec2 p2UV, glm::vec3 p3, glm::vec2 p3UV);

