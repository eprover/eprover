#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

#define MAX_NODE_ARRAY_SIZE 8

typedef struct arraytree_iter {
    long key;
    PStack_p tree_iter;
 } ArrayTreeIter, *TreeIter_p;

typedef struct arraytree_node {
    long key;
    uint8_t highest_index;
    IntOrP entries[MAX_NODE_ARRAY_SIZE];
    struct arraytree_node* lson;
    struct arraytree_node* rson;
} ArrayTreeNode, *ArrayTree_p;

#define ArrayTreeCellAlloc() (ArrayTree_p)SizeMalloc(sizeof(ArrayTreeNode))
#define ArrayTreeCellFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))

#ifdef CONSTANT_MEM_ESTIMATE
#define ARRAYTREECELL_MEM (sizeof(uint8_t) + sizeof(void*) * 2 + (MAX_NODE_ARRAY_SIZE * sizeof(ArrayEntry)))
#else
#define ARRAYTREECELL_MEM MEMSIZE(ArrayTreeNode)
#endif

void        ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only);
void        ArrayTreeFree(ArrayTree_p tree);
void        ArrayTreeNodeFree(ArrayTree_p node);
ArrayTree_p ArrayTreeNodeInsert(ArrayTree_p *root, long key) ;
ArrayTree_p ArrayTreeNodeAllocEmpty(void);
IntOrP      ArrayTreeFind(ArrayTree_p *root, long key);
void**      ArrayTreeFindRef(ArrayTree_p root, long key);
IntOrP      ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val);
IntOrP      ArrayTreeExtractEntry(ArrayTree_p *root, long key);
TreeIter_p  ArrayTreeLimitedTraverseInit(ArrayTree_p root,
                                         TreeIter_p iterator, long limit);
long        ArrayTreeMaxNode(ArrayTree_p root);

static inline int CalcKey(long key) {
    return key < 0 ? -2 : (key / MAX_NODE_ARRAY_SIZE * MAX_NODE_ARRAY_SIZE);
}

AVL_TRAVERSE_DECLARATION(ArrayTree, ArrayTree_p)
#define ArrayTreeTraverseExit(stack) PStackFree(stack)