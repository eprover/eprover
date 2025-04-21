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

// ArrayInit() -> ArrayTreeNodeAlloc()
// Initialize all values of the array

void array_init(ArrayTree_p node) {
    uint8_t i;

    for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        node->entries[i].p_val = NULL;
    }
}

// limited_entry_clear() -> ArrayTreeLimitedTraverseInit()
// returns node whitout entries below limit
// Which functions call ArrayTreeLimitedTraverseInit()?
//      cell needs to be freed!

uint8_t limited_entry_find(ArrayTree_p root, long limit) {
    // ArrayTree_p cell;
    uint8_t i;

    // cell = ArrayTreeNodeAllocEmpty();
    // cell->key = root->key;
    // cell->highest_index = root->highest_index;
    // cell->lson = root->lson;
    // cell->rson = root->rson;

    i = root->highest_index;
    while ((root->key + i) >= limit) {
        // cell->entries[i] = root->entries[i];
        i--;
    }
    return i;
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
    for (i = 0; i <= node->highest_index; i++) {
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
    handle->key = CalcKey(key);

    root = splay_tree(root, handle->key);
    if (root->key == handle->key) {
        ArrayTreeCellFree(handle);
        return root;
    }
    printf("root->key: %ld, key: %ld\n", root->key, key);
    if (handle->key < root->key) {
        handle->lson = root->lson;
        handle->rson = root;
        root->lson = NULL;
    } else {
        handle->rson = root->rson;
        handle->lson = root;
        root->rson = NULL;
    }
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
    array_init(handle);
    handle->highest_index = -1;
    return handle;
}

// FUNCTIONS ARRAY

// ArrayTreeFind() -> IntMapGetVal(), IntMapGetRef()
// Search for a specific value in the tree
// Return of the p_val of the entry

IntOrP ArrayTreeFind(ArrayTree_p root, long key) {
    IntOrP val;
    long nodeKey, idx;
    // Check if key is valid -> use inline function
    nodeKey = CalcKey(key);
    idx = key - nodeKey;
    root = splay_tree(root, key);
    // Find entry
    if (root->key == nodeKey && root->entries[idx].p_val) {
        return root->entries[idx];
    } else {
        val.p_val = NULL;
        return val;
    }
}

void** ArrayTreeFindRef(ArrayTree_p root, long key) {
    long nodeKey, idx;

    // Check if key is valid -> use inline function
    nodeKey = CalcKey(key);
    idx = key - nodeKey;

    root = splay_tree(root, nodeKey);

    // Find entry
    if (root && root->key == nodeKey) {
        return &(root->entries[idx].p_val);
    }

    return NULL;
}


// ArrayTreeStore() -> IntMapGetRef(), add_new_tree_node()
// Add a value to the array of the corresponding node
// Theoretically does not require a return value

IntOrP ArrayTreeStore(ArrayTree_p root, long key, IntOrP val) {
    long nodeKey, idx;
    // Check if key is valid -> use inline function
    nodeKey = CalcKey(key);
    idx = key - nodeKey;
    root = splay_tree(root, key);
    if (root->key == key) {
        if (!root->entries[idx].p_val) {
            root->entries[idx] = val;
            if (root->highest_index < idx) root->highest_index = idx;
        }
    } else {
        ArrayTreeNodeInsert(root, nodeKey);
        ArrayTreeStore(root, key, val);
    }
    return val;
}

// ArrayTreeExtractEntry() -> IntMapDelKey()
// Delete the value of a corresponding node
// Return value, the key of the node is necessary but given inside IntMapDelKey()

IntOrP ArrayTreeExtractEntry(ArrayTree_p root, long key) {
    IntOrP val;
    long nodeKey, idx;

    nodeKey = CalcKey(key);
    idx = key - nodeKey;
    root = splay_tree(root, nodeKey);

    if (root->key == nodeKey && root->entries[idx].p_val) {
        val = root->entries[idx];
        root->entries[idx].p_val = NULL;
        if (root->highest_index == idx) {
            for (idx = root->highest_index; idx >= -1; idx --) {
                if (root->entries[idx].p_val) break;
            }
            root->highest_index = idx;
        }
        // Optional: Check if node is empty
        if (root->highest_index < 0) ArrayTreeNodeFree(root);
        return val;
    } else {
        val.p_val = NULL;
        return val;
    }
}

// ArrayTreeLimitedTraverseInit() -> IntMapIterAlloc()
// Return a PStack of all nodes/values that are greater than a limit

TreeIter_p ArrayTreeLimitedTraverseInit(ArrayTree_p root,
                                      TreeIter_p iterator, long limit) {
    long     key = 0;
    PStack_p stack = PStackAlloc();

    while(root) {
        if ((root->key + root->highest_index) < limit) {
            root = root->rson;
        } else {
            PStackPushP(stack, root);
            if (root->key > limit) {
                root = root->lson;
            }
            if (root->key == limit) {
                root = NULL;
            }
            if (root->key < limit) {
                key = limited_entry_find(root, limit);
                root = NULL;
            }
        }
    }
    iterator->key = key;
    iterator->tree_iter = stack;
    return iterator;
}

// ArrayTreeMaxNode() -> IntMapDelKey()
// Return the highest existing key

long ArrayTreeMaxNode(ArrayTree_p root)
{
   if(root)
   {
      while(root->rson)
      {
         root = root->rson;
      }
   }
   return (root->key + root->highest_index);
}

AVL_TRAVERSE_DEFINITION(ArrayTree, ArrayTree_p)