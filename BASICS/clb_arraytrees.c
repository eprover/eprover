#include "clb_arraytrees.h"
#include "clb_simple_stuff.h"
#include <sys/stat.h>
#include <sys/types.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: splay_tree()
//
//   Perform the splay operation on tree at node with key.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static ArrayTree_p splay_tree(ArrayTree_p tree, long key) {
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


/*-----------------------------------------------------------------------
//
// Function: arraytree_print()
//
//   Print the tree with the appropriate indent level and return the
//   number of nodes.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void arraytree_print(FILE* out, ArrayTree_p tree, bool keys_only, int indent) {
    DStr_p indstr;
    int i;

    // Allocate a dynamic string for indentation
    indstr = DStrAlloc();
    for (i = 0; i < indent; i++) {
        DStrAppendStr(indstr, "  ");
    }

    // Handle empty tree
    if (!tree) {
        fprintf(out, "%s[]\n", DStrView(indstr));
    }
    else {
        // Print all entries in the current node
        for (uint8_t j = 0; j < MAX_NODE_ARRAY_SIZE; j++) {
            if (keys_only) {
                fprintf(out, "%sKey: %ld\n", DStrView(indstr), (tree->key + j));
            }
            else {
                fprintf(out, "%sKey: %ld\n", DStrView(indstr), (tree->key + j));
                fprintf(out, "%s  Val: %ld\n", DStrView(indstr),
                        tree->entries[j].i_val);
            }
        }

        // Print pointers to child nodes if keys_only is false
        if (!keys_only) {
            fprintf(out, "%sLeft child: %p  Right child: %p\n", DStrView(indstr),
                    (void*)tree->lson, (void*)tree->rson);
        }

        // Recursively print left and right subtrees
        if (tree->lson || tree->rson) {
            arraytree_print(out, tree->lson, keys_only, indent + 2);
            arraytree_print(out, tree->rson, keys_only, indent + 2);
        }
    }

    // Free the indentation string
    DStrFree(indstr);
}

/*-----------------------------------------------------------------------
//
// Function: arraytree_print_gv()
//
//   Print the tree with the appropriate indent level and return the
//   number of nodes.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void arraytree_print_gv(FILE* out, ArrayTree_p tree) {
    if (!tree) {
        return;
    }

    // Create node in GrpahViz
    fprintf(out, "    Node%p [label=\"", (void*)tree);
    for (uint8_t j = 0; j < MAX_NODE_ARRAY_SIZE; j++) {
        fprintf(out, "Key: %ld (Val: %ld)\\n", (tree->key + j),
                tree->entries[j].i_val);
    }
    //fprintf(out, "highest_index: %d\\n", tree->highest_index);
    //fprintf(out, "entry_count: %d\\n", tree->entry_count);
    fprintf(out, "\"];\n");

    // If left child exists, create edge
    if (tree->lson) {
        fprintf(out, "    Node%p -> Node%p [label=\"L\"];\n", (void*)tree, (void*)tree->lson);
        arraytree_print_gv(out, tree->lson);
    }

    // If left right exists, create edge
    if (tree->rson) {
        fprintf(out, "    Node%p -> Node%p [label=\"R\"];\n", (void*)tree, (void*)tree->rson);
        arraytree_print_gv(out, tree->rson);
    }
}

/*-----------------------------------------------------------------------
//
// Function: array_init()
//
//   Set all entries of the array of the given node to NULL
//
// Global Variables: -
//
// Side Effects    : Inits node
//
/----------------------------------------------------------------------*/

void array_init(ArrayTree_p node) {
    uint8_t i;

    for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        node->entries[i].p_val = NULL;
    }
}

/*-----------------------------------------------------------------------
//
// Function: clear_all_entries()
//
//   Reset all entries of the array of the given node to NULL
//
// Global Variables: -
//
// Side Effects    : Removes all values
//
/----------------------------------------------------------------------*/

#define clear_all_entries(node) array_init(node)

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDebugPrint()
//
//   Print the tree in an unattractive but debug-friendly way.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only) {
    fprintf(out, "root: %p\n", tree);
    arraytree_print(out, tree, keys_only, 0);
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreePrintGV()
//
//   Print the tree in an attractive and debug-friendly way via GraphViz.
//   Make sure the application is installed. The printed files can be
//   found in ../PROVER/trees/
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ArrayTreePrintGV(ArrayTree_p tree) {
    // Create target directory, if not existing yet
    char label[64];
    snprintf(label, sizeof(label), "tree_%p", (void*)tree);
    const char* dir_name = "trees";
    struct stat st = {0};
    if (stat(dir_name, &st) == -1) {
        mkdir(dir_name, 0777);  // Create folder with permission r/w
    }

    // Create filepath: "trees/filename"
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s.gv", dir_name, label);

    FILE* out = fopen(filepath, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
        return;
    }

    // Write GraphViz Header
    fprintf(out, "digraph ArrayTree {\n");
    fprintf(out, "    node [shape=record, style=filled, fillcolor=lightgrey];\n");

    // Recursive writing of all nodes
    arraytree_print_gv(out, tree);

    // Finalize Graph
    fprintf(out, "}\n");
    fclose(out);

    // Generate PNG file from printed output
    char command[512];
    snprintf(command, sizeof(command), "dot -Tpng %s -o %s/%s.png", filepath, dir_name, label);

    int result = system(command);  // Run shell command
    if (result != 0) {
        fprintf(stderr, "Error: GraphViz command failed.\n");
    }
}

/*-----------------------------------------------------------------------
//
//  Tree-related functions
//
/----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeFree()
//
//   Free an arraytree (including all entries, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

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
            clear_all_entries(tree);
            ArrayTreeCellFree(tree);
            tree = NULL;
        }
        PStackFree(stack);
    }
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeNodeFree()
//
//   Free a single node of an arraytree (including all entries, but not
//   potential objects pointed to in the val fields. It extracts the
//   respective node and reconstructs the arraytree
//
// Global Variables: -
//
// Side Effects    : Memory operations and changes the tree
//
/----------------------------------------------------------------------*/

void ArrayTreeNodeFree(ArrayTree_p *root, long key) {
    if (!root || !(*root)) return;

    key = CalcKey(key);
    *root = splay_tree(*root, key);
    if ((*root)->key != key) return;

    ArrayTree_p temp;

    if (!(*root)->lson) {
        temp = (*root)->rson;
    } else {
        temp = splay_tree((*root)->lson, key);
        temp->rson = (*root)->rson;
    }

    clear_all_entries(*root);
    ArrayTreeCellFree(*root);
    *root = temp;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeNodeInsert()
//
//   If necessary add a new node to the tree
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeNodeInsert(ArrayTree_p root, long key) {
    ArrayTree_p handle;

    handle = ArrayTreeNodeAllocEmpty();
    handle->key = CalcKey(key);

    if (!root) return handle;

    root = splay_tree(root, handle->key);
    if (root->key == handle->key) {
        ArrayTreeCellFree(handle);
        return root;
    }

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

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeNodeAllocEmpty()
//
//   Allocate a empty, initialized ArrayTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeNodeAllocEmpty(void) {
    // Allocate the new node
    ArrayTree_p handle;

    handle = ArrayTreeCellAlloc();
    handle->lson = handle->rson = NULL;
    array_init(handle);
    return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeFind()
//
//   Find the node containing key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key) {
    long nodeKey;

    if(*root) {
        nodeKey = CalcKey(key);
        *root = splay_tree(*root, nodeKey);
        if((*root)->key == nodeKey) {
            return *root;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------
//
//  Array-related functions
//
/----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeGetEntry()
//
//   Find the entry inside the respective node and return its value
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void* ArrayTreeGetEntry(ArrayTree_p node, long key) {
    uint8_t idx;

    idx = CalcIdx(key, node->key);
    return node->entries[idx].p_val;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeGetEntryRef()
//
//   Find the entry inside the respective node and return the adress of
//   its value
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void** ArrayTreeGetEntryRef(ArrayTree_p *node, long key) {
    uint8_t idx;

    idx = CalcIdx(key, (*node)->key);
    return &(*node)->entries[idx].p_val;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeCheckEmpty()
//
//   Check if an array of a given node is completely empty
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ArrayTreeCheckEmpty(ArrayTree_p root, long key) {
    long nodeKey = CalcKey(key);

    if (!root) return true;

    if (!root || root->key != nodeKey) {
        return true;
    }

    for (int i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        if (root->entries[i].p_val != NULL) {
            return false;
        }
    }

    return true;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeStore()
//
//   Insert a node associating key with val into the tree. If necessary
//   insert a new node aswell.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val) {
    long nodeKey, idx;

    nodeKey = CalcKey(key);

    if (!(*root)) {
        *root = ArrayTreeNodeInsert(NULL, nodeKey);
    } else {
        *root = splay_tree(*root, nodeKey);
        if ((*root)->key != nodeKey) {
            *root = ArrayTreeNodeInsert(*root, nodeKey);
        }
    }

    nodeKey = CalcKey(key);
    idx = key - nodeKey;
    if (!(*root)->entries[idx].p_val) {
        (*root)->entries[idx] = val;
    }

    return *root;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeExtractEntry()
//
//   Find the entry with key key, remove it from the array, rebalance
//   the tree if node is empty, and return the pointer to the removed
//   value. Return NULL if no matching element exists.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

void* ArrayTreeExtractEntry(ArrayTree_p *node, long key) {
    long idx;
    void* val = NULL;
    
    if (!(*node)) return NULL;

    idx = CalcIdx(key, (*node)->key);

    if ((*node)->entries[idx].p_val) {
        val = (*node)->entries[idx].p_val;
        (*node)->entries[idx].p_val = NULL;
    }

    if (ArrayTreeCheckEmpty(*node, key)) {
        ArrayTreeNodeFree(&(*node), key);
    }

    return val;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeLimitedTraverseInit()
//
//   Return a stack containing the path to the smallest element
//   smaller than or equal to limit in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TreeIter_p ArrayTreeLimitedTraverseInit(ArrayTree_p root,
                                        TreeIter_p *iterator, long limit) {
    PStack_p stack = PStackAlloc();
    ArrayTree_p handle;
    (*iterator)->idx = 0;
    (*iterator)->lower_bound = limit;
    (*iterator)->node = NULL;

    while(root) {
        if(root->key<limit) {
            if ((root->key + MAX_NODE_ARRAY_SIZE - 1) >= limit) {
                PStackPushP(stack, root);
                root = root->lson;
            } else {
                root = root->rson;
            }
        } else {
            PStackPushP(stack, root);
            if(root->key == limit) {
                root = NULL;
            } else {
                root = root->lson;
            }
        }
    }

    // Assign last pushed node to the iterator
    if (stack->current > 0) {
        (*iterator)->node = stack->stack[--stack->current].p_val;
    }
    handle = stack->stack[0].p_val;
    for (uint8_t i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        if (handle->entries[i].p_val) {
            // first match is new limit
            (*iterator)->upper_bound = handle->key + i;
            break;
        }
    }
    (*iterator)->tree_iter = stack;
    return *iterator;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeMaxKey()
//
//   Return the the largest key in the tree (or NULL if tree is empty).
//   Non-destructive/non-reorganizing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ArrayTreeMaxKey(ArrayTree_p root) {
    int i;

    if(root)
    {
        while(root->rson)
        {
            root = root->rson;
        }
    }

    for (i = MAX_NODE_ARRAY_SIZE - 1; i >= 0; i--) {
        if (root->entries[i].p_val) break;
    }
    if (i < 0) return root->key;
    return (root->key + i);
}

AVL_TRAVERSE_DEFINITION(ArrayTree, ArrayTree_p)