#pragma once
#include "red-black-tree.h"


class VertexIndexSearchTree {
    struct Normal {
        uint32_t normalIndex;
        int insertionIndex = -1;
    };
    
    struct TexCoord {
        uint32_t texCoordIndex;
        RBTree<Normal> normalTree;
    };

    struct Position {
        uint32_t positionIndex;
        RBTree<TexCoord> texCoordTree;
    };

    RBTree<Position> positionTree;

    public:

    /**
     * @brief Inserts a set of vertex indices into the search tree
     * 
     * @note The return value is a pointer so that the caller can set the insertion index if a new set of vertex indices was inserted
     * 
     * @return A pointer to the insertion index if a new set of vertex indices was inserted, nullptr otherwise
     */
    int* insertVertexIndices(uint32_t positionIndex, uint32_t texCoordIndex, uint32_t normalIndex) {

        // Skip the search if we have already determined the vertex index combination does not exist
        bool skipSearch = false; 
        RBTree<Position>::Node* positionNode = positionTree.find(positionIndex);
        if (positionNode == nullptr) {
            Position position = {positionIndex, RBTree<TexCoord>()};
            positionNode = positionTree.insert(positionIndex, position);
            skipSearch = true;
        }


        RBTree<TexCoord>::Node* texCoordNode = skipSearch ? nullptr : positionNode->data.front().texCoordTree.find(texCoordIndex);
        if (texCoordNode == nullptr) {
            TexCoord texCoord = {texCoordIndex, RBTree<Normal>()};
            texCoordNode = positionNode->data.front().texCoordTree.insert(texCoordIndex, texCoord);
            skipSearch = true;
        }

        RBTree<Normal>::Node* normalNode = skipSearch ? nullptr : texCoordNode->data.front().normalTree.find(normalIndex);
        if (normalNode == nullptr) {
            Normal normal = {normalIndex};
            normalNode = texCoordNode->data.front().normalTree.insert(normalIndex, normal);
        }

        // Returns a pointer to the insertion index, which will be -1 if the vertex index combination doesn't exist
        return &(normalNode->data.front().insertionIndex); 
    }
};
