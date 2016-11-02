/*-----------------------------------------------------------------------

File  : clb_stringtrees.c

Author: Stephan Schulz

Contents

  Functions for string-indexed SPLAY trees. Part of the implementation
  is based on public domain code by D.D. Sleator.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 25 02:36:58 MET DST 1997
    New
<2> Thu Apr  8 17:36:18 MET DST 1999
    Replaced AVL trees with splay trees (I didn't know AVL was still
    here ;-)

-----------------------------------------------------------------------*/

#include "clb_stringtrees.h"



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

static StrTree_p splay_tree(StrTree_p tree, const char* key)
{
   StrTree_p   left, right, tmp;
   StrTreeCell newnode;
   int         cmpres;

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
      cmpres = strcmp(key, tree->key);
      if (cmpres < 0)
      {
         if(!tree->lson)
         {
            break;
         }
         if(strcmp(key, tree->lson->key) < 0)
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
         if(strcmp(key, tree->rson->key) > 0)
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
// Function: StrTreeCellAllocEmpty()
//
//   Allocate a empty, initialized StrTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

StrTree_p StrTreeCellAllocEmpty(void)
{
   StrTree_p handle = StrTreeCellAlloc();

   handle->val1.i_val = handle->val2.i_val = 0;
   handle->lson       = handle->rson       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: StrTreeFree()
//
//   Free a stringtree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void StrTreeFree(StrTree_p junk)
{
   if(!junk)
   {
      return;
   }
   StrTreeFree(junk->lson);
   StrTreeFree(junk->rson);
   FREE(junk->key);
   StrTreeCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: StrTreeInsert()
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

StrTree_p StrTreeInsert(StrTree_p *root, StrTree_p newnode)
{
   int cmpres;
   if (!*root)
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode->key);

   cmpres = strcmp(newnode->key, (*root)->key);

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
// Function: StrTreeStore()
//
//   Insert a cell associating key with val1 and val2 into the
//   tree. Return NULL if an entry for this key exists, address of the
//   new node otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

StrTree_p StrTreeStore(StrTree_p *root, char* key, IntOrP val1, IntOrP val2)
{
   StrTree_p handle, old;

   handle = StrTreeCellAlloc();
   handle->key = SecureStrdup(key);
   handle->val1 = val1;
   handle->val2 = val2;

   old = StrTreeInsert(root, handle);

   if(old)
   {
      FREE(handle->key);
      StrTreeCellFree(handle);
      return NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: StrTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

StrTree_p StrTreeFind(StrTree_p *root, const char* key)
{
   if(*root)
   {
      *root = splay_tree(*root, key);
      if(strcmp((*root)->key,key)==0)
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: StrTreeExtractEntry()
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


StrTree_p StrTreeExtractEntry(StrTree_p *root, const char* key)
{
   StrTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if(strcmp(key, (*root)->key)==0)
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
// Function: StrTreeDeleteEntry()
//
//   Delete the entry with key key from the tree.
//
// Global Variables: -
//
// Side Effects    : By StrTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool StrTreeDeleteEntry(StrTree_p *root, const char* key)
{
   StrTree_p cell;

   cell = StrTreeExtractEntry(root, key);
   if(cell)
   {
      StrTreeFree(cell);
      return true;
   }
   return false;
}

AVL_TRAVERSE_DEFINITION(StrTree, StrTree_p)

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


