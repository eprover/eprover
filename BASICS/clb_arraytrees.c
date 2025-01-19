/*-----------------------------------------------------------------------

  This code is an initial attempts to implement a self-arranging array
  tree using the generic array 'PDRangeArray'. This code has not been
  tested yet.

  -----------------------------------------------------------------------*/

#include <clb_arraytrees.h>

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

// Replace this function with a define in the header file??
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

ArrayTree_p ArrayTreeTraverseNext(PStack_p state)
{
    ArrayTree_p handle, result;

    if (PStackEmpty(state))
    {
        return NULL;
    }

    /* Pop next node from stack */
    result = PStackPopP(state);

    /* Traverse the right subtree */
    handle = result->right;
    while (handle)
    {
        PStackPushP(state, handle);
        handle = handle->left;
    }

    return result;
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDebugPrint()
//
//   Initialize the traverse.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStack_p ArrayTreeTraverseInit(ArrayTree_p root)
{
    PStack_p stack = PStackAlloc();

    /* Move as far as possible to the left and add all nodes to the stack */
    while (root)
    {
        PStackPushP(stack, root);
        root = root->left;
    }

    return stack;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDebugPrint()
//
//   Free the stack which was needed for the traverse
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ArrayTreeTraverseExit(PStack_p stack)
{
    PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDebugPrint()
//
//   Initialize the traverse which is limited by the value of limit.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStack_p ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit)
{
    PStack_p stack = PStackAlloc();

    while (root)
    {
        if (root->key < limit)
        {
            root = root->right;
        }
        else
        {
            PStackPushP(stack, root);
            if (root->key == limit)
            {
                root = NULL;
            }
            else
            {
                root = root->left;
            }
        }
    }

    return stack;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeStore()
//
//   Add a new element to the ArrayTree. Check whether a key is existing
//   and override it eventually.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

bool ArrayTreeStore(ArrayTree_p *root, long key, void *value)
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
        return true;
    }

    *root = splay(*root, key);

    /* Check if key fits in an existing array */
    if (PDRangeArrIndexIsCovered((*root)->array, key) || PDRangeArrMembers((*root)->array) < MAX_ARRAYTREE_NODE_SIZE)
    {
        PDRangeArrAssignP((*root)->array, key, value);
        return true;
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
        return true;
    }
}


/*-----------------------------------------------------------------------
//
// Function: ArrayTreeNodeFree()
//
//   Initialize the traverse which is limited by the value of limit.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ArrayTreeNodeFree(ArrayTree_p *tree)
{
    PStack_p stack;

    if (!tree || !(*tree))
    {
        return;
    }

    stack = ArrayTreeTraverseInit(*tree);

    ArrayTree_p node;
    while ((node = ArrayTreeTraverseNext(stack)))
    {
        if (node->array)
        {
            PDRangeArrFree(node->array);
        }
        free(node);
    }

    ArrayTreeTraverseExit(stack);
    free(tree);
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeDeleteNode()
//
//   Delete a single node from the array tree while ensure the consistency
//   of the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeDeleteNode(ArrayTree_p *tree, long key) {
    if (!tree || !(*tree)) {
        return NULL;
    }

    ArrayTree_p parent = NULL, current = *tree, replacement = NULL;

    while (current && current->key != key) {
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (!current) {
        return *tree; // Return the original tree if the key is not found
    }

    if (!current->left && !current->right) {
        replacement = NULL;
    } else if (!current->left) {
        replacement = current->right;
    } else if (!current->right) {
        replacement = current->left;
    } else {
        ArrayTree_p successor_parent = current;
        ArrayTree_p successor = current->right;

        while (successor->left) {
            successor_parent = successor;
            successor = successor->left;
        }

        if (successor_parent != current) {
            successor_parent->left = successor->right;
        } else {
            successor_parent->right = successor->right;
        }

        replacement = successor;
        replacement->left = current->left;
        replacement->right = current->right;
    }

    if (!parent) {
        *tree = replacement; // Update the root of the tree
    } else if (parent->left == current) {
        parent->left = replacement;
    } else {
        parent->right = replacement;
    }

    if (current->array) {
        PDRangeArrFree(current->array);
    }
    free(current);

    return *tree; // Return the updated tree
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeLeftChild()
//
//   Fetch left tree node.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeLeftChild(ArrayTree_p node) {
    if (!node) {
        return NULL;
    }
    return node->left;
}

/*-----------------------------------------------------------------------
//
// Function: ArrayTreeRightChild()
//
//   Fetch right tree node.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ArrayTree_p ArrayTreeRightChild(ArrayTree_p node) {
    if (!node) {
        return NULL;
    }
    return node->right;
}

ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p *root, long key) {
    ArrayTree_p x, extracted;

    if (!(*root)) {
        return NULL;
    }

    *root = splay(*root, key);

    if ((*root)->key == key) {
        if (!(*root)->left) {
            x = (*root)->right;
        } else {
            x = splay((*root)->left, key);
            x->right = (*root)->right;
        }
        extracted = *root;
        extracted->left = extracted->right = NULL;
        *root = x;
        return extracted;
    }

    return NULL;
}

long ArrayTreeNodes(ArrayTree_p root) {
    if (!root) {
        return 0;
    }

    PStack_p stack = PStackAlloc();
    long count = 0;

    PStackPushP(stack, root);

    while (!PStackEmpty(stack)) {
        ArrayTree_p node = PStackPopP(stack);
        if (node) {
            count++;
            PStackPushP(stack, node->left);
            PStackPushP(stack, node->right);
        }
    }

    PStackFree(stack);
    return count;
}

ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p *root) {
    ArrayTree_p extracted_node = NULL;

    if (*root) {
        extracted_node = *root;

        // Splay the root to ensure it is at the top
        *root = ArrayTreeDeleteNode(root, extracted_node->key);
    }

    return extracted_node;
}
