/*-----------------------------------------------------------------------

  File  : clb_quadtrees.c

Author: Stephan Schulz

Contents
 
  Functions for SPLAY trees indexed by two pointers and two ints, and
  containing a IntOrP.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan  4 00:55:10 MET 2000
    Modified from clb_ptrees.c

-----------------------------------------------------------------------*/

#include "clb_quadtrees.h"



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

static QuadTree_p splay_tree(QuadTree_p tree, QuadKey_p key) 
{
   QuadTree_p   left, right, tmp;
   QuadTreeCell new;
   int       cmpres;

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
      cmpres = QuadKeyCmp(key, &(tree->key));
      if (cmpres < 0) 
      {
	 if(!tree->lson)
	 {
	    break;
	 }
	 if(QuadKeyCmp(key, &(tree->lson->key)) < 0)
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
	 if(QuadKeyCmp(key, &(tree->rson->key)) > 0) 
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
// Function: DoubleKeyCmp()
//
//   Compare two pointer/integer pairs.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int DoubleKeyCmp(void* p1, int i1, void *p2, int i2)
{
   int res;

   res = PCmp(p1, p2);
   if(res)
   {
      return res;
   }
   res = i1 - i2;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: QuadKeyCmp()
//
//   Compare two QuadKeys.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int QuadKeyCmp(QuadKey_p key1, QuadKey_p key2)
{
   int res;

   res = DoubleKeyCmp(key1->p1, key1->i1, key2->p1, key2->i1);
   if(res)
   {
      return res;
   }
   res = DoubleKeyCmp(key1->p2, key1->i2, key2->p2, key2->i2);
   return res;
}
      

/*-----------------------------------------------------------------------
//
// Function: QuadTreeFree()
//
//   Free a QuadTree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void QuadTreeFree(QuadTree_p junk)
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
	 QuadTreeCellFree(junk);	    	 
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: QuadTreeInsert()
//
//   If an entry with key *new->key exists in the tree return a
//   pointer to it. Otherwise insert *new in the tree and return
//   NULL. Will splay the tree!
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

QuadTree_p QuadTreeInsert(QuadTree_p *root, QuadTree_p new)
{
   int cmpres;

   if (!*root) 
   {
      new->lson = new->rson = NULL;
      *root = new;
      return NULL;
   }
   *root = splay_tree(*root, &(new->key));

   cmpres = QuadKeyCmp(&(new->key), &(*root)->key);
   
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
// Function: QuadTreeStore()
//
//   Insert a cell with given key into the tree. Return false if an
//   entry for this key exists, true otherwise. The key is never
//   freed! 
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

bool QuadTreeStore(QuadTree_p *root, QuadKey_p key, IntOrP val)
{
   QuadTree_p handle, new;

   handle = QuadTreeCellAlloc();
   handle->key.p1    = key->p1;
   handle->key.i1    = key->i1;
   handle->key.p2    = key->p2;
   handle->key.i2    = key->i2;
   handle->val.i_val = val.i_val;

   new = QuadTreeInsert(root, handle);

   if(new)
   {
      QuadTreeCellFree(handle);
      return false;
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: QuadTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

QuadTree_p QuadTreeFind(QuadTree_p *root, QuadKey_p key)
{
   if(*root)
   {
      *root = splay_tree(*root, key);  
      if(QuadKeyCmp(&((*root)->key), key)==0)
      {
	 return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: QuadTreeExtractEntry()
//
//   Find the entry with key key, remove it from the tree, rebalance
//   the tree, and return the pointer to the removed element. Return
//   NULL if no matching element exists.
//
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/


QuadTree_p QuadTreeExtractEntry(QuadTree_p *root, QuadKey_p key)
{
   QuadTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if(QuadKeyCmp(key, &((*root)->key))==0)
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
// Function: QuadTreeDeleteEntry()
//
//   Delete the entry with key key from the tree. Return true, if the
//   key existed, false otherwise.
//
// Global Variables: -
//
// Side Effects    : By QuadTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool QuadTreeDeleteEntry(QuadTree_p *root, QuadKey_p key)
{
   QuadTree_p cell;
   
   cell = QuadTreeExtractEntry(root, key);
   if(cell)
   {
      QuadTreeCellFree(cell);
      return true;
   }
   return false;
}


AVL_TRAVERSE_DEFINITION(QuadTree, QuadTree_p)

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


