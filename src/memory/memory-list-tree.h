#pragma once
#include "inttypes.h"
#include <list>
#include "red-black-tree.h"
#include <stdexcept>
#include "memory-block-node.h"

/**
 * A data structure that combines a Doubly Linked List and Red-Black Tree to manage reserved and free
 * sections of Memory. 
 */
class MemoryListTree {
    protected:

    std::list<MemoryBlockNode> memoryBlocks = {};
    RBTree<std::list<MemoryBlockNode>::iterator> freeBlocks;

    /**
     * @brief Takes a free block and either partially reserves it or fully reserves it depending on the size requested
     * 
     * @note Partially reserving splits the block into a reserved region of 'size' bytes and a free region of the remaining bytes
     * 
     * @param it An iterator to the free memory block that will be partially reserved
     * @param size The number of bytes to reserve
     * 
     * @return An iterator to the reserved region is returned
     */
    std::list<MemoryBlockNode>::iterator reserveBlockHelper(RBTree<std::list<MemoryBlockNode>::iterator>::Node* node, uint32_t reserveSize) {
        MemoryBlockNode* memoryBlockNode = &(*(node->data.front()));
        std::list<MemoryBlockNode>::iterator it = node->data.front();
        uint32_t blockSize = memoryBlockNode->byteSize;

        // The app has requested a larger size than is available, throw an exception
        if (blockSize < reserveSize) throw std::runtime_error(std::string("Attempted to partially allocate from a region that is too small"));

        // There is exactly enough space, delete the 'freeBlock' entry to now consider this region as reserved
        else if (blockSize == reserveSize) {
            freeBlocks.remove(memoryBlockNode->nodeRef);
            (*memoryBlockNode).nodeRef = nullptr;
            return it;
        }

        // There is more space than requested, split the free region into a reserved region (of size 'reserveSize') and a free region 
        else {
            // Delete the free block in the RBTree
            freeBlocks.remove(memoryBlockNode->nodeRef);
            
            // Insert a new block in front of the block referenced by (*it), this will be the new free block and takes the remaining size
            uint32_t oldOffset = memoryBlockNode->byteOffset;
            uint32_t oldSize = memoryBlockNode->byteSize;

            std::list<MemoryBlockNode>::iterator newBlock = memoryBlocks.insert(std::next(it), {oldSize - reserveSize, oldOffset + reserveSize, nullptr});

            // Resize the originally free block and nullify the original RBTree node ref since it isn't valid anymore
            (*memoryBlockNode).byteSize = reserveSize;
            (*memoryBlockNode).nodeRef = nullptr;

            // Add the new free block to the RBTree
            RBTree<std::list<MemoryBlockNode>::iterator>::Node* freeBlockNodeRef = freeBlocks.insert((*newBlock).byteSize, newBlock);
            (*newBlock).nodeRef = freeBlockNodeRef;

            return it;
        }
    }
    
    public:
    /**
     * @brief Initializes the memory list tree to represent a free and contiguous memory block
     * @param size The size of the contiguous initial region size in bytes
     */
    void init(uint32_t size) {

        // Create an initial memory block
        memoryBlocks.push_front({size, 0U, nullptr});

        std::list<MemoryBlockNode>::iterator beginIt = memoryBlocks.begin();
        RBTree<std::list<MemoryBlockNode>::iterator>::Node* nodeRef = freeBlocks.insert((*beginIt).byteSize, beginIt);
        (*beginIt).nodeRef = nodeRef;
    }

    /**
     * @brief Takes a reserved block and frees it
     * 
     * @note If the block to be freed is next to another free block on the left and/or right,
     * they will be merged together into one free block of memory
     * 
     * @param it An iterator to the memory block to be freed
     * 
     * @return An iterator to the free memory block
     */
    std::list<MemoryBlockNode>::iterator free(std::list<MemoryBlockNode>::iterator &it) {

        bool isLeftFree = it != memoryBlocks.begin() && (*std::prev(it)).getIsFree();
        bool isRightFree = std::next(it) != memoryBlocks.end() && (*std::next(it)).getIsFree();

        if (isLeftFree){
            std::list<MemoryBlockNode>::iterator leftIt = std::prev(it);
            // Remove the leftmost block's free region, it will be merged at the end in a new free region
            freeBlocks.remove((*leftIt).nodeRef);

            // Combine the size of the requested freed region with the free region on the left
            (*it).byteSize += (*leftIt).byteSize;

            // Inherit the left region's offset, since the new region will start at the left region's offset
            (*it).byteOffset = (*leftIt).byteOffset;

            // Erase the memory block on the left, it is merged into the new region
            memoryBlocks.erase(leftIt);
        }
        if (isRightFree){
            std::list<MemoryBlockNode>::iterator rightIt = std::next(it);

            // Remove the rightmost block's free region, it will be merged at the end in a new free region
            freeBlocks.remove((*rightIt).nodeRef);
            
            // Combine the size of the requested freed region (and the left region if also free) with the free region on the right
            (*it).byteSize += (*rightIt).byteSize;

            // Erase the memory block region on the right, it is merged into the new region
            memoryBlocks.erase(rightIt);
        }

        // Create a new free region entry for the combined blocks
        RBTree<std::list<MemoryBlockNode>::iterator>::Node* nodeRef = freeBlocks.insert((*it).byteSize, it);
        (*it).nodeRef = nodeRef;

        return it;
    }

    /**
     * @brief Allocates the smallest block of memory possible within the unallocated blocks, returns the MemoryBlockNode corresponding to the allocated block
     */
    std::list<MemoryBlockNode>::iterator reserve(uint32_t size) {
        RBTree<std::list<MemoryBlockNode>::iterator>::Node* nodeRef = freeBlocks.getSmallestNodeGreaterThan(size);
        return reserveBlockHelper(nodeRef, size);
    }


};