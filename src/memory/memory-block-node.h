#pragma once
#include "red-black-tree.h"

/**
 * offset: Offset from the start of an allocated section of memory
 * size: The size, in bytes, of this memory block
 * allocated: Whether this block is allocated or free.
 */
struct MemoryBlockNode {
    uint32_t byteSize = -1;
    uint32_t byteOffset = -1;

    bool getIsFree() {return nodeRef != nullptr;}
    RBTree<std::list<MemoryBlockNode>::iterator>::Node* nodeRef = nullptr;
};