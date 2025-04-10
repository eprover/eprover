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

/* Array-based node in the tree */
typedef struct arraytree_node {
   long key;                                // key of the first entry
   IntOrP entries[MAX_NODE_ARRAY_SIZE];     // Array for key-value pairs
   uint8_t entry_count;                     // Number of assigned entries (max. 255)
   uint8_t highest_index;                   // Highest used index of the array
   struct arraytree_node* lson;             // Pointer to the left child node
   struct arraytree_node* rson;             // Pointer to the right child node
} ArrayTreeNode, *ArrayTree_p;


/* Allocation macros for ArrayTree nodes */
#define ArrayTreeNodeAlloc() (ArrayTree_p)SizeMalloc(sizeof(ArrayTreeNode))
#define ArrayTreeNodeFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))

#ifdef CONSTANT_MEM_ESTIMATE
#define ARRAYTREECELL_MEM (sizeof(uint8_t) * 2 + sizeof(void*) * 2 + (MAX_NODE_ARRAY_SIZE * sizeof(ArrayEntry)))
#else
#define ARRAYTREECELL_MEM MEMSIZE(ArrayTreeNode)
#endif

#define KeyCmp(k1, k2)           KeyCmpFun(k1, k2)
#define CmpEqual(cmp,val)        (((long)(cmp))==((long)(val)))
#define CmpGreaterEqual(cmp,val) (((long)(cmp))>=((long)(val)))
#define CmpLessVal(cmp,val)      (((long)(cmp))< ((long)(val)))

static inline int KeyCmpFun(long k1, long k2);

/* Core functions for ArrayTree */
ArrayTree_p ArrayTreeNodeAllocEmpty(void);
void        ArrayTreeFree(ArrayTree_p root);
ArrayTree_p ArrayTreeInsert(ArrayTree_p *root, ArrayTree_p newnode, long idx);
bool        ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val1);
ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key);
bool        ArrayTreeDeleteEntry(ArrayTree_p *root, long key);
long        ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only);
ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p* root, long key);
ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p* root);
long        ArrayTreeNodes(ArrayTree_p root);
ArrayTree_p ArrayTreeMaxNode(ArrayTree_p root);
void        ArrayTreePrintGV(ArrayTree_p tree, const char* filename);
#define     ArrayTreeMaxKey(tree) ((tree && ArrayTreeMaxNode(tree)) ? \
                                  ArrayTreeMaxNode(tree)->key + ArrayTreeMaxNode(tree)->highest_index : 0)

/* Traversal functions for ArrayTree */
PStack_p    ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit);

void        ArrayTreeDebug();

AVL_TRAVERSE_DECLARATION(ArrayTree, ArrayTree_p)
#define ArrayTreeTraverseExit(stack) PStackFree(stack)

static inline int KeyCmpFun(long k1, long k2) {
   return k1 - k2;
}

static inline int CalcKey(long key) {
    return key / MAX_NODE_ARRAY_SIZE * MAX_NODE_ARRAY_SIZE;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
