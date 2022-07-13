/*-----------------------------------------------------------------------

File  : clb_objmaps.c

Author: Petar Vukmirovic

Contents

  Data structure for efficiently dealing with mapping a key
  to a value. You only need to provide a (total) comparison function
  on the keys and optionally a deleter function for keys.

Copyright 1998-2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> vr 23 jul 2021 16:47:42 CEST
    Built on top of objtrees.[ch]
-----------------------------------------------------------------------*/

#include "clb_objmaps.h"
#include "clb_objtrees.h"

struct objmap_node
{
   struct objmap_node *lson;
   struct objmap_node *rson;
   void* key;
   void* value;
};

typedef struct objmap_node PObjMap;

#define PObjMapNodeAlloc() (SizeMalloc(sizeof(PObjMap)))
#define PObjMapNodeFree(junk) SizeFree(junk, sizeof(PObjMap))

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

static PObjMap_p splay_tree(PObjMap_p tree, void* key,
                            ComparisonFunctionType cmpfun)
{
   PObjMap_p   left, right, tmp;
   PObjMap newnode;
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

/*-----------------------------------------------------------------------
//
// Function: do_extract_entry()
//
//   Find the entry with key key, remove it from the tree, rebalance
//   the tree, and return the pointer to the removed element. Return
//   NULL if no matching element exists. NB: Does not free the node.
//
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

PObjMap_p do_extract_entry(PObjMap_p *root, void* key,
                           ComparisonFunctionType cmpfun)
{
   PObjMap_p x, cell;

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



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PObjMapInsert()
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

PObjMap_p PObjMapInsert(PObjMap_p *root, PObjMap_p newnode,
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
// Function: PObjMapStore()
//
//   Stores a key value pair in the store. If a key already existed
//   in the tree, the old value is returned. Else, NULL is
//   returned. In either way, map is updated to store a mapping 
//   key -> value;
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void* PObjMapStore(PObjMap_p *root, void* key, void* value,
                   ComparisonFunctionType cmpfun)
{
   bool node_created;
   void** val_ref = PObjMapGetRef(root, key, cmpfun, &node_created);
   void* old_val = *val_ref;
   *val_ref = value;
   return !node_created ? old_val : NULL;
}

/*-----------------------------------------------------------------------
//
// Function: PObjMapGetRef()
//
//   Returns a reference to the value for the corresponding key. If the
//   key was previously not stored, new node is created and reference to
//   its "value" field is returned. updated_map is set to true if
//   a new node was created [if you are not interested in this
//   info, just pass NULL for updated_map].
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void** PObjMapGetRef(PObjMap_p *root, void* key, ComparisonFunctionType cmpfun, 
                    bool* updated_map)
{
   PObjMap_p handle, newnode;

   handle = PObjMapNodeAlloc();
   //printf("\nPObjMapStore: %p\n", handle);
   handle->key = key;
   handle->value = NULL;

   newnode = PObjMapInsert(root, handle, cmpfun);

   if(newnode)
   {
      PObjMapNodeFree(handle);
      if(updated_map)
      {
         *updated_map = false;
      }
      return &newnode->value;
   }
   else
   {
      if(updated_map)
      {
         *updated_map = true;
      }
      return &handle->value;
   }

}

/*-----------------------------------------------------------------------
//
// Function: PObjMapFind()
//
//   Finds a value associated to the key. If no such value exists,
//   NULL is returned. 
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void* PObjMapFind(PObjMap_p *root, void* key, ComparisonFunctionType cmpfun)
{
   if(*root)
   {
      *root = splay_tree(*root, key, cmpfun);
      if(cmpfun((*root)->key, key)==0)
      {
         return (*root)->value;
      }
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: PObjMapExtract()
//
//   Finds a value associated to the key, deletes it and returns it.
//   Returns NULL if no value is associated to the key. 
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void* PObjMapExtract(PObjMap_p *root, void* key,
                     ComparisonFunctionType cmpfun)
{
   PObjMap_p handle;
   void*   res = NULL;

   handle = do_extract_entry(root, key, cmpfun);
   if(handle)
   {
      res = handle->value;
      PObjMapNodeFree(handle);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: PObjTreeFreeWDeleter()
//
//   Free the tree using the functions that frees keys and values.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void PObjMapFreeWDeleter(PObjMap_p root, KeyValDelFun del_fun)
{
   if(root)
   {
      assert(!root->lson || !root->rson || root->lson != root->rson);
      PObjMapFreeWDeleter(root->lson, del_fun);
      PObjMapFreeWDeleter(root->rson, del_fun);
      del_fun(root->key, root->value);
      PObjMapNodeFree(root);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PObjTreeFreeWDeleter()
// 
//   Free the tree using the functions that free keys and values.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void dummy(void* a, void* b) {}

void PObjMapFree(PObjMap_p root)
{
   PObjMapFreeWDeleter(root, dummy);
}

/*-----------------------------------------------------------------------
//
// Function: PObjMapTraverseInit()
// 
//   Initialize the interator. Unlike other iterator initializers,
//   does not do memory managent on stacks.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

PStack_p PObjMapTraverseInit(PObjMap_p root, PStack_p stack)
{
   assert(stack);
   PStackReset(stack);
   while(root)
   {
      PStackPushP(stack, root);
      root = root->lson;
   }
   return stack;
}

/*-----------------------------------------------------------------------
//
// Function: PObjMapTraverseNext()
// 
//   Traverses the nodes and returns value stored in each node. If you want
//   to know the value of the key, use a non-NULL second argument,
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void* PObjMapTraverseNext(PStack_p state, void** key)
{
   if(PStackEmpty(state))
   {
      return NULL;
   }

   PObjMap_p handle = PStackPopP(state);
   if(key)
   {
      *key = handle->key;
   }
   void* res = handle->value;
   handle = handle->rson;
   while(handle)
   {
      PStackPushP(state, handle);
      handle = handle->lson;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SizeOfPObjNode()
// 
//   Size of one PObjMap node object.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/
inline size_t SizeOfPObjNode()
{
   return sizeof(PObjMap);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
