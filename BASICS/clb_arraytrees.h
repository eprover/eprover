#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

#define MAX_NODE_ARRAY_SIZE 8

typedef struct arraytree_node {
    long key;
    IntOrP entries[MAX_NODE_ARRAY_SIZE];
    struct arraytree_node* lson;
    struct arraytree_node* rson;
} ArrayTreeNode, *ArrayTree_p;

#define ArrayTreeCellFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))