/*-----------------------------------------------------------------------

File  : clb_numtrees.c

Author: Stephan Schulz

Contents
 
  Functions for long-indexed splay trees.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 25 02:36:58 MET DST 1997
    New
<2> Mon Mar  1 17:20:47 MET 1999
    Changed AVL tp splay trees

-----------------------------------------------------------------------*/

#include "clb_numtrees.h"



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
// Function: numtree_print()
//
//   Print the tree with the appropriate indent level and return the
//   number of nodes.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static long numtree_print(FILE* out, NumTree_p tree, bool keys_only,
                          int indent)
{
   DStr_p indstr;
   int i, size;

   indstr = DStrAlloc();
   for(i=0;i<indent;i++)
   {
      DStrAppendStr(indstr, "  ");
   }

   if(!tree)
   {
      fprintf(out, "%s[]\n", DStrView(indstr));
      size = 0;
   }
   else 
   {
      if(keys_only)
      {
         fprintf(out, "%s%ld\n", DStrView(indstr), tree->key);
      }
      else
      {
         fprintf(out, "%s%ld\n", DStrView(indstr), tree->key);
         fprintf(out, "%s Val1: %ld  Val2: %ld\n", DStrView(indstr),
                 tree->val1.i_val, tree->val2.i_val);
         fprintf(out, "%s lson: %p  rson: %p\n", DStrView(indstr),
                 (void*)tree->lson, (void*)tree->rson);
      }
      size = 1;
      if(tree->lson||tree->rson)
      {
         size += numtree_print(out, tree->lson, keys_only, indent+2);
         size += numtree_print(out, tree->rson, keys_only, indent+2);
      }
   }
   DStrFree(indstr);
   return size;   
}

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

static NumTree_p splay_tree(NumTree_p tree, long key) 
{
   NumTree_p   left, right, tmp;
   NumTreeCell new;
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
      cmpres = key-tree->key;
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
// Function: NumTreeCellAllocEmpty()
//
//   Allocate a empty, initialized NumTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

NumTree_p NumTreeCellAllocEmpty(void)
{
   NumTree_p handle = NumTreeCellAlloc();
   
   handle->val1.i_val = handle->val2.i_val = 0;
   handle->lson       = handle->rson       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: NumTreeFree()
//
//   Free a numtree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void NumTreeFree(NumTree_p junk)
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
         NumTreeCellFree(junk);            
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: NumTreeInsert()
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

NumTree_p NumTreeInsert(NumTree_p *root, NumTree_p new)
{
   int cmpres;
   if (!*root) 
   {
      new->lson = new->rson = NULL;
      *root = new;
      return NULL;
   }
   *root = splay_tree(*root, new->key);
   
   cmpres = new->key-(*root)->key;
   
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
// Function: NumTreeStore()
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

bool NumTreeStore(NumTree_p *root, long key, IntOrP val1, IntOrP val2)
{
   NumTree_p handle, new;

   handle = NumTreeCellAlloc();
   handle->key = key;
   handle->val1 = val1;
   handle->val2 = val2;
   
   new = NumTreeInsert(root, handle);

   if(new)
   {
      NumTreeCellFree(handle);
      return false;
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: NumTreeDebugPrint()
//
//   Print the tree in an unattractive but debug-friendly way.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

long NumTreeDebugPrint(FILE* out, NumTree_p tree, bool keys_only)
{
   long size;
   size = numtree_print(out, tree, keys_only, 0);
   fprintf(out, "Tree size: %ld\n", size);
   return size;
}


/*-----------------------------------------------------------------------
//
// Function: NumTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

NumTree_p NumTreeFind(NumTree_p *root, long key)
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
// Function: NumTreeExtractEntry()
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


NumTree_p NumTreeExtractEntry(NumTree_p *root, long key)
{
   NumTree_p x, cell;

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
// Function: NumTreeDeleteEntry()
//
//   Delete the entry with key key from the tree. 
//
// Global Variables: -
//
// Side Effects    : By NumTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool NumTreeDeleteEntry(NumTree_p *root, long key)
{
   NumTree_p cell;
   
   cell = NumTreeExtractEntry(root, key);
   if(cell)
   {
      NumTreeFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: NumTreeNodes()
//
//   Return the number of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long NumTreeNodes(NumTree_p root)
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


AVL_TRAVERSE_DEFINITION(NumTree, NumTree_p)



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


