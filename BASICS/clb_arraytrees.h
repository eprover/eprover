#ifndef ARRAYTREES_H
#define ARRAYTREES_H

#include <clb_pdrangearrays.h>
#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                       Data type declarations                        */
/*---------------------------------------------------------------------*/

typedef struct arraytree_node
{
    long key;                         /* Key for the node */
    PDRangeArr_p array;               /* Array of values */
    struct arraytree_node *left;
    struct arraytree_node *right;
} ArrayTreeNode, *ArrayTree_p;

#define MAX_ARRAYTREE_NODE_SIZE 8     /* Max Number of values per node */

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ARRAYTREECELL_MEM 24

ArrayTree_p ArrayTreeAlloc(void);
void ArrayTreeFree(ArrayTree_p tree);
void ArrayTreeInsert(ArrayTree_p *root, long key, void *value);
void* ArrayTreeFind(ArrayTree_p *root, long key);
void ArrayTreeDebugPrint(FILE *out, ArrayTree_p tree);
ArrayTree_p ArrayTreeTraverseNext(PStack_p state);
PStack_p ArrayTreeTraverseInit(ArrayTree_p root);
PStack_p ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit);
void ArrayTreeTraverseExit(PStack_p stack);
PStack_p ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit);
bool ArrayTreeStore(ArrayTree_p *root, long key, void *value);
void ArrayTreeNodeFree(ArrayTree_p *tree);
void ArrayTreeDeleteNode(ArrayTree_p *tree, long key);
ArrayTree_p ArrayTreeLeftChild(ArrayTree_p node);
ArrayTree_p ArrayTreeRightChild(ArrayTree_p node);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
