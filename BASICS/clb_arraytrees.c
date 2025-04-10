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

static ArrayTree_p splay_tree(ArrayTree_p tree, long key) {
    fprintf(stdout, "splay_tree -> key: %ld\n", key);
    ArrayTree_p left, right, tmp;
    ArrayTreeNode newnode; // Placeholder node for tree manipulation
    long cmpres;

    // Return if the tree is empty
    if (!tree) {
        return tree;
    }

    // Initialize the temporary nodes
    newnode.lson = NULL;
    newnode.rson = NULL;
    left = &newnode;
    right = &newnode;

    for (;;) {
        // Navigate left or right based on the first or last key in the node
        cmpres = KeyCmp(key, tree->key);
        if (CmpLessVal(cmpres, 0)) {
            // Key is smaller, move to the left subtree
            if (!tree->lson) {
                break;
            }
            if (key < tree->lson->key) {
                // Perform a right rotation
                tmp = tree->lson;
                tree->lson = tmp->rson;
                tmp->rson = tree;
                tree = tmp;
                if (!tree->lson) {
                    break;
                }
            }
            right->lson = tree;
            right = tree;
            tree = tree->lson;
        }
        else if (CmpGreaterEqual(cmpres, MAX_NODE_ARRAY_SIZE)) {
            if (!tree->rson) {
                // Tree is already shifted to the most right node
                break;
            }
            // Key is larger, move to the right subtree
            if (CmpGreaterEqual(key, (tree->rson->key + MAX_NODE_ARRAY_SIZE))) {
                // Perform a left rotation
                tmp = tree->rson;
                tree->rson = tmp->lson;
                tmp->lson = tree;
                tree = tmp;
                if (!tree->rson) {
                    break;
                }
            }
            left->rson = tree;
            left = tree;
            tree = tree->rson;
        }
        else {
            break;
        }
    }

    // Reassemble the tree
    left->rson = tree->lson;
    right->lson = tree->rson;
    tree->lson = newnode.rson;
    tree->rson = newnode.lson;

    return tree;
}

static long arraytree_print(FILE* out, ArrayTree_p tree, bool keys_only, int indent) {
    DStr_p indstr;
    int i, size;

    // Allocate a dynamic string for indentation
    indstr = DStrAlloc();
    for (i = 0; i < indent; i++) {
        DStrAppendStr(indstr, "  ");
    }

    // Handle empty tree
    if (!tree) {
        fprintf(out, "%s[]\n", DStrView(indstr));
        size = 0;
    }
    else {
        // Print all entries in the current node
        for (uint8_t j = 0; j <= tree->highest_index; j++) {
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

        // Initialize size counter
        size = tree->entry_count;

        // Recursively print left and right subtrees
        if (tree->lson || tree->rson) {
            size += arraytree_print(out, tree->lson, keys_only, indent + 2);
            size += arraytree_print(out, tree->rson, keys_only, indent + 2);
        }
    }

    // Free the indentation string
    DStrFree(indstr);

    return size;
}

static void arraytree_print_gv(FILE* out, ArrayTree_p tree) {
    if (!tree) {
        return;
    }

    // Create node in GrpahViz
    fprintf(out, "    Node%p [label=\"", (void*)tree);
    for (uint8_t j = 0; j <= tree->highest_index; j++) {
        fprintf(out, "Key: %ld (Val: %ld)\\n", (tree->key + j),
                tree->entries[j].i_val);
    }
    fprintf(out, "highest_index: %d\\n", tree->highest_index);
    fprintf(out, "entry_count: %d\\n", tree->entry_count);
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

ArrayTree_p ArrayTreeNodeAllocEmpty(void) {
    fprintf(stdout, "ArrayTreeNodeAllocEmpty\n");
    // Allocate memory for a new ArrayTree node
    ArrayTree_p handle = ArrayTreeNodeAlloc();

    handle->key = -3;            // Valid keys: [-2; LONG_MAX]
    for (uint8_t i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
       assert(i<MAX_NODE_ARRAY_SIZE);
       handle->entries[i].p_val = NULL;
    }

    // Initialize metadata
    handle->highest_index = 0;       // No entries initially
    handle->entry_count = 0;         // All entries are not valid

    // Initialize child pointers
    handle->lson = NULL;
    handle->rson = NULL;

    return handle;
}

void ArrayTreeFree(ArrayTree_p junk) {
    fprintf(stdout, "ArrayTreeFree\n");
    if (junk) {
        // Allocate a stack for iterative traversal of the tree
        PStack_p stack = PStackAlloc();

        // Push the root node onto the stack
        PStackPushP(stack, junk);

        // Process nodes iteratively until the stack is empty
        while (!PStackEmpty(stack)) {
            // Pop a node from the stack
            junk = PStackPopP(stack);

            // If the left child exists, push it onto the stack
            if (junk->lson) {
                PStackPushP(stack, junk->lson);
            }

            // If the right child exists, push it onto the stack
            if (junk->rson) {
                PStackPushP(stack, junk->rson);
            }
            ArrayTreeNodeFree(junk);
        }

        // Free the stack used for traversal
        PStackFree(stack);
    }
}

ArrayTree_p ArrayTreeInsert(ArrayTree_p *root, ArrayTree_p newnode, long idx) {
    fprintf(stdout, "ArrayTreeInsert -> newnode->key: %ld\n", newnode->key);
    long diff, nodeKey, validIdx;

    // Check if the value is inserted in the correct place
    // There are a few cases where a value is inserted in the wrong place
    // ex: node is inserted by add_new_tree_node (clb_intmap.c)
    //     extracted node is inserted again
    //     functions from other folders insert into the tree (compare annotations etc)
    nodeKey = CalcKey(newnode->key);
    validIdx = KeyCmp(newnode->key, nodeKey);
    if (!(nodeKey == newnode->key && validIdx == idx)) {
        newnode->key = nodeKey;
        newnode->entries[validIdx] = newnode->entries[idx];
        newnode->entries[idx].p_val = NULL;
    }

    // If the tree is empty, make the new node the root
    if (!*root) {
        newnode->lson = newnode->rson = NULL;
        *root = newnode;
        return NULL;
    }

    // Splay the tree to bring the closest key to the root
    *root = splay_tree(*root, newnode->key);
    // Check key of the current tree
    diff = KeyCmp(newnode->key, (*root)->key);

    // Node exists
    if (CmpEqual(idx, diff)) {
        // Check if entry exists
        if ((*root)->entries[idx].p_val) {
            return newnode;
        } else {
            (*root)->entries[idx] = newnode->entries[idx];
            if (CmpLessVal((*root)->highest_index, idx)) (*root)->highest_index = idx;
            ArrayTreeNodeFree(newnode);
            (*root)->entry_count++;
            return NULL;
        }
    // Node is not existing
    } else if (CmpLessVal(diff, 0)) {
        // Add entry in left subtree
        newnode->lson = (*root)->lson;
        newnode->rson = *root;
        (*root)->lson = NULL;
        *root = newnode;
        return NULL;
    } else if (CmpGreaterEqual(diff, MAX_NODE_ARRAY_SIZE)) {
        // Add entry in right subtree
        newnode->rson = (*root)->rson;
        newnode->lson = *root;
        (*root)->rson = NULL;
        *root = newnode;
        return NULL;
    } else {
        assert(newnode && "Out of memory in ArrayTreeNodeAllocEmpty");
        return newnode;
    }
}

bool ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val) {
    fprintf(stdout, "ArrayTreeStore -> key: %ld\n", key);
    long idx;
    // Allocate a new node for the key-value pair
    ArrayTree_p handle = ArrayTreeNodeAllocEmpty();
    assert(handle && "Out of memory in ArrayTreeNodeAllocEmpty");

    // Initialize the first entry in the new node
    handle->key = CalcKey(key);
    idx = KeyCmp(key, handle->key);
    handle->entries[idx] = val;

    // Initialize metadata for the new node
    handle->entry_count = 1;        // Only one entry initially
    handle->highest_index = idx;
    handle->lson = NULL;
    handle->rson = NULL;

    // Try to insert the node into the tree
    ArrayTree_p newnode = ArrayTreeInsert(root, handle, idx);

    if (newnode) {
        // If the key already exists, free the allocated node and return false
        ArrayTreeNodeFree(handle);
        return false;
    }
    return true;
}

void ArrayTreeDebug() {
    ArrayTree_p root = NULL, ext = NULL;
    IntOrP val;
    char input;
    int running = 1, key;

    while (running) {
        printf("\nGeben Sie ein Zeichen ein (c, d, e, f, l, i, p, r, s, p oder z zum Beenden):\nc: free, d: delete, e: extract, f: find, i: store, l: traverseLimit, p: print, r: root, s: splay, t: split, z: quit\nEingabe: ");
        scanf("%c", &input);
        while (getchar() != '\n');

        switch (input) {
            case 'f':   // ArrayTreeFind
                key = readInteger();
                root = ArrayTreeFind(&root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'd':   // ArrayTreeDeleteNode
                key = readInteger();
                ArrayTreeDeleteEntry(&root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'i':   // ArrayTreeStore
                key = readInteger();
                val.i_val = key;
                ArrayTreeStore(&root, key, val);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'r':   // ArrayTreeExtractRoot
                ext = ArrayTreeExtractRoot(&root);
                ArrayTreeDebugPrint(stdout, root, false);
                printf("extracted:\n");
                ArrayTreeDebugPrint(stdout, ext, false);
                break;
            case 'c':   // ArrayTreeFree
                ArrayTreeFree(root);
                root = NULL;
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'l':   // ArrayTreeTraverseLimit
                key = readInteger();
                ArrayTreeLimitedTraverseInit(root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                break;
            case 'e':   // ArrayTreeExtractNode
                key = readInteger();
                ext = ArrayTreeExtractEntry(&root, key);
                ArrayTreeDebugPrint(stdout, root, false);
                printf("extracted:\n");
                ArrayTreeDebugPrint(stdout, ext, false);
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

long ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only) {
    long size;
    fprintf(out, "root: %p\n", tree);
    size = arraytree_print(out, tree, keys_only, 0);
    fprintf(out, "Tree size: %ld\n", size);
    return size;
}

void ArrayTreePrintGV(ArrayTree_p tree, const char* filename) {
    fprintf(stdout, "ArrayTreePrintGV\n");
    // Create target directory, if not existing yet
    const char* dir_name = "trees";
    struct stat st = {0};
    if (stat(dir_name, &st) == -1) {
        mkdir(dir_name, 0777);  // Create folder with permission r/w
    }

    // Create filepath: "trees/filename"
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s.gv", dir_name, filename);

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
    snprintf(command, sizeof(command), "dot -Tpng %s -o %s/%s.png", filepath, dir_name, filename);

    int result = system(command);  // Run shell command
    if (result != 0) {
        fprintf(stderr, "Error: GraphViz command failed.\n");
    }
}

ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key) {
    fprintf(stdout, "ArrayTreeFind -> key: %ld\n", key);
    long nodeKey, idx;
    ArrayTree_p handle;

    // Check if the tree is empty
    if (*root) {
        // Perform the splay operation to bring the closest key to the root
        nodeKey = CalcKey(key);
        *root = splay_tree(*root, nodeKey);
        idx = KeyCmp(key, (*root)->key);
        assert(idx<MAX_NODE_ARRAY_SIZE);
        if (CmpGreaterEqual(idx, MAX_NODE_ARRAY_SIZE) || CmpLessVal(idx, 0)) {
            return NULL;
        }
        if ((*root)->entries[idx].p_val) {
            return handle;
        }
    }

    // Key not found
    return NULL;
}

ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p *root, long key) {
    fprintf(stdout, "ArrayTreeExtractEntry -> key: %ld\n", key);
    ArrayTree_p x = NULL, cell, handle;
    long idx, nodeKey;
    uint8_t i = 0;

    // Return NULL if the tree is empty
    if (!(*root)) {
        return NULL;
    }

    // Rework function here
    nodeKey = CalcKey(key);
    *root = splay_tree(*root, nodeKey);
    idx = KeyCmp(key, (*root)->key);
    assert(idx<MAX_NODE_ARRAY_SIZE);
    if (CmpGreaterEqual(idx, MAX_NODE_ARRAY_SIZE) || CmpLessVal(idx, 0)) {
        return NULL;
    }

    // Check if entry is valid
    if ((*root)->entries[idx].p_val) {
        cell = ArrayTreeNodeAllocEmpty();
        cell->key = nodeKey;
        cell->entries[idx] = (*root)->entries[idx];
        cell->entry_count++;
        cell->highest_index = idx;
        cell->lson = cell->rson = NULL;

        // Check if original node is empty
        (*root)->entry_count--;
        if (CmpEqual((*root)->entry_count, 0)) {
            // Reorganize the tree if the root is empty
            if (!(*root)->lson) {
                x = (*root)->rson;
            }
            else {
                x = splay_tree((*root)->lson, key);
                x->rson = (*root)->rson;
            }

            // Free the empty root node
            ArrayTreeNodeFree(*root);
            *root = x;
        } else {
            // Reset last used index, if last entry was extracted
            if (CmpGreaterEqual(idx, (*root)->highest_index)) {
                // Find the last used index of the array
                for (i = idx - 1; i >= 0; i--) {
                    assert(i<MAX_NODE_ARRAY_SIZE);
                    if ((*root)->entries[i].p_val != NULL) {
                        (*root)->highest_index = i;
                        break;
                    }
                }
            }
        }
        return cell;
    } else {
        return NULL;
    }
}

ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p *root) {
    fprintf(stdout, "ArrayTreeExtractRoot\n");

    if (*root) {
        return ArrayTreeExtractEntry(root, (*root)->key);
    }
    return NULL;
}

bool ArrayTreeDeleteEntry(ArrayTree_p *root, long key) {
    fprintf(stdout, "ArrayTreeDeleteEntry -> key: %ld\n", key);
    ArrayTree_p cell;

    // Extract the entry associated with the key
    cell = ArrayTreeExtractEntry(root, key);
    if (cell) {
        ArrayTreeNodeFree(cell);
        return true;
    }
    return false;
}

long ArrayTreeNodes(ArrayTree_p root) {
    fprintf(stdout, "ArrayTreeNodes\n");
    // Allocate a stack for iterative traversal
    PStack_p stack = PStackAlloc();
    long     res   = 0; // Counter for the total number of entries

    PStackPushP(stack, root);

    // Process nodes iteratively until the stack is empty
    while (!PStackEmpty(stack)) {
        root = PStackPopP(stack);

        if (root) {
            // Add the number of entries in the current node to the result
            res += root->entry_count;

            // Push the left and right children onto the stack
            if (root->lson) {
                PStackPushP(stack, root->lson);
            }
            if (root->rson) {
                PStackPushP(stack, root->rson);
            }
        }
    }
    PStackFree(stack);

    return res;
}

ArrayTree_p ArrayTreeMaxNode(ArrayTree_p root) {
    fprintf(stdout, "ArrayTreeMaxNode\n");
    // Traverse to the rightmost node
    while (root && root->rson) {
        root = root->rson;
    }

    // Return the rightmost node (if it exists)
    return root;
}

PStack_p ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit) {
    fprintf(stdout, "ArrayTreeLimitedTraverseInit -> limit: %ld\n", limit);
    uint8_t i, idx = 0;
    ArrayTree_p handle;
    PStack_p stack = PStackAlloc();
    
    while (root) {
        if (CmpLessVal((root->key + root->highest_index), limit)) {
            // Highest existing key is lower than limit
            root = root->rson;
        } else if (CmpGreaterEqual(root->key, limit)) {
            // All keys are at least matching the limit
            PStackPushP(stack, root);
            if (CmpEqual(root->key, limit)) {
                root = NULL;
            } else {
                root = root->lson;
            }
        } else if (CmpGreaterEqual((root->key + root->highest_index), limit)) {
            // Not all keys are greater than the limit
            for (i = 0; i <= root->highest_index; i++) {
                if (root->entries[i].p_val &&
                    CmpGreaterEqual((root->key + i), limit)) {
                    idx = i;
                    break;
                }
            }

            handle->key = root->key;
            handle->lson = root->lson;
            handle->rson = root->rson;
            for (i = idx; i <= root->highest_index; i++) {
                if (root->entries[i].p_val) {
                    handle->entries[i] = root->entries[i];
                    handle->entry_count++;
                }
            }
            handle->highest_index = root->highest_index;
            PStackPushP(stack, handle);
            if (CmpEqual((handle->key + idx), limit)) {
                root = NULL;
            } else {
                root = root->lson;
            }
        } else {
            printf("Warning LimitedTraverse: Edge case reached!");
        }
    }
    return stack;
}

AVL_TRAVERSE_DEFINITION(ArrayTree, ArrayTree_p)
