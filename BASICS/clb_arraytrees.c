#include "clb_arraytrees.h"
#include "clb_simple_stuff.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static ArrayTree_p splay_tree(ArrayTree_p tree, long key)
{
    ArrayTree_p left, right, tmp;
    ArrayTreeNode newnode; // Placeholder node for tree manipulation
    long cmpres;
    uint8_t i;

    // Return if the tree is empty
    if (!tree)
    {
        return tree;
    }

    // Initialize the temporary nodes
    newnode.lson = NULL;
    newnode.rson = NULL;
    left = &newnode;
    right = &newnode;

    for (;;)
    {
        // Compare the key with all entries in the current node
        bool found = false;
        for (i = 0; i < tree->entry_count; i++)
        {
            cmpres = key - tree->entries[i].key;
            if (cmpres == 0)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            // Key found in the current node
            tree->last_access_index = i; // Update last accessed index
            break;
        }

        // Navigate left or right based on the first or last key in the node
        cmpres = (key < tree->entries[0].key) ? -1 : (key > tree->entries[tree->entry_count - 1].key) ? 1 : 0;

        if (cmpres < 0)
        {
            // Key is smaller, move to the left subtree
            if (!tree->lson)
            {
                break;
            }
            if (key < tree->lson->entries[0].key)
            {
                // Perform a right rotation
                tmp = tree->lson;
                tree->lson = tmp->rson;
                tmp->rson = tree;
                tree = tmp;
                if (!tree->lson)
                {
                    break;
                }
            }
            right->lson = tree;
            right = tree;
            tree = tree->lson;
        }
        else if (cmpres > 0)
        {
            // Key is larger, move to the right subtree
            if (!tree->rson)
            {
                break;
            }
            if (key > tree->rson->entries[tree->rson->entry_count - 1].key)
            {
                // Perform a left rotation
                tmp = tree->rson;
                tree->rson = tmp->lson;
                tmp->lson = tree;
                tree = tmp;
                if (!tree->rson)
                {
                    break;
                }
            }
            left->rson = tree;
            left = tree;
            tree = tree->rson;
        }
        else
        {
            // Key is within the range of the current node but not found
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

static long arraytree_print(FILE* out, ArrayTree_p tree, bool keys_only, int indent)
{
    DStr_p indstr;
    int i, size;

    // Allocate a dynamic string for indentation
    indstr = DStrAlloc();
    for (i = 0; i < indent; i++)
    {
        DStrAppendStr(indstr, "  ");
    }

    // Handle empty tree
    if (!tree)
    {
        fprintf(out, "%s[]\n", DStrView(indstr));
        size = 0;
    }
    else
    {
        // Print all entries in the current node
        for (uint8_t j = 0; j < tree->entry_count; j++)
        {
            if (keys_only)
            {
                fprintf(out, "%sKey: %ld\n", DStrView(indstr), tree->entries[j].key);
            }
            else
            {
                fprintf(out, "%sKey: %ld\n", DStrView(indstr), tree->entries[j].key);
                fprintf(out, "%s  Val1: %ld  Val2: %ld\n", DStrView(indstr),
                        tree->entries[j].val1.i_val, tree->entries[j].val2.i_val);
            }
        }

        // Print pointers to child nodes if keys_only is false
        if (!keys_only)
        {
            fprintf(out, "%sLeft child: %p  Right child: %p\n", DStrView(indstr),
                    (void*)tree->lson, (void*)tree->rson);
        }

        // Initialize size counter
        size = tree->entry_count;

        // Recursively print left and right subtrees
        if (tree->lson || tree->rson)
        {
            size += arraytree_print(out, tree->lson, keys_only, indent + 2);
            size += arraytree_print(out, tree->rson, keys_only, indent + 2);
        }
    }

    // Free the indentation string
    DStrFree(indstr);

    return size;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

ArrayTree_p ArrayTreeNodeAllocEmpty(void)
{
    // Allocate memory for a new ArrayTree node
    ArrayTree_p handle = (ArrayTree_p)malloc(sizeof(ArrayTreeNode));
    if (!handle)
    {
        // Allocation failed, return NULL
        return NULL;
    }

    // Initialize the first entry of the array
    handle->entries[0].key = 0;
    handle->entries[0].val1.i_val = 0;
    handle->entries[0].val2.i_val = 0;

    // Initialize metadata
    handle->entry_count = 0;         // No entries initially
    handle->last_access_index = 0;   // Default to 0 (no access yet)

    // Initialize child pointers
    handle->lson = NULL;
    handle->rson = NULL;

    return handle;
}

void ArrayTreeFree(ArrayTree_p junk)
{
    if (junk)
    {
        // Allocate a stack for iterative traversal of the tree
        PStack_p stack = PStackAlloc();

        // Push the root node onto the stack
        PStackPushP(stack, junk);

        // Process nodes iteratively until the stack is empty
        while (!PStackEmpty(stack))
        {
            // Pop a node from the stack
            junk = PStackPopP(stack);

            // If the left child exists, push it onto the stack
            if (junk->lson)
            {
                PStackPushP(stack, junk->lson);
            }

            // If the right child exists, push it onto the stack
            if (junk->rson)
            {
                PStackPushP(stack, junk->rson);
            }
            free(junk);
        }

        // Free the stack used for traversal
        PStackFree(stack);
    }
}

ArrayTree_p ArrayTreeInsert(ArrayTree_p *root, ArrayTree_p newnode)
{
    // If the tree is empty, make the new node the root
    if (!*root)
    {
        newnode->lson = newnode->rson = NULL;
        *root = newnode;
        return NULL;
    }

    // Splay the tree to bring the closest key to the root
    *root = splay_tree(*root, newnode->entries[0].key);

    // Compare the key of the new node with the root's first key
    long cmpres = newnode->entries[0].key - (*root)->entries[0].key;

    if (cmpres < 0)
    {
        // Check if the root node has space in its array
        if ((*root)->entry_count < MAX_NODE_ARRAY_SIZE)
        {
            // Insert the new key into the existing node
            (*root)->entries[(*root)->entry_count] = newnode->entries[0];
            (*root)->entry_count++;
            return NULL;
        }

        // Create a new node and attach it as the left child
        newnode->lson = (*root)->lson;
        newnode->rson = *root;
        (*root)->lson = NULL;
        *root = newnode;
        return NULL;
    }
    else if(cmpres > 0)
    {
        // Check if the root node has space in its array
        if ((*root)->entry_count < MAX_NODE_ARRAY_SIZE)
        {
            // Insert the new key into the existing node
            (*root)->entries[(*root)->entry_count] = newnode->entries[0];
            (*root)->entry_count++;
            return NULL;
        }

        // Create a new node and attach it as the right child
        newnode->rson = (*root)->rson;
        newnode->lson = *root;
        (*root)->rson = NULL;
        *root = newnode;
        return NULL;
    }
    // The key already exists, return the existing root
    return *root;
}

bool ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val1, IntOrP val2)
{
    // Allocate a new node for the key-value pair
    ArrayTree_p handle = (ArrayTree_p)malloc(sizeof(ArrayTreeNode));
    if (!handle)
    {
        return false;
    }

    // Initialize the first entry in the new node
    handle->entries[0].key = key;
    handle->entries[0].val1 = val1;
    handle->entries[0].val2 = val2;

    // Initialize metadata for the new node
    handle->entry_count = 1;         // Only one entry initially
    handle->last_access_index = 0;  // This entry is the most recently used
    handle->lson = NULL;
    handle->rson = NULL;

    // Try to insert the node into the tree
    ArrayTree_p newnode = ArrayTreeInsert(root, handle);

    if (newnode)
    {
        // If the key already exists, free the allocated node and return false
        free(handle);
        return false;
    }
    return true;
}


long ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only)
{
    long size = 0;

    if (!tree)
    {
        fprintf(out, "Empty tree.\n");
        return size;
    }

    // Print the current node's entries
    fprintf(out, "Node: [\n");
    for (uint8_t i = 0; i < tree->entry_count; i++)
    {
        fprintf(out, "  Entry %d: Key = %ld", i, tree->entries[i].key);
        if (!keys_only)
        {
            fprintf(out, ", Val1 = %ld, Val2 = %ld",
                    tree->entries[i].val1.i_val,
                    tree->entries[i].val2.i_val);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "]\n");
    size++;

    // Recursively print the left and right subtrees
    if (tree->lson)
    {
        fprintf(out, "Left subtree:\n");
        size += ArrayTreeDebugPrint(out, tree->lson, keys_only);
    }
    if (tree->rson)
    {
        fprintf(out, "Right subtree:\n");
        size += ArrayTreeDebugPrint(out, tree->rson, keys_only);
    }
    fprintf(out, "Total size at this level: %ld\n", size);

    return size;
}

ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key)
{
    // Check if the tree is empty
    if (*root)
    {
        // Perform the splay operation to bring the closest key to the root
        *root = splay_tree(*root, key);

        // Search for the key in the root's entries array
        for (uint8_t i = 0; i < (*root)->entry_count; i++)
        {
            if ((*root)->entries[i].key == key)
            {
                // Key found: Swap it with the entry at index 0
                if (i != 0)
                {
                    ArrayEntry temp = (*root)->entries[0];
                    (*root)->entries[0] = (*root)->entries[i];
                    (*root)->entries[i] = temp;
                }

                // Update the last accessed index
                (*root)->last_access_index = 0;

                // Return the root
                return *root;
            }
        }
    }

    // Key not found
    return NULL;
}

ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p *root, long key)
{
    ArrayTree_p x, cell = NULL;

    // Return NULL if the tree is empty
    if (!(*root))
    {
        return NULL;
    }

    // Perform the splay operation to bring the closest key to the root
    *root = splay_tree(*root, key);

    // Search for the key in the root's entries array
    for (uint8_t i = 0; i < (*root)->entry_count; i++)
    {
        if ((*root)->entries[i].key == key)
        {
            // Key found, remove it from the array
            cell = (ArrayTree_p)malloc(sizeof(ArrayTreeNode));
            if (!cell)
            {
                return NULL;
            }

            // Copy the removed entry to the returned cell
            cell->entries[0] = (*root)->entries[i];
            cell->entry_count = 1;
            cell->last_access_index = 0;
            cell->lson = NULL;
            cell->rson = NULL;

            // Shift the remaining entries in the array
            for (uint8_t j = i; j < (*root)->entry_count - 1; j++)
            {
                (*root)->entries[j] = (*root)->entries[j + 1];
            }
            (*root)->entry_count--;

            // Check if the node is now empty
            if ((*root)->entry_count == 0)
            {
                // Reorganize the tree if the root is empty
                if (!(*root)->lson)
                {
                    x = (*root)->rson;
                }
                else
                {
                    x = splay_tree((*root)->lson, key);
                    x->rson = (*root)->rson;
                }

                // Free the empty root node
                free(*root);
                *root = x;
            }

            return cell;
        }
    }
    return NULL;
}

ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p *root)
{
    if (*root)
    {
        // Extract the first entry (key at the first index) from the root
        return (root, (*root)->entries[0].key);
    }
    return NULL;
}

bool ArrayTreeDeleteEntry(ArrayTree_p *root, long key)
{
    ArrayTree_p cell;

    // Extract the entry associated with the key
    cell = ArrayTreeExtractEntry(root, key);
    if (cell)
    {
        ArrayTreeFree(cell);
        return true;
    }
    return false;
}

long ArrayTreeNodes(ArrayTree_p root)
{
    // Allocate a stack for iterative traversal
    PStack_p stack = PStackAlloc();
    long     res   = 0; // Counter for the total number of entries

    PStackPushP(stack, root);

    // Process nodes iteratively until the stack is empty
    while (!PStackEmpty(stack))
    {
        root = PStackPopP(stack);

        if (root)
        {
            // Add the number of entries in the current node to the result
            res += root->entry_count;

            // Push the left and right children onto the stack
            if (root->lson)
            {
                PStackPushP(stack, root->lson);
            }
            if (root->rson)
            {
                PStackPushP(stack, root->rson);
            }
        }
    }
    PStackFree(stack);

    return res;
}

ArrayTree_p ArrayTreeMaxNode(ArrayTree_p root)
{
    // Traverse to the rightmost node
    while (root && root->rson)
    {
        root = root->rson;
    }

    // Return the rightmost node (if it exists)
    return root;
}

PStack_p ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit)
{
    PStack_p stack = PStackAlloc();

    // Traverse the tree to find nodes within the limit
    while (root)
    {
        // Check if the largest key in the current node is smaller than the limit
        if (root->entry_count > 0 && root->entries[root->entry_count - 1].key < limit)
        {
            root = root->rson;
        }
        else
        {
            PStackPushP(stack, root);

            // If the smallest key in the current node equals or exceeds the limit, stop traversing
            if (root->entries[0].key >= limit)
            {
                root = NULL;
            }
            else
            {
                root = root->lson;
            }
        }
    }
    return stack;
}

AVL_TRAVERSE_DEFINITION(ArrayTree, ArrayTree_p)