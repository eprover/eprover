/*-----------------------------------------------------------------------

File  : clb_intmap.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Functions implementing the multi-representation N_0->void* mapping
  data type.

  Copyright 2004 by the author.
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
// Function: switch_to_array()
//
//   Return true if representation should switch to array (because of
//   high density)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool switch_to_array(long old_min, long old_max, long new_key, long entries)
{
   printf("switch_to_array\n");
   long max_key = MAX(old_max, new_key);
   long min_key = MIN(old_min, new_key);

   if((entries * MIN_TREE_DENSITY) > (max_key-min_key))
   {
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: switch_to_tree()
//
//   Return true if representation should switch to tree (because of
//   low density)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool switch_to_tree(long old_min, long old_max, long new_key, long entries)
{
   printf("switch_to_tree\n");
   long max_key = MAX(old_max, new_key);
   long min_key = MIN(old_min, new_key);

   if((entries * MAX_TREE_DENSITY) < (max_key-min_key))
   {
      return true;
   }
   return false;
}

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

// static NumTree_p add_new_tree_node(IntMap_p map, long key, void* val)
// {
//    NumTree_p handle, check;
//    assert(map->type == IMTree);

//    handle = NumTreeCellAlloc();
//    handle->key = key;
//    handle->val1.p_val = val;
//    check = NumTreeInsert(&(map->values.tree), handle);
//    UNUSED(check); assert(!check);
//    map->entry_no++;

//    return handle;
// }


/*-----------------------------------------------------------------------
//
// Function: array_to_tree()
//
//   Convert a IntMap in array form to an equivalent one in tree
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void array_to_tree(IntMap_p map)
{
   printf("array_to_tree\n");
   PDRangeArr_p  tmp_arr;
   IntOrP        tmp_val;
   long          i;
   long          max_key = map->min_key;
   long          min_key = map->max_key;

   assert(map->type == IMArray);

   tmp_arr = map->values.array;
   map->values.tree = NULL;
   map->type = IMTree;
   map->entry_no = 0;

   for(i=PDRangeArrLowKey(tmp_arr); i<=map->max_key; i++)
   {
      tmp_val.p_val = PDRangeArrElementP(tmp_arr, i);
      if(tmp_val.p_val)
      {
         ArrayTreeStore(&(map->values.tree), i, tmp_val);
         map->entry_no++;
         max_key = i;
         min_key = MIN(min_key, i);
      }
   }
   map->max_key = max_key;
   map->min_key = MIN(min_key, max_key);
   PDRangeArrFree(tmp_arr);
   ArrayTreeDebugPrint(stdout, map->values.tree, false);
   //getchar();
}


/*-----------------------------------------------------------------------
//
// Function: tree_to_array()
//
//   Convert a IntMap in tree form to an equivalent one in array
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void tree_to_array(IntMap_p map)
{
   printf("tree_to_array\n");
   PDRangeArr_p  tmp_arr;
   long          max_key = map->min_key;
   long          min_key = map->max_key;
   PStack_p      tree_iterator;
   ArrayTree_p   handle;
   uint8_t       i;

   assert(map->type == IMTree);

   map->entry_no = 0;
   tmp_arr = PDRangeArrAlloc(map->min_key, IM_ARRAY_SIZE);
   ArrayTreeDebugPrint(stdout, map->values.tree, false);
   //getchar();
   tree_iterator = ArrayTreeTraverseInit(map->values.tree);
   while((handle = ArrayTreeTraverseNext(tree_iterator))) {
      for (i = 0; i < MAX_NODE_ARRAY_SIZE; i++) {
         if(handle->entries[i].p_val) {
            PDRangeArrAssignP(tmp_arr, (handle->key + i),
                                 handle->entries[i].p_val);
            map->entry_no++;
            max_key = handle->key + i;
            min_key = MIN(min_key, (handle->key + i));
         }
      }
   }
   // printf("Array:\n");
   // for (int i = 0; i < sizeof(tmp_arr); i++) {
   //    printf("i: %d\tvalue: %p\n", i, tmp_arr->array[i]);
   // }
   //  getchar();
   ArrayTreeTraverseExit(tree_iterator);
   ArrayTreeFree(map->values.tree);
   map->max_key = max_key;
   map->min_key = MIN(min_key, max_key);
   map->values.array = tmp_arr;
   map->type = IMArray;
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
   printf("IntMapAlloc\n");
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
   printf("IntMapFree\n");
   assert(map);

   switch(map->type)
   {
   case IMEmpty:
   case IMSingle:
         break;
   case IMArray:
         PDRangeArrFree(map->values.array);
         break;
   case IMTree:
         ArrayTreeFree(map->values.tree);
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
   printf("IntMapGetVal -> key: %ld\n", key);
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
   case IMArray:
      // printPDRangeArray(map->values.array);
         if(key <= map->max_key)
         {
            res = PDRangeArrElementP(map->values.array, key);
         }
         break;
   case IMTree:
         if(key <= map->max_key)
         {
            ArrayTree_p entry = ArrayTreeFind(&(map->values.tree), key);
            if(entry) {
               res = ArrayTreeGetEntry(entry, key);
            }
         }
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
   printf("IntMapGetRef -> key: %ld\n", key);
   void        **res = NULL;
   void        *val;
   ArrayTree_p handle;
   IntOrP      tmp;

   assert(map);

   /* printf("IntMapGetRef(%p,%ld) type %d, entries=%ld,
      maxkey=%ld...\n", map, key, map->type,map->entry_no,
      map->max_key);
   */
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
         else if(switch_to_array(key, map->min_key, map->max_key, 2))
         {
            map->type = IMArray;
            val = map->values.value;
            map->values.array = PDRangeArrAlloc(MIN(key, map->max_key),
                                                IM_ARRAY_SIZE);
            PDRangeArrAssignP(map->values.array, map->max_key, val);
            PDRangeArrAssignP(map->values.array, key, NULL);
            res = &(PDRangeArrElementP(map->values.array, key));
            map->entry_no = 2;
         }
         else
         {
            map->type = IMTree;
            val = map->values.value;
            map->values.tree = NULL;
            tmp.p_val = val;
            ArrayTreeStore(&(map->values.tree), map->max_key, tmp);
            handle = ArrayTreeStore(&(map->values.tree), key, (IntOrP)NULL);
            res = ArrayTreeGetEntryRef(&handle, key);
            map->entry_no = 2;
         }
         map->min_key = MIN(map->min_key, key);
         map->max_key = MAX(key, map->max_key);
         break;
   case IMArray:
         if(((key > map->max_key)||(key<map->min_key)) &&
            switch_to_tree(map->min_key, map->max_key, key, map->entry_no+1))
         {
            array_to_tree(map);
            res = IntMapGetRef(map, key);
         }
         else
         {
            res = &(PDRangeArrElementP(map->values.array, key));
            if(!(*res))
            {
               map->entry_no++;
            }
         }
         map->min_key=MIN(map->min_key, key);
         map->max_key=MAX(map->max_key, key);
         break;
   case IMTree:
      handle = ArrayTreeFind(&(map->values.tree), key);
      ArrayTreeDebugPrint(stdout, handle, false);
      if (handle) {
         if (ArrayTreeGetEntry(handle, key)) {
            res = ArrayTreeGetEntryRef(&handle, key);
         } else {
            map->max_key=MAX(map->max_key, key);
            map->min_key=MIN(map->min_key, key);
            res = ArrayTreeGetEntryRef(&handle, key);
            map->entry_no++;
         }
      } else {
         if(switch_to_array(map->min_key, map->max_key, key, map->entry_no+1)) {
            tree_to_array(map);
            res = IntMapGetRef(map, key);
         } else {
            handle = ArrayTreeStore(&(map->values.tree), key, (IntOrP)NULL);
            map->max_key=MAX(map->max_key, key);
            map->min_key=MIN(map->min_key, key);
            res = ArrayTreeGetEntryRef(&handle, key);
            map->entry_no++;
         }
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
   printf("IntMapAssign -> key: %ld\n", key);
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
   printf("IntMapDelKey -> key: %ld\n", key);
   void* res = NULL;
   ArrayTree_p handle;

   assert(map);

   switch(map->type)
   {
   case IMEmpty:
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
   case IMArray:
         if(key > map->max_key)
         {
            res = NULL;
         }
         /* if key == map->max_key optionally do something (shrink
          * array, recompute map->max_key - likely unnecessary at
          * least for my current applications */
         else if((res = PDRangeArrElementP(map->values.array, key)))
         {
            PDRangeArrAssignP(map->values.array, key, NULL);
            map->entry_no--;
            if(switch_to_tree(map->min_key, map->max_key, map->max_key, map->entry_no))
            {
               array_to_tree(map);
            }
         }
         break;
   case IMTree:
      handle = ArrayTreeFind(&(map->values.tree), key);
      if(handle) {
         res = ArrayTreeExtractEntry(&handle, key);
         if (res) {
            map->entry_no--;
            if (handle != map->values.tree) {
               map->values.tree = handle;
            }
            if(key == map->max_key) {
               if(map->values.tree) {
                  map->max_key = ArrayTreeMaxKey(map->values.tree);
                  printf("maxKey: %ld\n", map->max_key);
               }
               else {
                  map->max_key = map->min_key;
               }

               if(switch_to_array(map->min_key, map->max_key,
                                    map->max_key, map->entry_no)) {
                  tree_to_array(map);
               }
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
   printf("IntMapIterAlloc -> lower_key: %ld, upper_key: %ld\n", lower_key, upper_key);
   IntMapIter_p handle = IntMapIterCellAlloc();
   TreeIter_p iter = malloc(sizeof(ArrayTreeIter));
   iter->limit = 0;
   iter->key = 0;

   handle->map = map;
   if(map)
   {
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
      case IMArray:
            handle->admin_data.current = lower_key;
            break;
      case IMTree:
            handle->admin_data.iterator =
               ArrayTreeLimitedTraverseInit(map->values.tree, &iter, lower_key);
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
   printf("IntMapIterFree\n");
   assert(junk);

   if(junk->map)
   {
      switch(junk->map->type)
      {
      case IMEmpty:
      case IMSingle:
      case IMArray:
            break;
      case IMTree:
            PStackFree(junk->admin_data.iterator->tree_iter);
            free(junk->admin_data.iterator);
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

   fprintf(out, "# ==== IntMapType %d Size = %ld\n", map->type, IntMapStorage(map));
   for(val=IntMapIterNext(iter, &key); val; val=IntMapIterNext(iter, &key))
   {
      fprintf(out, "# %5ld : %p\n", key, val);
   }
   fprintf(out, "# ==== IntMap End\n");

   IntMapIterFree(iter);
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
