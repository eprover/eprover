/*-----------------------------------------------------------------------

  File  : clb_ptrees.c

  Author: Stephan Schulz

  Contents

  Functions for pointer storing SPLAY trees.

  Copyright 1998-2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Dec 17 21:17:34 MET 1997

  -----------------------------------------------------------------------*/

#include "clb_ptrees.h"


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

static PTree_p splay_ptree(PTree_p tree, void* key)
{
   PTree_p   left, right, tmp;
   PTreeCell newnode;

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
      if(PLesser(key, tree->key))
      {
         if(!tree->lson)
         {
            break;
         }
         if(PLesser(key, tree->lson->key))
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
      else if(PGreater(key, tree->key))
      {
         if (!tree->rson)
         {
            break;
         }
         if(PGreater(key, tree->rson->key))
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
// Function: PTreeCellAllocEmpty()
//
//   Allocate a empty, initialized PTreeCell. Pointers to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PTree_p PTreeCellAllocEmpty(void)
{
   PTree_p handle = PTreeCellAlloc();

   handle->lson = handle->rson = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeFree()
//
//   Free a PTree (including the keys, but not potential objects
//   pointed to in the val fields
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PTreeFree(PTree_p junk)
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
         PTreeCellFree(junk);
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PTreeInsert()
//
//   If an entry with key *newnode->key exists in the tree return a
//   pointer to it. Otherwise insert *newnode in the tree and return
//   NULL. Will splay the tree!
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

PTree_p PTreeInsert(PTree_p *root, PTree_p newnode)
{
   if(!*root)
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_ptree(*root, newnode->key);

   if(PLesser(newnode->key, (*root)->key))
   {
      newnode->lson = (*root)->lson;
      newnode->rson = *root;
      (*root)->lson = NULL;
      *root = newnode;
      return NULL;
   }
   else if(PGreater(newnode->key, (*root)->key))
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
// Function: PTreeStore()
//
//   Insert a cell with given key into the tree. Return false if an
//   entry for this key exists, true otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

bool PTreeStore(PTree_p *root, void* key)
{
   PTree_p handle, newnode;

   handle = PTreeCellAlloc();
   //printf("\nPTreeStore: %p\n", handle);
   handle->key = key;

   newnode = PTreeInsert(root, handle);
   //printf("\nNewnode, handle: %p, %p\n", newnode, handle);

   if(newnode)
   {
      PTreeCellFree(handle);
      return false;
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PTree_p PTreeFind(PTree_p *root, void* key)
{
   if(*root)
   {
      *root = splay_ptree(*root, key);
      if(PEqual((*root)->key, key))
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeFindBinary()
//
//   Find an entry by simple binary search. This does not reorganize
//   the tree, otherwise it is inferior to PTreeFind()!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PTree_p PTreeFindBinary(PTree_p root, void* key)
{
   while(root)
   {
      if(PLesser(key, root->key))
      {
         root = root->lson;
      }
      else if(PGreater(key, root->key))
      {
         root = root->rson;
      }
      else
      {
         break;
      }
   }
   return root;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeExtractEntry()
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


PTree_p PTreeExtractEntry(PTree_p *root, void* key)
{
   PTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_ptree(*root, key);
   if(PEqual(key, (*root)->key))
   {
      if (!(*root)->lson)
      {
         x = (*root)->rson;
      }
      else
      {
         x = splay_ptree((*root)->lson, key);
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
// Function: PTreeExtractKey()
//
//   Extract the entry with key key, delete the PTree-Node and return
//   the key.
//
// Global Variables: -
//
// Side Effects    : Memory operations, by PTreeExtractEntry()
//
/----------------------------------------------------------------------*/

void* PTreeExtractKey(PTree_p *root, void* key)
{
   PTree_p handle;
   void*   res = NULL;

   handle = PTreeExtractEntry(root, key);
   if(handle)
   {
      res = handle->key;
      PTreeCellFree(handle);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeExtractRootKey()
//
//   Extract the root node of the tree, delete it and return the
//   key. Return NULL if the tree is empty.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

void* PTreeExtractRootKey(PTree_p *root)
{
   if(*root)
   {
      return PTreeExtractKey(root, (*root)->key);
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeDeleteEntry()
//
//   Delete the entry with key key from the tree. Return true, if the
//   key existed, false otherwise.
//
// Global Variables: -
//
// Side Effects    : By PTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool PTreeDeleteEntry(PTree_p *root, void* key)
{
   PTree_p cell;

   cell = PTreeExtractEntry(root, key);
   if(cell)
   {
      PTreeCellFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeMerge()
//
//   Merge the two trees, i.e. destroy the second one and add its
//   elements to the first one. Return true if *root gains a new
//   element.
//
// Global Variables: -
//
// Side Effects    : Changes both trees.
//
/----------------------------------------------------------------------*/

bool PTreeMerge(PTree_p *root, PTree_p add)
{
   PStack_p stack = PStackAlloc();
   PTree_p  tmp;
   bool     res = false;

   //printf("Allocated %p\n", stack);
   PStackPushP(stack, add);

   while(!PStackEmpty(stack))
   {
      add = PStackPopP(stack);
      if(add)
      {
         PStackPushP(stack, add->lson);
         PStackPushP(stack, add->rson);
         tmp = PTreeInsert(root, add);
         if(tmp)
         {
            PTreeCellFree(add);
         }
         else
         {
            res = true;
         }
      }
   }
   //printf("Freeing %p\n", stack);
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeInsertTree()
//
//   Insert the elements stored in add into *root. The tree at add
//   remains unchanged.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void PTreeInsertTree(PTree_p *root, PTree_p add)
{
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, add);

   while(!PStackEmpty(stack))
   {
      add = PStackPopP(stack);
      if(add)
      {
         PStackPushP(stack, add->lson);
         PStackPushP(stack, add->rson);
         PTreeStore(root, add->key);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: PTreeNodes()
//
//   Return the number of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PTreeNodes(PTree_p root)
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
// Function: PTreeDebugPrint()
//
//   Print the keys stored in the tree. Returns number of nodes (why
//   not ?).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

long PTreeDebugPrint(FILE* out, PTree_p root)
{
   PStack_p stack = PStackAlloc();
   long     res   = 0;

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      root = PStackPopP(stack);
      if(root)
      {
         if(res % 10 == 0)
         {
            fprintf(out, "\n"COMCHAR);
         }
         fprintf(out, " %7p", root->key);
         PStackPushP(stack, root->lson);
         PStackPushP(stack, root->rson);
         res++;
      }
   }
   PStackFree(stack);
   fprintf(out, "\n");

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PStackToPTree()
//
//   Interprete a stack as a list of pointers and insert these
//   pointers into the tree at *root. Returns number of new elements
//   in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PStackToPTree(PTree_p *root, PStack_p stack)
{
   PStackPointer i;
   long          res = 0;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      if(PTreeStore(root, PStackElementP(stack,i)))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeToPStack()
//
//   Push all the keys in the tree onto the stack (in arbitrary
//   order). Return number of values pushed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PTreeToPStack(PStack_p target_stack, PTree_p root)
{
   long res = 0;
   PStack_p stack = PStackAlloc();
   PTree_p handle;

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle)
      {
         PStackPushP(target_stack, handle->key);
         res++;
         PStackPushP(stack,handle->lson);
         PStackPushP(stack,handle->rson);
      }
   }
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeSharedElement()
//
//   If there exists an element common in both trees, return the first
//   one found. Otherwise return NULL. This iterates over the elements
//   of the second tree and searches in the first, so make the second
//   one smaller if you have a choice.
//
// Global Variables: -
//
// Side Effects    : Reorganizes tree1!
//
/----------------------------------------------------------------------*/

void* PTreeSharedElement(PTree_p *tree1, PTree_p tree2)
{
   PStack_p stack = PStackAlloc();
   PTree_p handle, tmp;
   void* res = NULL;

   PStackPushP(stack, tree2);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle)
      {
         tmp = PTreeFind(tree1, handle->key);
         if(tmp)
         {
            res = tmp->key;
            break;
         }
         PStackPushP(stack,handle->lson);
         PStackPushP(stack,handle->rson);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeIntersection()
//
//   Compute the intersection of the two PTrees and return it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PTree_p PTreeIntersection(PTree_p tree1, PTree_p tree2)
{
   PStack_p stack = PStackAlloc();
   PTree_p handle, tmp, res=NULL;

   PStackPushP(stack, tree2);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle)
      {
         tmp = PTreeFindBinary(tree1, handle->key);
         if(tmp)
         {
            PTreeStore(&res, handle->key);
         }
         PStackPushP(stack,handle->lson);
         PStackPushP(stack,handle->rson);
      }
   }
   PStackFree(stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeIntersection()
//
//   Make tree1 the intersection of tree1 and tree2. Does not change
//   tree 2. Return the number of keys removed.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long PTreeDestrIntersection(PTree_p *tree1, PTree_p tree2)
{
   PTree_p tmp = NULL;
   void* key;
   long res = 0;

   while((key = PTreeExtractRootKey(tree1)))
   {
      if(PTreeFindBinary(tree2, key))
      {
         PTreeStore(&tmp, key);
      }
      else
      {
         res++;
      }
   }
   assert(!(*tree1));
   *tree1 = tmp;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeCopy()
//
//   Return a Ptree that stores the same elements as tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

PTree_p PTreeCopy(PTree_p tree)
{
   PTree_p res = NULL, handle;
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, tree);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle)
      {
         PTreeStore(&res, handle->key);
         PStackPushP(stack,handle->lson);
         PStackPushP(stack,handle->rson);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeEquiv()
//
//   Determin if two PTrees contain the same pointers.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool PTreeEquiv(PTree_p t1, PTree_p t2)
{
   bool res =  true;
   PStack_p iter1 = PTreeTraverseInit(t1);
   PStack_p iter2 = PTreeTraverseInit(t2);

   PTree_p  handle1, handle2;

   while((handle1 = PTreeTraverseNext(iter1)))
   {
      handle2 = PTreeTraverseNext(iter2);
      if(!handle2 || (handle1->key != handle2->key))
      {
         res = false;
         break;
      }
   }
   if(!handle1)
   {
      handle2= PTreeTraverseNext(iter2);
      if(handle2)
      {
         res = false;
      }
   }
   PTreeTraverseExit(iter2);
   PTreeTraverseExit(iter1);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeIsSubset()
//
//   Determine if pointers stored in sub are a subset of pointers
//   stored in super.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool PTreeIsSubset(PTree_p sub, PTree_p *super)
{
   bool res =  true;
   PStack_p iter = PTreeTraverseInit(sub);

   PTree_p  handle;

   while((handle = PTreeTraverseNext(iter)))
   {
      if(!PTreeFind(super, handle->key))
      {
         res = false;
         break;
      }
   }
   PTreeTraverseExit(iter);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeVisitInOrder()
//
//   Apply function visitor to every key stored in PTree t. Nodes will be
//   visited as in inorder traversal. "arg" is an additional (first)
//   arg to the visitor function.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PTreeVisitInOrder(PTree_p t, void (*visitor)(void*, void*), void* arg)
{
   PStack_p iter = PTreeTraverseInit(t);
   PTree_p  handle = NULL;

   while((handle = PTreeTraverseNext(iter)))
   {
      visitor(arg, handle->key);
   }

   PTreeTraverseExit(iter);
}


AVL_TRAVERSE_DEFINITION(PTree, PTree_p)


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
