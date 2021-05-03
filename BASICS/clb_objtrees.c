/*-----------------------------------------------------------------------

File  : clb_objtrees.c

Author: Stephan Schulz

Contents

  Functions for object storing SPLAY trees.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 15 16:43:21 MET 1999
    Imported from clb_ptrees.c

-----------------------------------------------------------------------*/

#include "clb_objtrees.h"



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

static PObjTree_p splay_tree(PObjTree_p tree, void* key,
           ComparisonFunctionType cmpfun)
{
   PObjTree_p   left, right, tmp;
   PTreeCell newnode;
   int       cmpres;

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
      cmpres = cmpfun(key, tree->key);
      if (cmpres < 0)
      {
    if(!tree->lson)
    {
       break;
    }
    if(cmpfun(key, tree->lson->key) < 0)
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
    if(cmpfun(key, tree->rson->key) > 0)
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
// Function: PTreeObjInsert()
//
//   If an entry with cmpfun(*root->key, newnode->key) == 0  exists in the
//   tree return a pointer to it. Otherwise insert *newnode in the tree
//   and return NULL. Will splay the tree!
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

PObjTree_p PTreeObjInsert(PObjTree_p *root, PObjTree_p newnode,
          ComparisonFunctionType cmpfun)
{
   int cmpres;
   if (!*root)
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode->key, cmpfun);

   cmpres = cmpfun(newnode->key, (*root)->key);

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
// Function: PTreeObjStore()
//
//   Store object in the tree. If an object that is equal to obj
//   already exists in the tree, return it, otherwise return NULL.
//
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

void* PTreeObjStore(PObjTree_p *root, void* key,
          ComparisonFunctionType cmpfun)
{
   PObjTree_p handle, newnode;

   handle = PTreeCellAlloc();
   //printf("\nPTreeObjStore: %p\n", handle);
   handle->key = key;

   newnode = PTreeObjInsert(root, handle, cmpfun);

   if(newnode)
   {
      PTreeCellFree(handle);
      return newnode->key;
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeObjFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PObjTree_p PTreeObjFind(PObjTree_p *root, void* key, ComparisonFunctionType
           cmpfun)
{
   if(*root)
   {
      *root = splay_tree(*root, key, cmpfun);
      if(cmpfun((*root)->key, key)==0)
      {
    return *root;
      }
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeObjFindObj()
//
//   Find and return object matching key (if any), return NULL if
//   none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void* PTreeObjFindObj(PObjTree_p *root, void* key,
                      ComparisonFunctionType cmpfun)
{
   PObjTree_p node = PTreeObjFind(root, key, cmpfun);

   if(node)
   {
      return node->key;
   }
   return NULL;
}



/*-----------------------------------------------------------------------
//
// Function: PTreeObjFindBinary()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists. Does not reorganize the tree!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PObjTree_p PTreeObjFindBinary(PObjTree_p root, void* key, ComparisonFunctionType
            cmpfun)
{
   int cmpres;

   while(root)
   {
      cmpres = cmpfun(key, root->key);
      if(cmpres < 0)
      {
    root = root->lson;
      }
      else if(cmpres > 0)
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
// Function: PTreeObjExtractEntry()
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

PObjTree_p PTreeObjExtractEntry(PObjTree_p *root, void* key,
              ComparisonFunctionType cmpfun)
{
   PObjTree_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key, cmpfun);
   if(cmpfun(key, (*root)->key)==0)
   {
      if (!(*root)->lson)
      {
    x = (*root)->rson;
      }
      else
      {
    x = splay_tree((*root)->lson, key, cmpfun);
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
// Function: PTreeObjExtractObject()
//
//   Extract the entry object, delete the PTree-Node and return
//   the pointer to the object.
//
// Global Variables: -
//
// Side Effects    : Memory operations, by PTreeExtractEntry()
//
/----------------------------------------------------------------------*/

void* PTreeObjExtractObject(PObjTree_p *root, void* key,
             ComparisonFunctionType cmpfun)
{
   PObjTree_p handle;
   void*   res = NULL;

   handle = PTreeObjExtractEntry(root, key, cmpfun);
   if(handle)
   {
      res = handle->key;
      PTreeCellFree(handle);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PTreeObjExtractRootObject()
//
//   Extract the root node of the tree, delete it and return the
//   key. Return NULL if the tree is empty.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

void* PTreeObjExtractRootObject(PObjTree_p *root, ComparisonFunctionType
            cmpfun)
{
   if(*root)
   {
      return PTreeObjExtractObject(root, (*root)->key, cmpfun);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PTreeObjMerge()
//
//   Merge the two trees, i.e. destroy the second one and add its
//   element to the first one.
//
// Global Variables: -
//
// Side Effects    : Changes both trees.
//
/----------------------------------------------------------------------*/

void PTreeObjMerge(PObjTree_p *root, PObjTree_p add, ComparisonFunctionType
      cmpfun)
{
   PStack_p stack = PStackAlloc();
   PObjTree_p  res;

   PStackPushP(stack, add);

   while(!PStackEmpty(stack))
   {
      add = PStackPopP(stack);
      if(add)
      {
    PStackPushP(stack, add->lson);
    PStackPushP(stack, add->rson);
    res = PTreeObjInsert(root, add, cmpfun);
    UNUSED(res); assert(!res); /* Pointers should never be in two trees at
                                     once for my intended application */
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: PObjTreeFree()
//
//   Free a PObjTree, including the objects.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PObjTreeFree(PObjTree_p root, ObjDelFun del_fun)
{
   if(root)
   {
      PObjTreeFree(root->lson, del_fun);
      PObjTreeFree(root->rson, del_fun);
      del_fun(root->key);
      PTreeCellFree(root);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PObjTreeNodes()
//
//   Return the number of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PObjTreeNodes(PObjTree_p root)
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
// Function: DummyObjDelFun()
//
//   Do nothing, with a pointer ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DummyObjDelFun(void* Junk)
{
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
