/*-----------------------------------------------------------------------

  This code is an initial attempts to implement a self-arranging array
  tree using the generic array 'PDRangeArray'. This code has not been
  tested yet.

  -----------------------------------------------------------------------*/

#include "arraytrees.h"

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
// Function: splay()
//
//   Perform the splay operation on tree at node with key.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static ArrayTree_p splay(ArrayTree_p root, long key)
{
    if (!root)
        return NULL;

    ArrayTreeNode dummy = {0, NULL, NULL, NULL};
    ArrayTree_p left = &dummy, right = &dummy;
    while (1)
    {
        if (key < root->key)
        {
            if (!root->left)
                break;
            if (key < root->left->key)
            {
                ArrayTree_p tmp = root->left;
                root->left = tmp->right;
                tmp->right = root;
                root = tmp;
                if (!root->left)
                    break;
            }
            right->left = root;
            right = root;
            root = root->left;
        }
        else if (key > root->key)
        {
            if (!root->right)
                break;
            if (key > root->right->key)
            {
                ArrayTree_p tmp = root->right;
                root->right = tmp->left;
                tmp->left = root;
                root = tmp;
                if (!root->right)
                    break;
            }
            left->right = root;
            left = root;
            root = root->right;
        }
        else
        {
            break;
        }
    }
    left->right = root->left;
    right->left = root->right;
    root->left = dummy.right;
    root->right = dummy.left;
    return root;
}

/*---------------------------------------------------------------------*/
/*                       Exported Functions                            */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeAlloc()
//
//   Allocate a empty, initialized ArrayTree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeAlloc(void)
{
    return NULL; /* An empty tree is a NULL-Pointer */
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeFree()
//
//   Free a arraytree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ArrayTreeFree(ArrayTree_p tree)
{
    if (!tree)
        return;

    ArrayTreeFree(tree->left);
    ArrayTreeFree(tree->right);
    PDRangeArrFree(tree->array);
    free(tree);
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeInsert()
//
//   Insert a new value to the PDRangeArray in the respective tree node.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

void ArrayTreeInsert(ArrayTree_p *root, long key, void *value)
{
    assert(root);

    if (!*root)
    {
        /* New node */
        *root = malloc(sizeof(ArrayTreeNode));
        (*root)->key = key;
        (*root)->array = PDRangeArrAlloc(key, MAX_ARRAYTREE_NODE_SIZE);
        (*root)->left = NULL;
        (*root)->right = NULL;
        PDRangeArrAssignP((*root)->array, key, value);
        return;
    }

    *root = splay(*root, key);

    /* Check if key fits in an existing array */
    if (PDRangeArrIndexIsCovered((*root)->array, key) || PDRangeArrMembers((*root)->array) < MAX_ARRAYTREE_NODE_SIZE)
    {
        PDRangeArrAssignP((*root)->array, key, value);
    }
    else
    {
        /* New node necessary */
        ArrayTree_p new_node = malloc(sizeof(ArrayTreeNode));
        new_node->key = key;
        new_node->array = PDRangeArrAlloc(key, MAX_ARRAYTREE_NODE_SIZE);
        PDRangeArrAssignP(new_node->array, key, value);

        if (key < (*root)->key)
        {
            new_node->left = (*root)->left;
            new_node->right = *root;
            (*root)->left = NULL;
        }
        else
        {
            new_node->right = (*root)->right;
            new_node->left = *root;
            (*root)->right = NULL;
        }
        *root = new_node;
    }
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeFind()
//
//   Find the entry with key key in the array tree and return it.
//   Return NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void* ArrayTreeFind(ArrayTree_p *root, long key)
{
    if (!*root)
        return NULL;

    *root = splay(*root, key);

    if (PDRangeArrIndexIsCovered((*root)->array, key))
    {
        return PDRangeArrElementP((*root)->array, key);
    }
    return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDebugPrint()
//
//   Print the tree for debugging.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ArrayTreeDebugPrint(FILE *out, ArrayTree_p tree)
{
    if (!tree)
    {
        fprintf(out, "NULL\n");
        return;
    }

    fprintf(out, "Key: %ld, Values: ", tree->key);
    for (long i = PDRangeArrLowKey(tree->array); i < PDRangeArrLimitKey(tree->array); i++)
    {
        void* value = PDRangeArrElementP(tree->array, i);
        if (value)
        {
            fprintf(out, "[%ld: %p] ", i, value);
        }
    }
    fprintf(out, "\n");
    ArrayTreeDebugPrint(out, tree->left);
    ArrayTreeDebugPrint(out, tree->right);
}
