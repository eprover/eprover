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

//DF-STOP
#ifdef MEASURE_EMPTY
long countEmpty = 0;
#endif

#ifdef MEASURE_INT
long countInt = 0;
#endif

#ifdef MEASURE_ARRAY
long countArray = 0;
#endif

#ifdef MEASURE_LIST
long countList = 0;
#endif

#ifdef MEASURE_DELETE
long countDelete = 0;
long countDeleteEmpty = 0;
long countDeleteInt = 0;
long countDeleteArray = 0;
long countDeleteList = 0;
#endif
//DF-START

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
   long max_key = MAX(old_max, new_key);
   long min_key = MIN(old_min, new_key);

   if((entries * MIN_LIST_DENSITY) > (max_key-min_key))
   {
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: switch_to_list()
//
//   Return true if representation should switch to list (because of
//   low density)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool switch_to_list(long old_min, long old_max, long new_key, long entries)
{
   long max_key = MAX(old_max, new_key);
   long min_key = MIN(old_min, new_key);

   if((entries * MAX_LIST_DENSITY) < (max_key-min_key))
   {
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: add_new_list_node()
//
//   Add a *new* key node to a IntMap in list form and return its
//   address. Assertion fail, if key is not new. Increases element
//   count!
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static node_p add_new_list_node(IntMap_p map, long key, void* val)
{
   node_p handle, check;
   assert(map->type == IMList);

   handle = SkipListNodeAlloc();
   handle->key = key;
   handle->value.p_val = val;
   check = SkipListInsert(map->values.list, handle);
   UNUSED(check); assert(!check);
   map->entry_no++;

   return handle;
}

/////
/*-----------------------------------------------------------------------
//
// Function: array_to_list()
//
//   Convert a IntMap in array form to an equivalent one in list
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void array_to_list(IntMap_p map)
{
   PDRangeArr_p  tmp_arr;
   IntOrP        tmp_val;
   long          i;
   long          max_key = map->min_key;
   long          min_key = map->max_key;

   assert(map->type == IMArray);
   tmp_arr = map->values.array;

   map->values.list = initSkiplist();
   map->type = IMList;
   map->entry_no = 0;

   for(i=PDRangeArrLowKey(tmp_arr); i<=map->max_key; i++)
   {
      tmp_val.p_val = PDRangeArrElementP(tmp_arr, i);
      if(tmp_val.p_val)
      {
//         printf("\t Array to tree \t :%d\n", i);
         SkipListStore(map->values.list, i, tmp_val);
         map->entry_no++;
         max_key = i;
         min_key = MIN(min_key, i);
      }
   }
   map->max_key = max_key;
   map->min_key = MIN(min_key, max_key);
   PDRangeArrFree(tmp_arr);

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countArrayToList++;
#endif
//DF-STOP
}


/*-----------------------------------------------------------------------
//
// Function: list_to_array()
//
//   Convert a IntMap in list form to an equivalent one in array
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void list_to_array(IntMap_p map)
{
   PDRangeArr_p  tmp_arr;
   long          max_key = map->min_key;
   long          min_key = map->max_key;
   node_p        handle;

   assert(map->type == IMList);
   map->entry_no = 0;
   tmp_arr = PDRangeArrAlloc(map->min_key, IM_ARRAY_SIZE);
   handle = SkipListMinNode(map->values.list);

   while(handle && handle->next[1] != map->values.list->header)
   {
      if(handle->value.p_val)
      {
         PDRangeArrAssignP(tmp_arr, handle->key, handle->value.p_val);
         map->entry_no++;
         max_key = handle->key;
         min_key = MIN(min_key, handle->key);
         handle = handle->next[1];
      }
   }

   //freeNode(handle);
   SkipListFree(map->values.list);
   map->max_key = max_key;
   map->min_key = MIN(min_key, max_key);
   map->values.array = tmp_arr;
   map->type = IMArray;

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countListToArray++;
#endif
//DF-STOP
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

//DF-START
#ifdef MEASURE_INTMAP_STATS
PTree_p intmaps = NULL;
#endif
//DF-STOP

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

//DF-START
#ifdef MEASURE_INTMAP_STATS
   handle->countGetVal = 0;
   handle->countGetRef = 0;
   handle->countAssign = 0;
   handle->countDelKey = 0;
   handle->countArrayToList = 0;
   handle->countListToArray = 0;

   PTreeStore(&intmaps, handle);
#endif
//DF-STOP

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
//DF-START
#ifdef MEASURE_DELETE
         countDeleteEmpty++;
         countDelete++;
#endif
//DF-STOP
   case IMSingle:
         break;
//DF-START
#ifdef MEASURE_DELETE
         countDeleteInt++;
         countDelete++;
#endif
//DF-STOP
   case IMArray:
         PDRangeArrFree(map->values.array);
//DF-START
#ifdef MEASURE_DELETE
         countDeleteArray++;
         countDelete++;
#endif
//DF-STOP
         break;
   case IMList:
         SkipListFree(map->values.list);
//DF-START
#ifdef MEASURE_DELETE
         countDeleteList++;
         countDelete++;
#endif
//DF-STOP

         break;
   default:
         assert(false && "Unknown IntMap type.");
   }

//DF-START
#ifdef MEASURE_DELETE
   PTreeDeleteEntry(&intmaps, map);
#endif
//DF-STOP

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
   case IMArray:
         if(key <= map->max_key)
         {
            res = PDRangeArrElementP(map->values.array, key);
         }
         break;
   case IMList:
         if(key <= map->max_key)
         {
            node_p entry = SkipListFind(map->values.list, key);
            if(entry)
            {
               res = entry->value.p_val;
            }
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
   }

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countGetVal++;
#endif
//DF-STOP

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
//////////////////////

void** IntMapGetRef(IntMap_p map, long key)
{
   void      **res = NULL;
   void      *val;
   node_p handle;
   IntOrP tmp;

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
         res = map->values.value;
         map->entry_no = 1;

         //DF-START
#ifdef MEASURE_EMPTY
         countEmpty++;
#endif
         //DF-STOP
         break;
   case IMSingle:
         if(key == map->max_key)
         {
            res = map->values.value;

            //DF-START
#ifdef MEASURE_INT
            countInt++;
#endif
            //DF-STOP

         }
         else if(switch_to_array(key, map->min_key, map->max_key, 2))
         {
            map->type = IMArray;
            val = map->values.value;
            map->values.array = PDRangeArrAlloc(MIN(key, map->max_key),
                                                IM_ARRAY_SIZE);
            PDRangeArrAssignP(map->values.array, map->max_key, val);
            PDRangeArrAssignP(map->values.array, key, NULL);
            res = PDRangeArrElementP(map->values.array, key);
            map->entry_no = 2;

            //DF-START
#ifdef MEASURE_ARRAY
            countArray++;
#endif
            //DF-STOP

         }
         else
         {
            map->type = IMList;
            val = map->values.value;
            map->values.list = initSkiplist();;
            tmp.p_val = val;
            SkipListStore(map->values.list, map->max_key, tmp);
            handle = add_new_list_node(map, key, NULL);
            res = handle->value.p_val;
            map->entry_no = 2;

            //DF-START
#ifdef MEASURE_LIST
            countList++;
#endif
            //DF-STOP
         }
         map->min_key = MIN(map->min_key, key);
         map->max_key = MAX(key, map->max_key);
         break;
   case IMArray:

         if(((key > map->max_key)||(key<map->min_key)) &&
            switch_to_list(map->min_key, map->max_key, key, map->entry_no+1))
         {
            array_to_list(map);
            res = IntMapGetRef(map, key);

            //DF-START
#ifdef MEASURE_LIST
            countList++;
#endif
            //DF-STOP
         }
         else
         {
            res = PDRangeArrElementP(map->values.array, key);
            if(!(*res))
            {
               map->entry_no++;

               //DF-START
#ifdef MEASURE_ARRAY
               countArray++;
#endif
               //DF-STOP

            }
         }
         map->min_key=MIN(map->min_key, key);
         map->max_key=MAX(map->max_key, key);
         break;
   case IMList:
         handle = SkipListFind(map->values.list, key);
         if(handle)
         {
            res = handle->value.p_val;
         }
         else
         {
            if(switch_to_array(map->min_key, map->max_key, key, map->entry_no+1))
            {
               list_to_array(map);
               res = IntMapGetRef(map, key);

               //DF-START
#ifdef MEASURE_ARRAY
               countArray++;
#endif
               //DF-STOP

            }
            else
            {
               handle = add_new_list_node(map, key, NULL);
               map->max_key=MAX(map->max_key, key);
               map->min_key=MIN(map->min_key, key);
               res = handle->value.p_val;

               //DF-START
#ifdef MEASURE_LIST
               countList++;
#endif
               //DF-STOP
            }
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
   }

   assert(res);

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countGetRef++;
#endif
//DF-STOP

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


//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countAssign++;
#endif
//DF-STOP

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
   node_p   handle;

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
            if(switch_to_list(map->min_key, map->max_key, map->max_key, map->entry_no))
            {
               array_to_list(map);
            }
         }
         break;
   case IMList:
         handle = SkipListExtractEntry(map->values.list, key);
         if(handle)
         {
            map->entry_no--;
            res = handle->value.p_val;
            if(handle->key == map->max_key)
            {
               if(map->values.list)
               {
                  map->max_key = SkipListMaxKey(map->values.list);
               }
               else
               {
                  map->max_key = map->min_key;
               }
               if(switch_to_array(map->min_key, map->max_key, map->max_key, map->entry_no))
               {
                  list_to_array(map);
               }
            }
            SkipListNodeFree(handle);
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
         break;
   }

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countDelKey++;
#endif
//DF-STOP

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
      case IMList:
            printf("B\n");
            handle->admin_data.list_iter = SkipListMinNode(map->values.list); //header? header->next[1]
            //NumTreeLimitedTraverseInit(map->values.tree, lower_key);
            /**/
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
      case IMArray:
            //break;
      case IMList:
            //SkipListNodeFree(junk->admin_data.list_iter);             //PStackFree(junk->admin_data.tree_iter);
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


