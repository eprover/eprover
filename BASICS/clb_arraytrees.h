/*-----------------------------------------------------------------------
  File    : clb_arraytrees.h
  Purpose : Definition of an array-based splay tree (ArrayTree),
            where each node contains an array for multiple key-value pairs.
            Optimized for quick access to the last used key
            and memory-efficient due to the uint8_t data type.
-----------------------------------------------------------------------*/

#ifndef CLB_ARRAYTREES_H
#define CLB_ARRAYTREES_H

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/* Maximum allowed size of the array per node */
#define MAX_NODE_ARRAY_SIZE 8

/* Structure for displaying a key-value pair */
typedef struct
{
    long key;
    IntOrP val1;
    IntOrP val2;
} ArrayEntry;

/* Array-based node in the tree */
typedef struct arraytree_node
{
    ArrayEntry entries[MAX_NODE_ARRAY_SIZE]; // Array for key-value pairs
    uint8_t entry_count;                     // Number of assigned entries (max. 255)
    uint8_t last_access_index;               // Index of the last used entry (max. 255)
    struct arraytree_node* lson;             // Pointer to the left child node
    struct arraytree_node* rson;             // Pointer to the right child node
} ArrayTreeNode, *ArrayTree_p;


/* Allocation macros for ArrayTree nodes */
#define ArrayTreeNodeAlloc() (ArrayTree_p)SizeMalloc(sizeof(ArrayTreeNode))
#define ArrayTreeNodeFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))

#ifdef CONSTANT_MEM_ESTIMATE
#define ARRAYTREECELL_MEM (8 + (MAX_NODE_ARRAY_SIZE * sizeof(ArrayEntry)))
#else
#define ARRAYTREECELL_MEM MEMSIZE(ArrayTreeNode)
#endif

/* Core functions for ArrayTree */
ArrayTree_p ArrayTreeNodeAllocEmpty(void);
void        ArrayTreeFree(ArrayTree_p root);
ArrayTree_p ArrayTreeInsert(ArrayTree_p *root, ArrayTree_p newnode);
bool        ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val1, IntOrP val2);
ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key);
bool        ArrayTreeDeleteEntry(ArrayTree_p *root, long key);
long        ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only);
ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p* root, long key);
ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p* root);
long        ArrayTreeNodes(ArrayTree_p root);
ArrayTree_p ArrayTreeMaxNode(ArrayTree_p root);
#define     ArrayTreeMaxKey(tree) ((tree && ArrayTreeMaxNode(tree)) ? \
                                  ArrayTreeMaxNode(tree)->entries[tree->entry_count - 1].key : 0)

/* Traversal functions for ArrayTree */
PStack_p    ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit);

AVL_TRAVERSE_DECLARATION(ArrayTree, ArrayTree_p)
#define ArrayTreeTraverseExit(stack) PStackFree(stack)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/