#pragma once
#include "inttypes.h"
#include <list>
#include <functional>
#include <stdexcept>

enum class RBTreeColor {RED, BLACK};

/**
 * A red-black tree capable of storing duplicate occurrences
 */

template <typename T>
class RBTree {
    public:
    struct Node {
        uint32_t key;
        // We store each occurrence in a new list element
        std::list<T> data;
        RBTreeColor color;
        Node* left;
        Node* right;
        Node* parent;
    };
    private:

    Node* root = nullptr;

    Node* bstInsert(uint32_t key, T &value, Node* &root) {
        // Simple case of the tree being empty
        if (root == nullptr){
            root = new Node{ key, {value}, RBTreeColor::RED, nullptr, nullptr, nullptr};
            return root;
        }


        // The values are equal, increment occurrences
        if (key == root->key){
            root->data.push_front(value);
            return root;
        }
        
        // The new entry is greater than the root node data
        else if (key > (root->key)) {
            if (root->right != nullptr){
                return bstInsert(key, value, root->right);
            }
            else {
                root->right = new Node{key, {value}, RBTreeColor::RED, nullptr, nullptr, root};
                return root->right;
            }
        }
        // The new entry is less than the root node data
        else {
            if (root->left != nullptr){
                return bstInsert(key, value, root->left);
            }
            else {
                root->left = new Node{key, {value}, RBTreeColor::RED, nullptr, nullptr, root};
                return root->left;
            }

        }
    }


    Node* getSmallestNodeGreaterThanHelper(Node* suitableNode, Node* rootNode, int minimum) {

        // If we have reached the end of the tree, return the most suitable node (may be nullptr)
        if (rootNode == nullptr) return suitableNode;

        if (rootNode->key > minimum) {

            // If the root node is lower than the previous most suitable node, or a suitable node has not been found, set the root node as the most suitable
            if (suitableNode == nullptr || (suitableNode->key > rootNode->key)) {
                suitableNode = rootNode;
            }

            // Proceed down the left side of the tree, since we want to find a lower value if possible
            return getSmallestNodeGreaterThanHelper(suitableNode, rootNode->left, minimum);

        }
        else if (rootNode->key < minimum) {
            return getSmallestNodeGreaterThanHelper(suitableNode, rootNode->right, minimum);
        }
        else {
            // The root node is exactly equal to the minimum we are searching for, return it
            return rootNode;
        }
    }

    /**
     * 
     * A right rotation will take the parent provided and make it's left child the new parent.
     * Since the left child is less than the parent, the parent will become the right child.
     */
    void rightRotation(Node* parent) {
        // Get the left child, which will become the new parent
        Node* leftChild = parent->left;

        // If the parent passed in is the root, the left child becomes the root
        if (parent == root) {
            root = leftChild;
        }
        // Otherwise, adjust the grandparent's left or right child reference depending on if the parent passed in is the grandparent's left or right child
        else if (parent == parent->parent->left) {
            parent->parent->left = leftChild;
        }
        else if (parent == parent->parent->right) {
            parent->parent->right = leftChild;
        }

        // Set the left child's parent to the original parent's parent
        leftChild->parent = parent->parent;

        // Set the original parent's parent to be the left child
        parent->parent = leftChild;

        // Set the original parent's left child to the left child's right child
        parent->left = leftChild->right;

        if (leftChild->right != nullptr) {
            leftChild->right->parent = parent;
        }

        // Set the left child's right child to be the original parent
        leftChild->right = parent;
    }

    void leftRotation(Node* parent) {
        // Get the right child, which will become the new parent
        Node* rightChild = parent->right;

        // If the parent passed in is the root, the right child becomes the root
        if (parent == root) {
            root = rightChild;
        }
        // Otherwise, adjust the grandparent's left or right child reference depending on if the parent passed in is the grandparent's left or right child
        else if (parent == parent->parent->left) {
            parent->parent->left = rightChild;
        }
        else if (parent == parent->parent->right) {
            parent->parent->right = rightChild;
        }

        // Set the right child's parent to the original parent's parent
        rightChild->parent = parent->parent;

        // Set the original parent's parent to be the right child
        parent->parent = rightChild;

        // Set the original parent's right child to the right child's left child
        parent->right = rightChild->left;

        // We previously set the old parent's right child to be equal to the right child's left child, set the inverse relationship 
        if (rightChild->left != nullptr) {
            rightChild->left->parent = parent;
        }

        // Set the left child's right child to be the original parent
        rightChild->left = parent;
    }

    void fixInsertionViolations(Node* node) {
        while (node != root && node->parent->color == RBTreeColor::RED) {
            Node* parent = node->parent;
            Node* grandparent = parent->parent;
            Node* uncle = nullptr;

            // Determine the uncle node
            if (parent == grandparent->left)
                uncle = grandparent->right;
            else
                uncle = grandparent->left;

            // Case 1: Uncle is red
            if (uncle != nullptr && uncle->color == RBTreeColor::RED) {
                parent->color = RBTreeColor::BLACK;
                uncle->color = RBTreeColor::BLACK;
                grandparent->color = RBTreeColor::RED;
                node = grandparent; // Move up the tree to check for further violations
            }
            else {
                // Parent is left child of grandparent
                if (parent == grandparent->left) {
                    // Case 2: Node is right child
                    if (node == parent->right) {
                        leftRotation(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    // Case 3: Node is left child
                    rightRotation(grandparent);
                    // Swap colors of parent and grandparent
                    RBTreeColor tempColor = parent->color;
                    parent->color = grandparent->color;
                    grandparent->color = tempColor;
                    node = parent; // The tree is now balanced at this level
                }
                // Parent is right child of grandparent
                else {
                    // Case 2 Mirror: Node is left child
                    if (node == parent->left) {
                        rightRotation(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    // Case 3 Mirror: Node is right child
                    leftRotation(grandparent);
                    // Swap colors of parent and grandparent
                    RBTreeColor tempColor = parent->color;
                    parent->color = grandparent->color;
                    grandparent->color = tempColor;
                    node = parent; // The tree is now balanced at this level
                }
            }
        }
        // Ensure the root is black
        root->color = RBTreeColor::BLACK;
    }

    public:

    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }

        if (v != nullptr) {
            v->parent = u->parent;
        }
    }


    Node* insert(uint32_t key, T& value) {
        // Perform a regular BST insertion
        Node* newNode = bstInsert(key, value, root);

        // Fix red-black tree violations
        fixInsertionViolations(newNode);

        return newNode;
    }

    Node* minimum(Node* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    void fixDeletionViolations(Node* x, Node* xParent) {
        while (x != root && (x == nullptr || x->color == RBTreeColor::BLACK)) {
            if (x == xParent->left) {
                Node* w = xParent->right;

                if (w->color == RBTreeColor::RED) {
                    w->color = RBTreeColor::BLACK;
                    xParent->color = RBTreeColor::RED;
                    leftRotation(xParent);
                    w = xParent->right;
                }

                if ((w->left == nullptr || w->left->color == RBTreeColor::BLACK) &&
                    (w->right == nullptr || w->right->color == RBTreeColor::BLACK)) {
                    w->color = RBTreeColor::RED;
                    x = xParent;
                    xParent = xParent->parent;
                } else {
                    if (w->right == nullptr || w->right->color == RBTreeColor::BLACK) {
                        if (w->left != nullptr)
                            w->left->color = RBTreeColor::BLACK;
                        w->color = RBTreeColor::RED;
                        rightRotation(w);
                        w = xParent->right;
                    }
                    w->color = xParent->color;
                    xParent->color = RBTreeColor::BLACK;
                    if (w->right != nullptr)
                        w->right->color = RBTreeColor::BLACK;
                    leftRotation(xParent);
                    x = root;
                }
            } else { // Mirror image of above code
                Node* w = xParent->left;

                if (w->color == RBTreeColor::RED) {
                    w->color = RBTreeColor::BLACK;
                    xParent->color = RBTreeColor::RED;
                    rightRotation(xParent);
                    w = xParent->left;
                }

                if ((w->right == nullptr || w->right->color == RBTreeColor::BLACK) &&
                    (w->left == nullptr || w->left->color == RBTreeColor::BLACK)) {
                    w->color = RBTreeColor::RED;
                    x = xParent;
                    xParent = xParent->parent;
                } else {
                    if (w->left == nullptr || w->left->color == RBTreeColor::BLACK) {
                        if (w->right != nullptr)
                            w->right->color = RBTreeColor::BLACK;
                        w->color = RBTreeColor::RED;
                        leftRotation(w);
                        w = xParent->left;
                    }
                    w->color = xParent->color;
                    xParent->color = RBTreeColor::BLACK;
                    if (w->left != nullptr)
                        w->left->color = RBTreeColor::BLACK;
                    rightRotation(xParent);
                    x = root;
                }
            }
        }

        if (x != nullptr)
            x->color = RBTreeColor::BLACK;
    }


    T* remove(Node* node) {
        if (node == nullptr)
            throw std::runtime_error("Attempted to remove a null node");

        T* data = &(node->data.front());

        // If there are multiple occurrences, just decrement the count
        if (node->data.size() > 1) {
            node->data.pop_front();
            return data;
        }

        Node* y = node;
        Node* x = nullptr;
        Node* xParent = nullptr;
        RBTreeColor originalColor = y->color;

        // If node has no left child
        if (node->left == nullptr) {
            x = node->right;
            transplant(node, node->right);
            xParent = node->parent;
        }
        // If node has no right child
        else if (node->right == nullptr) {
            x = node->left;
            transplant(node, node->left);
            xParent = node->parent;
        }
        // If node has two children
        else {
            y = minimum(node->right); // Successor
            originalColor = y->color;
            x = y->right;

            if (y->parent == node) {
                if (x != nullptr)
                    x->parent = y;
                xParent = y;
            } else {
                transplant(y, y->right);
                y->right = node->right;
                if (y->right != nullptr)
                    y->right->parent = y;
                xParent = y->parent;
            }

            transplant(node, y);
            y->left = node->left;
            y->left->parent = y;
            y->color = node->color;
        }

        delete node; // Free memory

        if (originalColor == RBTreeColor::BLACK) {
            fixDeletionViolations(x, xParent);
        }

        return data;
    }

    Node* findHelper(Node* node, uint32_t key) {
        if (node == nullptr) return nullptr;
        else if (node->key == key) return node;
        else if (key < node->key) return findHelper(node->left, key);
        return findHelper(node->right, key);
    }

    Node* find(uint32_t key) {
        return findHelper(root, key);
    }

    Node* getSmallestNodeGreaterThan(int minimum) {
        return getSmallestNodeGreaterThanHelper(nullptr, root, minimum);
    }

};