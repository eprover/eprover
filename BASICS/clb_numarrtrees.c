/*-----------------------------------------------------------------------

File  : clb_numarrtrees.c

Author: Albert Eisfeld

Contents

  Functions for long-indexed splay trees with arrays of values.

Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes (vastly incomplete, see CVS log)

<1> Mon Aug  1 11:03:32 CEST 2011
    New from clb_numtxrees.h

-----------------------------------------------------------------------*/

#include "clb_numarrtrees.h"

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
// Function: node_cmp()
//
//   Compare key with the key range of node. Return a negative value
//   if key is smaller than all keys in the node, a positive value if
//   it is larger, and 0 if the node holds key.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline int node_cmp(NumArrTree_p node, long key)
{
   if(key < node->key)
   {
      return -1;
   }
   /* Unsigned difference, so that this cannot overflow */
   if((unsigned long)key - (unsigned long)node->key >= (unsigned long)node->size)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: splay_tree()
//
//   Perform the splay operation on tree at node with key. Afterwards,
//   the root is the node holding key, or, if there is none, a node
//   adjacent to key in key order.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static NumArrTree_p splay_tree(NumArrTree_p tree, long key)
{
   NumArrTree_p   left, right, tmp;
   NumArrTreeCell newnode;

   if (!tree)
   {
      return tree;
   }

   newnode.lson = NULL;
   newnode.rson = NULL;
   left = &newnode;
   right = &newnode;

   for (;;)
   {
      int cmpres = node_cmp(tree, key);
      if (cmpres < 0)
      {
         if(!tree->lson)
         {
            break;
         }
         if(node_cmp(tree->lson, key) < 0)
         {
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
      else if(cmpres > 0)
      {
         if (!tree->rson)
         {
            break;
         }
         if(node_cmp(tree->rson, key) > 0)
         {
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
         break;
      }
   }
   left->rson = tree->lson;
   right->lson = tree->rson;
   tree->lson = newnode.rson;
   tree->rson = newnode.lson;

   return tree;
}


/*-----------------------------------------------------------------------
//
// Function: tree_insert_root()
//
//   Insert node, whose key range must not overlap with that of any
//   node in tree, into tree. Return the new tree, with node at the
//   root.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static NumArrTree_p tree_insert_root(NumArrTree_p tree, NumArrTree_p node)
{
   if(!tree)
   {
      node->lson = node->rson = NULL;
      return node;
   }
   tree = splay_tree(tree, node->key);
   assert(node_cmp(tree, node->key) != 0);

   if(node->key < tree->key)
   {
      node->lson = tree->lson;
      node->rson = tree;
      tree->lson = NULL;
   }
   else
   {
      node->rson = tree->rson;
      node->lson = tree;
      tree->rson = NULL;
   }
   return node;
}


/*-----------------------------------------------------------------------
//
// Function: tree_remove_root()
//
//   Unlink the root from tree and return the remaining tree. The
//   root itself is not freed.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static NumArrTree_p tree_remove_root(NumArrTree_p root)
{
   NumArrTree_p res;

   if(!root->lson)
   {
      return root->rson;
   }
   /* All keys in the left subtree are smaller, so this splays its
      maximum to the top, which has no right son */
   res = splay_tree(root->lson, root->key);
   assert(!res->rson);
   res->rson = root->rson;
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: merge_buddies()
//
//   node is in the tree at *root. As long as node has a buddy (the
//   node of the same size that together with node covers an aligned
//   key range of twice the size), replace both by a single node, up
//   to NUMARRTREEMAXVALUES values. Return the resulting node.
//
// Global Variables: -
//
// Side Effects    : Changes tree, memory operations
//
/----------------------------------------------------------------------*/

static NumArrTree_p merge_buddies(NumArrTree_p *root, NumArrTree_p node)
{
   NumArrTree_p buddy, low, high, merged;
   long size, buddy_key;

   while(node->size < NUMARRTREEMAXVALUES)
   {
      size      = node->size;
      buddy_key = node->key ^ size;
      buddy     = splay_tree(*root, buddy_key);
      *root     = buddy;
      if(buddy->key != buddy_key || buddy->size != size)
      {
         break;
      }
      *root = tree_remove_root(buddy);
      *root = splay_tree(*root, node->key);
      assert(*root == node);
      *root = tree_remove_root(node);

      low  = (buddy->key < node->key)? buddy : node;
      high = (buddy->key < node->key)? node  : buddy;
      merged = NumArrTreeCellAlloc(2*size);
      merged->key  = low->key;
      merged->size = 2*size;
      memcpy(merged->vals, low->vals, size*sizeof(IntOrP));
      memcpy(merged->vals+size, high->vals, size*sizeof(IntOrP));
      NumArrTreeCellFree(low);
      NumArrTreeCellFree(high);

      *root = tree_insert_root(*root, merged);
      node  = merged;
   }
   return node;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: NumArrTreeCellAllocEmpty()
//
//   Allocate an empty, initialized NumArrTreeCell for the size keys
//   starting at key. Pointers to children are NULL, all values are
//   0/NULL.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

NumArrTree_p NumArrTreeCellAllocEmpty(long key, long size)
{
   NumArrTree_p handle = NumArrTreeCellAlloc(size);

   assert((key & (size - 1)) == 0);
   handle->key  = key;
   handle->size = size;
   for(long i=0; i<size; i++)
   {
      handle->vals[i].p_val = NULL;
   }
   handle->lson = handle->rson = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: NumArrTreeFree()
//
//   Free a numtree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void NumArrTreeFree(NumArrTree_p junk)
{
   if(junk)
   {
      PStack_p stack = PStackAlloc();

      PStackPushP(stack, junk);

      while(!PStackEmpty(stack))
      {
         junk = PStackPopP(stack);
         if(junk->lson)
         {
            PStackPushP(stack, junk->lson);
         }
         if(junk->rson)
         {
            PStackPushP(stack, junk->rson);
         }
         NumArrTreeCellFree(junk);
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeNodeEmpty()
//
//   Return true if the elements of node->vals of node are NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool NumArrTreeNodeEmpty(NumArrTree_p node)
{
   for(long i = 0; i < node->size; i++)
   {
      if (node->vals[i].p_val)
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeInsertKeyValPair()
//
//   If a val at key exists in the tree return NULL. Otherwise insert val
//   in the tree and return a pointer to the node of the array containing
//   it. If this creates a new node, it is merged with its buddies
//   where possible.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

NumArrTree_p NumArrTreeInsertKeyValPair(NumArrTree_p *root, long key, void* val)
{
   NumArrTree_p handle, newnode;

   handle = NumArrTreeFindNode(root, key);
   if (handle)
   {
      if (!handle->vals[NumArrTreeSlot(handle, key)].p_val)
      {
         handle->vals[NumArrTreeSlot(handle, key)].p_val = val;
         return handle;
      }
      return NULL;
   }

   newnode = NumArrTreeCellAllocEmpty(NumArrTreeNodeKey(key), NUMARRTREEVALUES);
   newnode->vals[NumArrTreeSlot(newnode, key)].p_val = val;
   *root = tree_insert_root(*root, newnode);

   return merge_buddies(root, newnode);
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeFindNode()
//
//   Find the node containing the val with key key in the tree and return
//   it. Return NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : Reorganizes the tree
//
/----------------------------------------------------------------------*/

NumArrTree_p NumArrTreeFindNode(NumArrTree_p *root, long key)
{
   if (*root)
   {
      *root = splay_tree(*root, key);

      if (node_cmp(*root, key) == 0)
      {
         return *root;
      }
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: NumArrTreeDeleteKey()
//
//   Remove the value stored for key from the tree and return it (NULL
//   if there is none). If this empties the node, remove the node from
//   the tree and free it. Nodes are never split.
//
// Global Variables: -
//
// Side Effects    : Changes the tree, memory operations
//
/----------------------------------------------------------------------*/

void* NumArrTreeDeleteKey(NumArrTree_p *root, long key)
{
   NumArrTree_p cell = NumArrTreeFindNode(root, key);
   void* res;

   if(!cell)
   {
      return NULL;
   }
   res = cell->vals[NumArrTreeSlot(cell, key)].p_val;
   cell->vals[NumArrTreeSlot(cell, key)].p_val = NULL;

   if(res && NumArrTreeNodeEmpty(cell))
   {
      *root = tree_remove_root(cell);
      NumArrTreeCellFree(cell);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeMaxNode()
//
//   Return the node with the largest key in the tree (or NULL if tree
//   is empty). Non-destructive/non-reorganizing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

NumArrTree_p NumArrTreeMaxNode(NumArrTree_p root)
{
   if(root)
   {
      while(root->rson)
      {
         root = root->rson;
      }
   }
   return root;
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeMaxKey()
//
//   Return the largest key with a value in node. For a node without
//   values, return the largest key the node can hold, so that the
//   result is always an upper bound.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long NumArrTreeMaxKey(NumArrTree_p node)
{
   assert(node);

   for(long i = node->size-1; i >= 0; i--)
   {
      if(node->vals[i].p_val)
      {
         return node->key + i;
      }
   }
   return node->key + node->size - 1;
}


/*-----------------------------------------------------------------------
//
// Function: NumArrTreeLimitedTraverseInit()
//
//   Return a stack containing the path to the first node that may
//   hold keys >= limit, i.e. the first node in key order that does
//   not lie completely below limit. Keys in that node that are
//   smaller than limit have to be skipped by the caller.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PStack_p NumArrTreeLimitedTraverseInit(NumArrTree_p root, long limit)
{
   PStack_p stack = PStackAlloc();
   int      cmpres;

   while(root)
   {
      cmpres = node_cmp(root, limit);
      if(cmpres > 0)
      {
         root = root->rson;
      }
      else
      {
         PStackPushP(stack, root);
         root = (cmpres == 0)? NULL : root->lson;
      }
   }
   return stack;
}

/*-----------------------------------------------------------------------
//
// Function: NumArrTreeTraverseNext()
//
//   Given a stack describing a traversal state, return the next node
//   (in key order) and update the stack. Return NULL at the end.
//
// Global Variables: -
//
// Side Effects    : Updates stack
//
/----------------------------------------------------------------------*/

NumArrTree_p NumArrTreeTraverseNext(PStack_p state)
{
   NumArrTree_p handle, res;

   if(PStackEmpty(state))
   {
      return NULL;
   }
   res = PStackPopP(state);
   handle = res->rson;
   while(handle)
   {
      PStackPushP(state, handle);
      handle = handle->lson;
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


