/*-----------------------------------------------------------------------

File  : clb_intmap.c

Authors: Stephan Schulz (schulz@eprover.org), Albert Eisfeld

Contents

  Functions implementing the multi-representation N_0->void* mapping
  data type.

  Copyright 2004, 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Dec 27 17:34:48 CET 2004
    New

-----------------------------------------------------------------------*/

#include "clb_intmap.h"



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
// Function: add_new_tree_node()
//
//   Add a *new* key node to a IntMap in tree form and return its
//   address. Assertion fail, if key is not new. Increases element
//   count!
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static NumArrTree_p add_new_tree_node(IntMap_p map, long key, void* val)
{
   NumArrTree_p res;
   assert(map->type == IMArrTree);
   res = NumArrTreeInsertKeyValPair(&(map->values.tree), key, val);
   UNUSED(res); assert(res);
   map->entry_no++;

   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: IntMapAlloc()
//
//   Allocate an empty int mapper.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

IntMap_p IntMapAlloc(void)
{
   IntMap_p handle = IntMapCellAlloc();

   handle->type = IMEmpty;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapFree()
//
//   Free an int mapper (does _not_ free pointed-to elements).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void IntMapFree(IntMap_p map)
{
   assert(map);

   switch(map->type)
   {
   case IMEmpty:
   case IMSingle:
         break;
   case IMArrTree:
         NumArrTreeFree(map->values.tree);
         break;
   default:
         assert(false && "Unknown IntMap type.");
   }
   IntMapCellFree(map);
}


/*-----------------------------------------------------------------------
//
// Function: IntMapGetVal()
//
//   Given a key, return the associated value or NULL, if no suitable
//   key/value pair exists.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void* IntMapGetVal(IntMap_p map, long key)
{
   void* res = NULL;

   if(!map)
   {
      return NULL;
   }

   switch(map->type)
   {
   case IMEmpty:
         break;
   case IMSingle:
         if(map->max_key == key)
         {
            res = map->values.value;
         }
         break;
   case IMArrTree:
         if(key <= map->max_key)
         {
            NumArrTree_p entry = NumArrTreeFindNode(&(map->values.tree), key);
            if(entry)
            {
               res = entry->vals[key & (NUMARRTREEVALUES - 1)].p_val; 
            }
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapGetRef()
//
//   Get a reference to the address of the value of a key/value
//   pair. Note that this always creates the key value pair (with
//   empty value) if it does not exist yet.
//
// Global Variables: -
//
// Side Effects    : May reorganize the map.
//
/----------------------------------------------------------------------*/


void** IntMapGetRef(IntMap_p map, long key)
{
   void      **res = NULL;
   void      *val;
   NumArrTree_p handle;
   IntOrP tmp;
   long index = key & (NUMARRTREEVALUES - 1);

   assert(map);

   switch(map->type)
   {
   case IMEmpty:
         map->type = IMSingle;
         map->max_key = key;
         map->min_key = key;
         map->values.value = NULL;
         res = &(map->values.value);
         map->entry_no = 1;
         break;
   case IMSingle:
         if(key == map->max_key)
         {
            res = &(map->values.value);
         }
         else
         {
            map->type = IMArrTree;
            val = map->values.value;
            map->values.tree = NULL;
            tmp.p_val = val;
            NumArrTreeStoreNode(&(map->values.tree), map->max_key, tmp);
            handle = add_new_tree_node(map, key, NULL);
            res = &(handle->vals[index].p_val);
            map->entry_no = 2;
         }
         map->min_key = MIN(map->min_key, key);
         map->max_key = MAX(key, map->max_key);
         break;
   case IMArrTree:
         handle = NumArrTreeFindNode(&(map->values.tree), key);
         if(handle)
         {
            res = &(handle->vals[index].p_val);
            /* if key/value pair does not exist yet, treat it like you are 
               creating a pair */
            if(!(*res)) 
            {
               map->entry_no++;
               map->max_key=MAX(map->max_key, key);
               map->min_key=MIN(map->min_key, key);
            }
         }
         else
         {
            handle = add_new_tree_node(map, key, NULL);
            map->max_key=MAX(map->max_key, key);
            map->min_key=MIN(map->min_key, key);
            res = &(handle->vals[index].p_val);
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
   }

   assert(res);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapAssign()
//
//   Add key/value pair to map, overriding any previous association.
//
// Global Variables: -
//
// Side Effects    : Changes map, may trigger reorganization
//
/----------------------------------------------------------------------*/

void IntMapAssign(IntMap_p map, long key, void* value)
{
   void** ref;

   assert(map);

   ref  = IntMapGetRef(map, key);
   *ref = value;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapDelKey()
//
//   Delete a key/value association. If there was one, return the
//   value, otherwise return NULL.
//
//   **Currently, arrays never shrink. This might be worth
//   **changing (unlikely, though).
//
// Global Variables: -
//
// Side Effects    : May reorganize map
//
/----------------------------------------------------------------------*/

void* IntMapDelKey(IntMap_p map, long key)
{

   void* res = NULL;
   NumArrTree_p  handle;

   assert(map);

   switch(map->type)
   {
   case IMEmpty:
         //  Rauswerfen, da Redundanz?
         res = NULL;
         break;
   case IMSingle:
         if(key == map->max_key)
         {
            res = map->values.value;
            map->type = IMEmpty;
            map->entry_no = 0;
         }
         break;
   case IMArrTree:
      handle = NumArrTreePrepDel(&(map->values.tree), key);
         if(handle)
         {
            map->entry_no--;
            res = handle->vals[key & (NUMARRTREEVALUES - 1)].p_val;
            if(!(NumArrTreeNodeSingleton(handle, key & (NUMARRTREEVALUES - 1))))
            {
               // Setting the val NULL since it's not actually removed
               // in NumArrTreePrepDel()
               handle->vals[key & (NUMARRTREEVALUES - 1)].p_val = NULL;
            }
            if((handle->key + (key & (NUMARRTREEVALUES - 1))) == map->max_key)
            {
               if(map->values.tree)
               {
                  map->max_key = NumArrTreeMaxKey(NumArrTreeMaxNode(map->values.tree));
               }
               else
               {
                  map->max_key = map->min_key;
               }
            }
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
         break;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: IntMapIterAlloc()
//
//   Allocate an iterator object iterating over key range lower_key to
//   upper_key (both inclusive) in map. This is only valid as long as
//   no new key is introduced or old key is deleted.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

IntMapIter_p IntMapIterAlloc(IntMap_p map, long lower_key, long upper_key)
{
   IntMapIter_p handle = IntMapIterCellAlloc();

   handle->map = map;
   if(map)
   {
      handle->last_seen_key = MAX(lower_key, map->min_key) - 1;
      handle->lower_key = MAX(lower_key, map->min_key);
      handle->upper_key = MIN(upper_key, map->max_key);

      switch(map->type)
      {
      case IMEmpty:
            break;
      case IMSingle:
            handle->admin_data.seen = true;
            if((map->max_key >= lower_key) && (map->max_key <= upper_key))
            {
               handle->admin_data.seen = false;
            }
            break;
      case IMArrTree:
            handle->admin_data.tree_iter =
               NumArrTreeLimitedTraverseInit(map->values.tree, lower_key);
            break;
      default:
            assert(false && "Unknown IntMap type.");
            break;
      }
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: IntMapIterFree()
//
//   Free an IntMapIterator.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void IntMapIterFree(IntMapIter_p junk)
{
   assert(junk);

   if(junk->map)
   {
      switch(junk->map->type)
      {
      case IMEmpty:
      case IMSingle:
         break;
      case IMArrTree:
            PStackFree(junk->admin_data.tree_iter);
         break;
      default:
            assert(false && "Unknown IntMap type.");
            break;
      }
   }
   IntMapIterCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: IntMapDebugPrint()
//
//   Print an intmap datatype as a list of key:value pairs.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void IntMapDebugPrint(FILE* out, IntMap_p map)
{
   IntMapIter_p iter = IntMapIterAlloc(map, LONG_MIN, LONG_MAX);
   void* val;
   long  key = 0;

   fprintf(out, COMCHAR" ==== IntMapType %d Size = %ld\n", map->type, IntMapStorage(map));
   for(val=IntMapIterNext(iter, &key); val; val=IntMapIterNext(iter, &key))
   {
      fprintf(out, COMCHAR" %5ld : %p\n", key, val);
   }
   fprintf(out, COMCHAR" ==== IntMap End\n");

   IntMapIterFree(iter);
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
