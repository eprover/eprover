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

#include "clb_objmaps.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


typedef struct {
   void* key;
   void* value;
   ComparisonFunctionType comparator;
} kv_pair;

#define AllocKVPair() (SizeMalloc(sizeof(kv_pair)))
#define FreeKVPair(junk) SizeFree(junk, sizeof(kv_pair))



/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: mk_pair()
//
//   Constructs a key value pair with a given comparison function.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

static inline kv_pair* mk_pair(void* key, void* val, ComparisonFunctionType f)
{
   kv_pair* p = AllocKVPair();
   p->key = key;
   p->value = val;
   p->comparator = f;
   return p;
}

/*-----------------------------------------------------------------------
//
// Function: objtree_store_fun()
//
//   Function that uses the underlying comparator on keys to
//   store pairs in ObjTree.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

int objtree_store_fun(const void* kv1, const void* kv2)
{
   kv_pair* x = (kv_pair*) kv1;
   kv_pair* y = (kv_pair*) kv2;
   assert(x->comparator == y->comparator);

   return x->comparator(x->key, y->key);
}

/*-----------------------------------------------------------------------
//
// Function: dummy_del_fun()
//
//   Deletion function that does nothing.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void dummy_del_fun(void* p)
{
   return;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PObjMapStore()
//
//   Stores a key value pair in the store.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void* PObjMapStore(PObjMap_p *root, void* key, void* value,
                   ComparisonFunctionType cmpfun)
{
   kv_pair* p = mk_pair(key, value, cmpfun);
   kv_pair* res = PTreeObjStore(root, p, objtree_store_fun);
   return res ? res->value : NULL;
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
   kv_pair p = {.key=key, .value=NULL, .comparator=cmpfun};
   kv_pair* res = PTreeObjFindObj(root, &p, objtree_store_fun);
   return res ? res->value : NULL;
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
   kv_pair p = {.key=key, .value=NULL, .comparator=cmpfun};
   kv_pair* res = PTreeObjExtractObject(root, &p, objtree_store_fun);
   if(res)
   {
      void* value = res->value;
      FreeKVPair(res);
      return value;
   }
   else
   {
      return NULL;
   }
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

void PObjMapFreeWDeleter(PObjMap_p root, ObjDelFun del_key_fun, ObjDelFun del_val_fun)
{
   if(root)
   {
      PObjMapFreeWDeleter(root->lson, del_key_fun, del_val_fun);
      PObjMapFreeWDeleter(root->rson, del_key_fun, del_val_fun);
      kv_pair* p = root->key;
      del_key_fun(p->key);
      del_val_fun(p->value);
      FreeKVPair(p);
      PTreeCellFree(root);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PObjTreeFreeWDeleter()
// 
//   Free the tree using the functions that do nothing on keys
//   and values.
//
// Global Variables: -
//
// Side Effects    : Changes the tree.
//
/----------------------------------------------------------------------*/

void PObjMapFree(PObjMap_p root)
{
   PObjMapFreeWDeleter(root, dummy_del_fun, dummy_del_fun);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
