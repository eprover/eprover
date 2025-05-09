#include "clb_arraytrees.h"
#include "clb_simple_stuff.h"

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


// ArrayInit() -> ArrayTreeNodeAlloc()
// Initialize all values of the array

void array_init(ArrayTree_p node) {
    printf("array_init\n");
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
    printf("limited_entry_find -> limit: %ld\n", limit);
    // ArrayTree_p cell;
    uint8_t i;

    i = MAX_NODE_ARRAY_SIZE;
    while ((root->key + i) >= limit) {
        // cell->entries[i] = root->entries[i];
        i--;
    }
    return i;
}

void clear_all_entries(ArrayTree_p node) {
    printf("clear_all_entries\n");
    uint8_t i;
    // Clean up all entries
    for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
        node->entries[i].p_val = NULL;
    }
}

uint8_t get_highest_index(ArrayTree_p root) {
    printf("get_highest_index\n");
    uint8_t i;

    for (i = MAX_NODE_ARRAY_SIZE - 1; i >= 0; i--) {
        if (root->entries[i].p_val) break;
    }
    printf("highest idx: %i\n", i);
    return i;
}

int readInteger() {
    int number;
    printf("Bitte geben Sie den key ein: ");
    scanf("%d", &number);
    while (getchar() != '\n');
    return number;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

void TreeDebug() {
    ArrayTree_p root = NULL;
    char input;
    int running = 1, key;
    bool empty;

    while (running) {
        printf("\nGeben Sie ein Zeichen ein (c, f, i, n, p, s oder z zum Beenden):\n"
               "c: free, e: checkEmpty, f: find, i: insert, n: nodeFree, p: print, s: splay, z: quit\n"
               "Eingabe: ");
        scanf("%c", &input);
        while (getchar() != '\n');

        switch (input) {
            case 'e':   // ArrayTreeCheckEmpty
                key = readInteger();
                empty = ArrayTreeCheckEmpty(root, key);
                printf("Node %d is %s\n", key, empty ? "empty" : "not empty");
                break;
            case 'f':   // ArrayTreeFind
                key = readInteger();
                root = ArrayTreeFind(&root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'c':   // ArrayTreeFree
                ArrayTreeFree(&root);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'n':
                key = readInteger();
                ArrayTreeNodeFree(&root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'i':
                key = readInteger();
                root = ArrayTreeNodeInsert(root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'p':   // ArrayTreeDebugPrint
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 's':    // splay_tree
                key = readInteger();
                root = splay_tree(root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'z':
                running = 0;
                printf("Programm wird beendet.\n");
                break;
            default:
                printf("Ungültige Eingabe, bitte erneut versuchen.\n");
                break;
        }
    }
}


void ArrayDebug() {
    ArrayTree_p root = NULL, handle = NULL;
    IntOrP val;
    char input;
    int running = 1, key, nodeKey;
    long maxKey;
    TreeIter_p iter = malloc(sizeof(ArrayTreeIter));
    void **ref;
    void *entry;

    while (running) {
        printf("\nGeben Sie ein Zeichen ein:\n"
               "e: extract, g: get, i: insert, l: traverseLimit, m: maxKey, p: print, r: getRef, z: quit\n"
               "Eingabe: ");
        scanf(" %c", &input);
        while (getchar() != '\n');

        switch (input) {
            case 'i':   // ArrayTreeStore
                key = readInteger();
                val.p_val = 0x400;
                ArrayTreeStore(&root, key, val);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'g':   // ArrayTreeGetEntry
                key = readInteger();
                nodeKey = CalcKey(key);
                root = splay_tree(root, nodeKey);
                entry = ArrayTreeGetEntry(root, key);
                printf("entry: %p\n", entry);
                break;
            case 'r':   // ArrayTreeGetEntryRef
                key = readInteger();
                nodeKey = CalcKey(key);
                root = splay_tree(root, nodeKey);
                ref = ArrayTreeGetEntryRef(&root, key);
                printf("ref: %p\n", ref);
                break;
            case 'e':   // ArrayTreeExtractEntry
                key = readInteger();
                nodeKey = CalcKey(key);
                root = splay_tree(root, nodeKey);
                entry = ArrayTreeExtractEntry(&root, key);
                printf("tree:\n");
                ArrayTreeDebugPrint(stdout, root, false);
                printf("entry: %p\n", entry);
                break;
            case 'm':   // ArrayTreeMaxKey
                maxKey = ArrayTreeMaxKey(root);
                printf("maxKey: %ld\n", maxKey);
                break;
            case 'l':   // ArrayTreeLimitedTraverseInit
                key = readInteger();
                ArrayTreeLimitedTraverseInit(root, &iter, key);
                printf("limit: %ld\n", iter->limit);
                while ((handle = ArrayTreeTraverseNext(iter->tree_iter))) {
                    ArrayTreeDebugPrint(stdout, handle, false);
                }
                break;
            case 'p':   // ArrayTreeDebugPrint
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'z':
                running = 0;
                printf("Programm wird beendet.\n");
                break;
            default:
                printf("Ungültige Eingabe.\n");
                break;
        }
    }
    free(iter);
}


// FUNCTIONS TREE

void ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only) {
    fprintf(out, "root: %p\n", tree);
    arraytree_print(out, tree, keys_only, 0);
}


// ArrayTreeFree() -> IntMapFree()
// Delete the entire tree (also empty the arrays of all nodes)
// No return value required

void ArrayTreeFree(ArrayTree_p *tree) {
    printf("ArrayTreeFree\n");

    if (*tree) {
        PStack_p stack = PStackAlloc();
        PStackPushP(stack, *tree);

        while (!PStackEmpty(stack)) {
            *tree = PStackPopP(stack);
            if ((*tree)->lson) {
                PStackPushP(stack, (*tree)->lson);
            }
            if ((*tree)->rson) {
                PStackPushP(stack, (*tree)->rson);
            }
            clear_all_entries(*tree);
            ArrayTreeCellFree(*tree);
            *tree = NULL;
        }
        PStackFree(stack);
    }
}

void ArrayTreeNodeFree(ArrayTree_p *root, long key) {
    printf("ArrayTreeNodeFree -> key: %ld\n", key);
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


// ArrayTreeNodeInsert() -> ArrayTreeStore()
// Insert new, single node (if necessary)
// Returns the memory address of the new node

ArrayTree_p ArrayTreeNodeInsert(ArrayTree_p root, long key) {
    printf("ArrayTreeNodeInsert -> key: %ld\n", key);
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


// ArrayTreeNodeAllocEmpty() -> ArrayTreeNodeInsert()
// Allocate an empty node
// Return the memory address of the new node

ArrayTree_p ArrayTreeNodeAllocEmpty(void) {
    printf("ArrayTreeNodeAllocEmpty\n");
    // Allocate the new node
    ArrayTree_p handle;

    handle = ArrayTreeCellAlloc();
    handle->lson = handle->rson = NULL;
    array_init(handle);
    return handle;
}

// ArrayTreeFind() -> IntMapGetVal(), IntMapGetRef()
// Search for a specific value in the tree
// Return of the p_val of the entry

ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key) {
    printf("ArrayTreeFind -> key: %ld\n", key);
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

// FUNCTIONS ARRAY

// ArrayTreeGetEntry() -> IntMapGetVal()
// Find Entry of given node
// return IntOrP

void* ArrayTreeGetEntry(ArrayTree_p node, long key) {
    printf("ArrayTreeGetEntry -> key: %ld\n", key);
    uint8_t idx;

    idx = CalcIdx(key, node->key);
    return node->entries[idx].p_val;
}

void** ArrayTreeGetEntryRef(ArrayTree_p *node, long key) {
    printf("ArrayTreeGetEntryRef -> key: %ld\n", key);
    uint8_t idx;

    idx = CalcIdx(key, (*node)->key);
    return &(*node)->entries[idx].p_val;
}

bool ArrayTreeCheckEmpty(ArrayTree_p root, long key) {
    printf("ArrayTreeCheckEmpty -> key: %ld\n", key);
    long nodeKey = CalcKey(key);

    if (!root) return true;

    root = splay_tree(root, nodeKey);

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



// ArrayTreeStore() -> IntMapGetRef(), add_new_tree_node()
// Add a value to the array of the corresponding node
// Theoretically does not require a return value

ArrayTree_p ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val) {
    printf("ArrayTreeStore -> key: %ld, val: %p\n", key, val);
    long nodeKey, idx;

    nodeKey = CalcKey(key);
    idx = key - nodeKey;
    assert(idx >= 0 && idx < MAX_NODE_ARRAY_SIZE);

    if (!(*root)) {
        *root = ArrayTreeNodeInsert(NULL, nodeKey);
    } else {
        *root = splay_tree(*root, nodeKey);
        if ((*root)->key != nodeKey) {
            *root = ArrayTreeNodeInsert(*root, nodeKey);
        }
    }

    if (!(*root)->entries[idx].p_val) {
        (*root)->entries[idx] = val;
    }

    return *root;
}



// ArrayTreeExtractEntry() -> IntMapDelKey()
// Delete the value of a corresponding node
// Return value, the key of the node is necessary but given inside IntMapDelKey()

void* ArrayTreeExtractEntry(ArrayTree_p *node, long key) {
    printf("ArrayTreeExtractEntry -> key: %ld\n", key);
    long idx;
    void* val = NULL;
    
    if (!(*node)) return NULL;

    idx = key - (*node)->key;
    assert(idx > -1 && idx < MAX_NODE_ARRAY_SIZE);

    if ((*node)->entries[idx].p_val) {
        val = (*node)->entries[idx].p_val;
        (*node)->entries[idx].p_val = NULL;
    }

    if (ArrayTreeCheckEmpty(*node, key)) {
        ArrayTreeNodeFree(&(*node), key);
    }

    return val;
}

// ArrayTreeLimitedTraverseInit() -> IntMapIterAlloc()
// Return a PStack of all nodes/values that are greater than a limit

TreeIter_p ArrayTreeLimitedTraverseInit(ArrayTree_p root,
                                        TreeIter_p *iterator, long limit) {
    printf("ArrayTreeLimitedTraverseInit -> limit: %ld\n", limit);
    PStack_p stack = PStackAlloc();
    uint8_t  maxIdx;

    while (root) {
        maxIdx = get_highest_index(root);
        if ((root->key + maxIdx) < limit) {
            root = root->rson;
        } else {
            PStackPushP(stack, root);

            if (root->key > limit) {
                root = root->lson;
            } else if (root->key == limit) {
                root = NULL;
            } else if (root->key < limit && (root->key + maxIdx) >= limit) { 
                //idx = limited_entry_find(root, limit);
                root = NULL;
            }
        }
    }

    (*iterator)->limit = limit;
    (*iterator)->tree_iter = stack;
    return *iterator;
}


// ArrayTreeMaxNode() -> IntMapDelKey()
// Return the highest existing key

long ArrayTreeMaxKey(ArrayTree_p root) {
    printf("ArrayTreeMaxNode\n");
    uint8_t i;

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
    
    return (root->key + i);
}

AVL_TRAVERSE_DEFINITION(ArrayTree, ArrayTree_p)