#ifndef ARRAYTREES_H
#define ARRAYTREES_H

#include <clb_pdrangearrays.h>

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

ArrayTree_p ArrayTreeAlloc(void);
void ArrayTreeFree(ArrayTree_p tree);
void ArrayTreeInsert(ArrayTree_p *root, long key, void *value);
void* ArrayTreeFind(ArrayTree_p *root, long key);
void ArrayTreeDebugPrint(FILE *out, ArrayTree_p tree);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
