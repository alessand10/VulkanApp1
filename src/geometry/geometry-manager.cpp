#include "geometry-manager.h"
#include <fstream>
#include <iostream>
#include <string>
#include "geometry-utilities.h"
#include <sstream>
#include "string-utilities.h"
#include "tiny_obj_loader.h"
#include <set>
#include "resource-structs.h"


// void GeometryManager::setVertexBuffers(AppBufferBundle stagingVertexBuffer, AppBufferBundle deviceVertexBuffer)
// {
//     this->stagingVertexBuffer = stagingVertexBuffer;
//     this->deviceVertexBuffer = deviceVertexBuffer;
// }

// void GeometryManager::setIndexBuffers(AppBufferBundle stagingIndexBuffer, AppBufferBundle deviceIndexBuffer)
// {
//     this->stagingIndexBuffer = stagingIndexBuffer;
//     this->deviceIndexBuffer = deviceIndexBuffer;
// }

void importHelper(std::vector<glm::vec3> &positionData, std::vector<glm::vec3> &normalData, std::vector<glm::vec2> &texCoordData, std::vector<Face> &faces, std::ifstream &file)
{
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string lineType = "";
        iss >> lineType;
        // Line is a vertex, extract xyz coordinate
        if (lineType == "v") {
            float x,y,z;
            if (!(iss >> x >> y >> z)) std::cout << "Corrupt vertex entry";
            positionData.push_back(glm::vec3{x,y,z});
        }
        else if (lineType == "vt") {
            float u,v;
            if (!(iss >> u >> v)) std::cout << "Corrupt texcoord entry";
            texCoordData.push_back(glm::vec2{u, v});
        }
        else if (lineType == "vn") {
            float x,y,z;
            if (!(iss >> x >> y >> z)) std::cout << "Corrupt normal entry";
            normalData.push_back(glm::vec3{x, y, z});
        }
        else if (lineType == "f") {
            std::string indexGroup;
            Face face;
            while (iss >> indexGroup) {
                std::vector<std::string> indices = splitString(indexGroup, "/");
                for (uint32_t index = 0u ; index < indices.size() ; index++) {
                    uint32_t indexUL = std::stoul(indices[index]) - 1U;
                    if (index == 0u) face.posIndices.push_back(indexUL);
                    else if (index == 1u) face.texCoordIndices.push_back(indexUL);
                    else if (index == 2u) face.normalIndices.push_back(indexUL);
                }
            }
            faces.push_back(face);
        }
    }
}

/**
 * @brief Splits a vertex into multiple vertices if it has multiple normals or texcoords
 */
void splitVertexHelper(std::vector<glm::vec3> &positionData, std::vector<glm::vec3> &normalData, std::vector<glm::vec2> &texCoordData, std::vector<Face> &faces) {
    std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> vertexMap;
    int uniqueVertices = 0;
    for (uint32_t i = 0U ; i < faces.size() ; i++) {
        Face face = faces[i];
        for (uint32_t j = 0U ; j < face.posIndices.size() ; j++) {
            uint32_t posIndex = face.posIndices[j];
            uint32_t normalIndex = face.normalIndices[j];
            uint32_t texCoordIndex = face.texCoordIndices[j];

            // Check if the pair already exists in the map
            if (vertexMap.find(posIndex) == vertexMap.end()) {
                vertexMap[posIndex].push_back(std::pair<uint32_t, uint32_t>{normalIndex, texCoordIndex});
                uniqueVertices++;
            }
            else {
                // If the key exists, check if the pair is the same
                bool pairExists = false;
                for (uint32_t k = 0U ; k < vertexMap[posIndex].size() ; k++) {
                    if (vertexMap[posIndex][k].first == normalIndex && vertexMap[posIndex][k].second == texCoordIndex) {
                        pairExists = true;
                        break;
                    }
                }
                if (!pairExists) {
                    uniqueVertices++;
                    vertexMap[posIndex].push_back({ normalIndex, texCoordIndex});
                    faces[i].posIndices[j] = positionData.size() - 1;
                    faces[i].normalIndices[j] = normalData.size() - 1;
                    faces[i].texCoordIndices[j] = texCoordData.size() - 1;
                }
            }
        }
    }
    // At this point, each vertex in the mesh has a unique position, normal and texcoord
}

/**
 * @brief Assembles a list of vertices from the position, normal and texcoord data
 * 
 * @note This function assumes that vertices have been split if they have multiple normals or texcoords
 */
void assembleVerticesAndIndices(std::vector<glm::vec3> &positionData, std::vector<glm::vec3> &normalData, std::vector<glm::vec2> &texCoordData, std::vector<Face> &faces, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    
    // Stores the position index's insertion index in the vertices vector
    std::map<uint32_t, uint32_t> vertexMap;

    for (uint32_t i = 0U ; i < faces.size() ; i++) {
        Face face = faces[i];
        for (uint32_t j = 0U ; j < face.posIndices.size() ; j++) {
            // If this vertex positon index has not been inserted into the vertex list, insert it
            if (vertexMap.find(face.posIndices[j]) == vertexMap.end()) {

                // Set the insertion index of the vertex in the vertex list
                vertexMap[face.posIndices[j]] = vertices.size();

                // Insert the vertex into the list
                vertices.push_back({
                    positionData[face.posIndices[j]],
                    normalData[face.normalIndices[j]],
                    glm::vec3{0.0f, 0.0f, 0.0f},
                    texCoordData[face.texCoordIndices[j]]
                });
                // Insert the index into the index list
                indices.push_back(vertices.size() - 1);
            }
            // Otherwise, insert the index of the vertex into the index list
            else {
                indices.push_back(vertexMap[face.posIndices[j]]);
            }
        }
    }
}

int GeometryManager::importOBJ(const char *path, VkCommandBuffer commandBuffer)
{
    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = "./"; // Path to the .mtl file
    readerConfig.triangulate = true; // Triangulate the faces

    // Instantiate the reader
    tinyobj::ObjReader reader;

    // Attempt to parse the file from the path
    if (!reader.ParseFromFile(path, readerConfig)) {

        // Display any errors that occurred during the parsing process
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return 0;
    }

    // Get the parsed attributes
    std::vector<tinyobj::shape_t> shapes = reader.GetShapes();
    tinyobj::attrib_t attrib = reader.GetAttrib();
    std::vector<tinyobj::material_t> materials = reader.GetMaterials();

    // position, normal and texCoord indices are absolute indices, so we must keep track of the offset for a shape
    uint32_t shapeIndexOffset = 0U;

    // Iterate through the shapes that we've parsed
    for (uint32_t s = 0U ; s < shapes.size() ; s++) {

        // Create a new mesh for this shape
        meshes.push_back({});
        Mesh* mesh = &meshes[meshes.size() - 1];

        // Store the indices of vertices, normals, texCoords that we must copy to the mesh
        // The key is the vertex index, the value is the local index in the mesh
        std::map<uint32_t, uint32_t> positionSet;
        std::map<uint32_t, uint32_t> normalSet;
        std::map<uint32_t, uint32_t> texCoordSet;
        mesh->setShapeName(shapes[s].name);

        // The faces are initially using an order that leads to inverted normals, push them to the mesh in this order
        int faceIndexOrder[] = {0, 1, -1};
        uint32_t numFacesProcessed = 0u;

        // Iterate through the indices of the shape
        for (int i = 0U ; i < shapes[s].mesh.indices.size() ; i++) {

            // Get the adjusted index, which is used to flip the normal (retrieve triangle indices in order 0, 2, 1 instead of 0, 1, 2)
            uint32_t adjustedIndex = i + faceIndexOrder[i % 3];
            tinyobj::index_t index = shapes[s].mesh.indices[adjustedIndex];

            GeometryBase::VertexIndices localIndices {};

            // If we haven't already inserted this vertex index into the mesh, insert it
            if (positionSet.find(index.vertex_index) == positionSet.end()) {
                positionSet[index.vertex_index] = mesh->getPositions().size() * 3;
                glm::vec3 position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                mesh->addPosition(position);
            }
            localIndices.positionIndex = positionSet[index.vertex_index] / 3;


            // If we haven't already inserted this normal index into the mesh, insert it
            if (normalSet.find(index.normal_index) == normalSet.end()) {
                normalSet[index.normal_index] = mesh->getNormals().size() * 3;
                glm::vec3 normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
                mesh->addNormal(normal);
            }
            localIndices.normalIndex = normalSet[index.normal_index] / 3;
            

            // If we haven't already inserted this texcoord index into the mesh, insert it
            if (texCoordSet.find(index.texcoord_index) == texCoordSet.end()) {
                texCoordSet[index.texcoord_index] = mesh->getTexCoords().size() * 2;
                glm::vec2 texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
                mesh->addTexCoord(texCoord);
            }
            localIndices.texCoordIndex = texCoordSet[index.texcoord_index] / 2;

            mesh->addVertexIndex(localIndices.positionIndex, localIndices.texCoordIndex, localIndices.normalIndex);
        }
    }
    return shapes.size();
}
