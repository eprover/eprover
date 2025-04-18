#include "clb_arraytrees.h"
#include "clb_simple_stuff.h"

static ArrayTree_p splay_tree(ArrayTree_p tree, long key) {
    fprintf(stdout, "splay_tree -> key: %ld\n", key);

    if (!tree) return NULL;

    ArrayTree_p left, right, tmp;
    ArrayTreeNode newnode; // Dummy root
    long cmpres;

    newnode.lson = NULL;
    newnode.rson = NULL;
    left = &newnode;
    right = &newnode;

    for (;;) {
        cmpres = key - tree->key;

        if (cmpres < 0) {
            if (!tree->lson) break;
            if (key < tree->lson->key) {
                // Zig-Zig (Right Rotation)
                tmp = tree->lson;
                tree->lson = tmp->rson;
                tmp->rson = tree;
                tree = tmp;
                if (!tree->lson) break;
            }
            right->lson = tree;
            right = tree;
            tree = tree->lson;

        } else if (cmpres > 0) {
            if (!tree->rson) break;
            if (key > tree->rson->key) {
                // Zag-Zag (Left Rotation)
                tmp = tree->rson;
                tree->rson = tmp->lson;
                tmp->lson = tree;
                tree = tmp;
                if (!tree->rson) break;
            }
            left->rson = tree;
            left = tree;
            tree = tree->rson;

        } else {
            break;
        }
    }

    // Reassemble
    left->rson = tree->lson;
    right->lson = tree->rson;
    tree->lson = newnode.rson;
    tree->rson = newnode.lson;

    return tree;
}

// FUNCTIONS TREE

// ArrayTreeFree() -> IntMapFree()
// Delete the entire tree (also empty the arrays of all nodes)
// No return value required

void ArrayTreeFree(ArrayTree_p tree) {

    if (tree) {
        PStack_p stack = PStackAlloc();
        PStackPushP(stack, tree);

        while (!PStackEmpty(stack)) {
            tree = PStackPopP(stack);
            if (tree->lson) {
                PStackPushP(stack, tree->lson);
            }
            if (tree->rson) {
                PStackPushP(stack, tree->rson);
            }

            ArrayTreeNodeFree(tree);
        }
        PStackFree(stack);
    }
}

// splay_tree() -> ArrayTreeFind(), ArrayTreeStore(),
//                 ArrayTreeExtractEntry(), ArrayTreeLimitedTraverseInit()
// Search for the node with the corresponding key
// Return of the new root node
// look above

// ArrayTreeNodeFree() -> ArrayTreeExtractEntry()
// Delete single node
// No return value required

void ArrayTreeNodeFree(ArrayTree_p node) {
    uint8_t i;
    // Clean up all entries
    for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        node->entries[i].p_val = NULL;
    }
    ArrayTreeCellFree(node);
}

// ArrayTreeNodeInsert() -> ArrayTreeStore()
// Insert new, single node (if necessary)
// Returns the memory address of the new node

ArrayTree_p ArrayTreeNodeInsert(ArrayTree_p root, long key) {
    ArrayTree_p handle;

    handle = ArrayTreeNodeAllocEmpty();
    // Add handle to tree
    return handle;
}

// ArrayTreeNodeAllocEmpty() -> ArrayTreeNodeInsert()
// Allocate an empty node
// Return the memory address of the new node

ArrayTree_p ArrayTreeNodeAllocEmpty(void) {
    // Allocate the new node
    ArrayTree_p handle;

    handle = ArrayTreeCellAlloc();
    handle->key = -3;
    handle->lson = handle->rson = NULL;
    ArrayInit(handle);
    return handle;
}

// FUNCTIONS ARRAY

// ArrayTreeFind() -> IntMapGetVal(), IntMapGetRef()
// Search for a specific value in the tree
// Return of the p_val of the entry

IntOrP ArrayTreeFind(ArrayTree_p root, long key, uint8_t idx) {
    // Check if key is valid -> use inline function
    root = splay_tree(root, key);
    // Find entry
    return root->entries[idx];
}

// ArrayTreeStore() -> IntMapGetRef()
// Add a value to the array of the corresponding node
// Theoretically does not require a return value

void ArrayTreeStore(ArrayTree_p root, long key, uint8_t idx, IntOrP val) {
    // Check if key is valid -> use inline function
    root = splay_tree(root, key);
    if (root->key == key) {
        if (!root->entries[idx].p_val) {
            root->entries[idx] = val;
        }
    }
}

// ArrayTreeExtractEntry() -> IntMapDelKey()
// Delete the value of a corresponding node
// Return the node (key is needed in IntMapDelKeys())

ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p root, long key) {
    ArrayTree_p cell;

    cell = ArrayTreeNodeAllocEmpty();
    root = splay_tree(root, key);
}

// ArrayTreeLimitedTraverseInit() -> IntMapIterAlloc()
// Return a PStack of all nodes/values that are greater than a limit

// ArrayInit() -> ArrayTreeNodeAlloc()
// Initialize all values of the array

void ArrayInit(ArrayTree_p node) {
    uint8_t i;

    for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        node->entries[i].p_val = NULL;
    }
}
