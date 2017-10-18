/*-----------------------------------------------------------------------

File  : clb_floattrees.c

Author: Stephan Schulz

Contents

  Functions for long-indexed splay trees.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 25 02:36:58 MET DST 1997
    New
<2> Mon Mar  1 17:20:47 MET 1999
    Changed AVL tp splay trees

-----------------------------------------------------------------------*/

#include "clb_floattrees.h"



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

static FloatTree_p splay_tree(FloatTree_p tree, double key)
{
   FloatTree_p   left, right, tmp;
   FloatTreeCell new;

   if (!tree)
   {
      return tree;
   }

   new.lson = NULL;
   new.rson = NULL;
   left = &new;
   right = &new;

   for (;;)
   {
      double cmpres = key-tree->key;
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
   tree->lson = new.rson;
   tree->rson = new.lson;

   return tree;
}





/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: FloatTreeCellAllocEmpty()
//
//   Allocate a empty, initialized FloatTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FloatTree_p FloatTreeCellAllocEmpty(void)
{
   FloatTree_p handle = FloatTreeCellAlloc();

   handle->val1.i_val = handle->val2.i_val = 0;
   handle->lson       = handle->rson       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: FloatTreeFree()
//
//   Free a floattree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FloatTreeFree(FloatTree_p junk)
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
         FloatTreeCellFree(junk);
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FloatTreeInsert()
//
//   If an entry with key *new->key exists in the tree return a
//   pointer to it. Otherwise insert *new in the tree and return
//   NULL.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

FloatTree_p FloatTreeInsert(FloatTree_p *root, FloatTree_p new)
{
   if (!*root)
   {
      new->lson = new->rson = NULL;
      *root = new;
      return NULL;
   }
   *root = splay_tree(*root, new->key);

   double cmpres = new->key-(*root)->key;

   if (cmpres < 0)
   {
      new->lson = (*root)->lson;
      new->rson = *root;
      (*root)->lson = NULL;
      *root = new;
      return NULL;
   }
   else if(cmpres > 0)
   {
      new->rson = (*root)->rson;
      new->lson = *root;
      (*root)->rson = NULL;
      *root = new;
      return NULL;
   }
   return *root;
}


/*-----------------------------------------------------------------------
//
// Function: FloatTreeStore()
//
//   Insert a cell associating key with val1 and val2 into the
//   tree. Return false if an entry for this key exists, true
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

bool FloatTreeStore(FloatTree_p *root, double key, IntOrP val1, IntOrP val2)
{
   FloatTree_p handle, new;

   handle = FloatTreeCellAlloc();
   handle->key = key;
   handle->val1 = val1;
   handle->val2 = val2;

   new = FloatTreeInsert(root, handle);

   if(new)
   {
      FloatTreeCellFree(handle);
      return false;
   }
   return true;
}



/*-----------------------------------------------------------------------
//
// Function: FloatTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FloatTree_p FloatTreeFind(FloatTree_p *root, double key)
{
   if(*root)
   {
      *root = splay_tree(*root, key);
      if((*root)->key==key)
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: FloatTreeExtractEntry()
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


FloatTree_p FloatTreeExtractEntry(FloatTree_p *root, double key)
{
   FloatTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if(key==(*root)->key)
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
// Function: FloatTreeDeleteEntry()
//
//   Delete the entry with key key from the tree.
//
// Global Variables: -
//
// Side Effects    : By FloatTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool FloatTreeDeleteEntry(FloatTree_p *root, double key)
{
   FloatTree_p cell;

   cell = FloatTreeExtractEntry(root, key);
   if(cell)
   {
      FloatTreeFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: FloatTreeNodes()
//
//   Return the floatber of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FloatTreeNodes(FloatTree_p root)
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


AVL_TRAVERSE_DEFINITION(FloatTree, FloatTree_p)



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


