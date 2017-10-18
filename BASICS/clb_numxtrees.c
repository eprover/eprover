/*-----------------------------------------------------------------------

File  : clb_numxtrees.c

Author: Stephan Schulz

Contents

  Functions for long-indexed splay trees with fixed-sized array of
  values.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes (vastly incomplete, see CVS log)

<1> Mon Aug  1 11:03:32 CEST 2011
    New from clb_numtree.h

-----------------------------------------------------------------------*/

#include "clb_numxtrees.h"



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
// Function: splay_tree()
//
//   Perform the splay operation on tree at node with key.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static NumXTree_p splay_tree(NumXTree_p tree, long key)
{
   NumXTree_p   left, right, tmp;
   NumXTreeCell newnode;

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
      long cmpres = key-tree->key;
      if (cmpres < 0)
      {
         if(!tree->lson)
         {
            break;
         }
         if((key- tree->lson->key) < 0)
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
         if((key-tree->rson->key) > 0)
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





/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: NumXTreeCellAllocEmpty()
//
//   Allocate a empty, initialized NumXTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

NumXTree_p NumXTreeCellAllocEmpty(void)
{
   NumXTree_p handle = NumXTreeCellAlloc();
   int i;

   for(i=0; i<NUMXTREEVALUES; i++)
   {
      handle->vals[i].i_val = 0;
   }
   handle->lson = handle->rson       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: NumXTreeFree()
//
//   Free a numtree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void NumXTreeFree(NumXTree_p junk)
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
         NumXTreeCellFree(junk);
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeInsert()
//
//   If an entry with key *newnode->key exists in the tree return a
//   pointer to it. Otherwise insert *newnode in the tree and return
//   NULL.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

NumXTree_p NumXTreeInsert(NumXTree_p *root, NumXTree_p newnode)
{
   if (!*root)
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode->key);

   long cmpres = newnode->key-(*root)->key;

   if (cmpres < 0)
   {
      newnode->lson = (*root)->lson;
      newnode->rson = *root;
      (*root)->lson = NULL;
      *root = newnode;
      return NULL;
   }
   else if(cmpres > 0)
   {
      newnode->rson = (*root)->rson;
      newnode->lson = *root;
      (*root)->rson = NULL;
      *root = newnode;
      return NULL;
   }
   return *root;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeStore()
//
//   Insert a cell associating key with val1 and val2 into the
//   tree. Return false if an entry for this key exists, true
//   otherwise. Values beyond the second are zero.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

bool NumXTreeStore(NumXTree_p *root, long key, IntOrP val1, IntOrP val2)
{
   NumXTree_p handle, newnode;

   handle = NumXTreeCellAlloc();
   handle->key = key;
   handle->vals[0] = val1;
   handle->vals[1] = val2;

   newnode = NumXTreeInsert(root, handle);

   if(newnode)
   {
      NumXTreeCellFree(handle);
      return false;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

NumXTree_p NumXTreeFind(NumXTree_p *root, long key)
{
   if(*root)
   {
      *root = splay_tree(*root, key);
      if(((*root)->key-key)==0)
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeExtractEntry()
//
//   Find the entry with key key, remove it from the tree, rebalance
//   the tree, and return the pointer to the removed element. Return
//   NULL if no matching element exists.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/


NumXTree_p NumXTreeExtractEntry(NumXTree_p *root, long key)
{
   NumXTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if((key-(*root)->key)==0)
   {
      if (!(*root)->lson)
      {
         x = (*root)->rson;
      }
      else
      {
         x = splay_tree((*root)->lson, key);
         x->rson = (*root)->rson;
      }
      cell = *root;
      cell->lson = cell->rson = NULL;
      *root = x;
      return cell;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeExtractRoot()
//
//   Extract the NumXTreeCell at the root of the tree and return it (or
//   NULL if the tree is empty).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

NumXTree_p NumXTreeExtractRoot(NumXTree_p *root)
{
   if(*root)
   {
      return NumXTreeExtractEntry(root, (*root)->key);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeDeleteEntry()
//
//   Delete the entry with key key from the tree.
//
// Global Variables: -
//
// Side Effects    : By NumXTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool NumXTreeDeleteEntry(NumXTree_p *root, long key)
{
   NumXTree_p cell;

   cell = NumXTreeExtractEntry(root, key);
   if(cell)
   {
      NumXTreeFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeNodes()
//
//   Return the number of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long NumXTreeNodes(NumXTree_p root)
{
   PStack_p stack = PStackAlloc();
   long     res   = 0;

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      root = PStackPopP(stack);
      if(root)
      {
    PStackPushP(stack, root->lson);
    PStackPushP(stack, root->rson);
    res++;
      }
   }
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: NumXTreeMaxNode()
//
//   Return the node with the largest key in the tree (or NULL if tree
//   is empty). Non-destructive/non-reorganizing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

NumXTree_p NumXTreeMaxNode(NumXTree_p root)
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
// Function: NumXTreeLimitedTraverseInit()
//
//   Return a stack containing the path to the smallest element
//   smaller than or equal to limit in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStack_p NumXTreeLimitedTraverseInit(NumXTree_p root, long limit)
{
   PStack_p stack = PStackAlloc();

   while(root)
   {
      if(root->key<limit)
      {
         root = root->rson;
      }
      else
      {
         PStackPushP(stack, root);
         if(root->key == limit)
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




AVL_TRAVERSE_DEFINITION(NumXTree, NumXTree_p)



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


