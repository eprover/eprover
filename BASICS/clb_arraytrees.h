#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

#define MAX_NODE_ARRAY_SIZE 8

typedef struct arraytree_node {
    long key;
    // uint8_t highest_index;
    IntOrP entries[MAX_NODE_ARRAY_SIZE];
    struct arraytree_node* lson;
    struct arraytree_node* rson;
} ArrayTreeNode, *ArrayTree_p;

typedef struct arraytree_iter {
    long key;
    long lower_bound;
    long upper_bound;
    ArrayTree_p node;
    PStack_p tree_iter;
 } ArrayTreeIter, *TreeIter_p;

#define ArrayTreeCellAlloc() (ArrayTree_p)SizeMalloc(sizeof(ArrayTreeNode))
#define ArrayTreeCellFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))

#ifdef CONSTANT_MEM_ESTIMATE
#define ARRAYTREECELL_MEM (sizeof(uint8_t) + sizeof(void*) * 2 + (MAX_NODE_ARRAY_SIZE * sizeof(ArrayEntry)))
#else
#define ARRAYTREECELL_MEM MEMSIZE(ArrayTreeNode)
#endif

/*-----------------------------------------------------------------------
//
// Functions regarding the tree
//
/----------------------------------------------------------------------*/

void        ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only);
void        ArrayTreeFree(ArrayTree_p tree);
void        ArrayTreeNodeFree(ArrayTree_p *root, long key);
ArrayTree_p ArrayTreeNodeInsert(ArrayTree_p root, long key);
ArrayTree_p ArrayTreeNodeAllocEmpty(void);
ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key);
bool        ArrayTreeCheckEmpty(ArrayTree_p root, long key);

/*-----------------------------------------------------------------------
//
// Functions regarding the arrays
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val);
void*       ArrayTreeGetEntry(ArrayTree_p node, long key);
void**      ArrayTreeGetEntryRef(ArrayTree_p *node, long key);
void*       ArrayTreeExtractEntry(ArrayTree_p *node, long key);
long        ArrayTreeMaxKey(ArrayTree_p root);

TreeIter_p  ArrayTreeLimitedTraverseInit(ArrayTree_p root,
                                        TreeIter_p *iterator, long limit);
void ArrayDebug();
void TreeDebug();
void ArrayTreePrintGV(ArrayTree_p tree);

static inline int CalcKey(long key) {
    if (key >= 0) return (key / MAX_NODE_ARRAY_SIZE) * MAX_NODE_ARRAY_SIZE;
    return ((key - (MAX_NODE_ARRAY_SIZE - 1)) / MAX_NODE_ARRAY_SIZE) * MAX_NODE_ARRAY_SIZE;
}

static inline uint8_t CalcIdx(long val1, long val2) {
    return (uint8_t) (val1 - val2);
}

AVL_TRAVERSE_DECLARATION(ArrayTree, ArrayTree_p)
#define ArrayTreeTraverseExit(stack) PStackFree(stack)
