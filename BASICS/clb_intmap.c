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

#ifdef MEASURE_TREE
long countTree = 0;
#endif

#ifdef MEASURE_DELETE
long countDelete = 0;
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

static NumTree_p add_new_tree_node(IntMap_p map, long key, void* val)
{
   NumTree_p handle, check;
   assert(map->type == IMTree);

   handle = NumTreeCellAlloc();
   handle->key = key;
   handle->val1.p_val = val;
   check = NumTreeInsert(&(map->values.tree), handle);
   UNUSED(check); assert(!check);
   map->entry_no++;

   return handle;
}


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
//         printf("\t Array to tree \t :%d\n", i);
         NumTreeStore(&(map->values.tree), i, tmp_val, tmp_val);
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
   map->countArrayToTree++;
#endif
//DF-STOP
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
   PDRangeArr_p  tmp_arr;
   long          max_key = map->min_key;
   long          min_key = map->max_key;
   PStack_p      tree_iterator;
   NumTree_p     handle;

   assert(map->type == IMTree);

   map->entry_no = 0;
   tmp_arr = PDRangeArrAlloc(map->min_key, IM_ARRAY_SIZE);
   tree_iterator = NumTreeTraverseInit(map->values.tree);
   while((handle = NumTreeTraverseNext(tree_iterator)))
   {
      if(handle->val1.p_val)
      {
         PDRangeArrAssignP(tmp_arr, handle->key, handle->val1.p_val);
         map->entry_no++;
         max_key = handle->key;
         min_key = MIN(min_key, handle->key);
      }
   }
   NumTreeTraverseExit(tree_iterator);
   NumTreeFree(map->values.tree);
   map->max_key = max_key;
   map->min_key = MIN(min_key, max_key);
   map->values.array = tmp_arr;
   map->type = IMArray;

//DF-START
#ifdef MEASURE_INTMAP_STATS
   map->countTreeToArray++;
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
   PTreeStore(&intmaps, handle);
   //IMPTreeStore(&intmaps_tree, handle); //Where is your fucking problem???
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
   case IMSingle:
//DF-START
#ifdef MEASURE_INTMAP_STATS
/*
         printf("# IntMap-Type: \t IMSingle(%p)\n", map);
         printf("# Map-Traits (IMSingle %p): \t Entry-No:%d \t Min:%d \t Max:%d\n", map, map->entry_no, map->min_key, map->max_key);
         printf("# Value (IMSingle %p): \tValue:%d\n",  map, map->values.value);
*/
#endif
//DF-STOP
         break;
   case IMArray:
//DF-START
#ifdef MEASURE_INTMAP_STATS

         printf("# IntMap-Type: \t IMArray(%p)\n", map);
         printf("# Map-Traits (IMArray %p): \t Entry-No:%d \t Min:%d \t Max:%d\n", map, map->entry_no, map->min_key, map->max_key);
         printf("# Executed function IntMapGetVal (IMArray %p): \t %d\n", map, map->countGetVal);
         printf("# Executed function IntMapGetRef (IMArray %p): \t %d\n", map, map->countGetRef);
         printf("# Executed function IntMapAssign (IMArray %p): \t %d\n", map, map->countAssign);
         printf("# Executed function IntMapDelKey (IMArray %p): \t %d\n", map, map->countDelKey);
         printf("# Executed function array_to_tree (IMArray %p): \t %d\n", map, map->countArrayToTree);
         printf("# Executed function tree_to_array (IMArray %p): \t %d\n", map, map->countTreeToArray);

         //printf("# Map-Traits (IMArray %p): \t Entry-No:%d \t Min:%d \t Max:%d\n", map, map->entry_no, map->min_key, map->max_key);
        // printf("# Values-Traits (IMArray %p): \t Offset:%d \t Size:%d \t Grow:%d \t Integer:%s \n", map, map->values.array->offset, map->values.array->size, map->values.array->grow, map->values.array->integer? "True":"False" );
         //PDRangeArrLimitKey = offset

         //PDRangeArrMembers(
         //printf("# Anzahl %p \t %d\n", map->values.array->array,  map->values.array->array.i_val );

         PDRangeArr_p  tmp_arr;                 //temporary Array
         IntOrP        tmp_val;                 //Value
         long          i;                       //Iterator
         long          max_key = map->min_key;  //Maxschlüssel
         long          min_key = map->max_key;  //Minschlüssel

         tmp_arr = map->values.array;           //Belegung mit dem Pointer

         // Auch keine Lösung
         for(i=PDRangeArrLowKey(tmp_arr); i<=map->max_key; i++)
         {
            tmp_val.p_val = PDRangeArrElementP(tmp_arr, i);
            if(tmp_val.p_val)
            {
               printf("# Anzahl Pointer:%p \n", tmp_val.p_val);
             } else {
               printf("# Anzahl i:%d Nummer:%d lower_key: %d\n", i, tmp_val.i_val, PDRangeArrLowKey(tmp_arr));
            }
         }



#endif
//DF-STOP
         PDRangeArrFree(map->values.array);
//DF-START
#ifdef MEASURE_DELETE
         countDelete++;
#endif
//DF-STOP
         break;
   case IMTree:
//DF-START
#ifdef MEASURE_INTMAP_STATS

         printf("# IntMap-Type: \t IMTree(%p)\n", map);
         printf("# Executed function IntMapGetVal (IMTree %p): \t %d\n", map, map->countGetVal);
         printf("# Executed function IntMapGetRef (IMTree %p): \t %d\n", map, map->countGetRef);
         printf("# Executed function IntMapAssign (IMTree %p): \t %d\n", map, map->countAssign);
         printf("# Executed function IntMapDelKey (IMTree %p): \t %d\n", map, map->countDelKey);
         printf("# Executed function array_to_tree (IMArray %p): \t %d\n", map, map->countArrayToTree);
         printf("# Executed function tree_to_array (IMArray %p): \t %d\n", map, map->countTreeToArray);

         //printf("# Map-Traits (IMTree %p): \t Entry-No:%d \t Min:%d \t Max:%d\n", map, map->entry_no, map->min_key, map->max_key);
         //printf("# Values-Traits (IMTree %p): \t key:%d\n", map, map->values.tree->key);
         //printf("# Number of items (IMTree %p): \t %d\n", map, NumTreeNodes(map->values.tree));

#endif
//DF-STOP
         NumTreeFree(map->values.tree);
//DF-START
#ifdef MEASURE_DELETE
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
   case IMTree:
         if(key <= map->max_key)
         {
            NumTree_p entry = NumTreeFind(&(map->values.tree), key);
            if(entry)
            {
               res = entry->val1.p_val;
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


void** IntMapGetRef(IntMap_p map, long key)
{
   void      **res = NULL;
   void      *val;
   NumTree_p handle;
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
         res = &(map->values.value);
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
            res = &(map->values.value);

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
            res = &(PDRangeArrElementP(map->values.array, key));
            map->entry_no = 2;

            //DF-START
#ifdef MEASURE_ARRAY
            countArray++;
#endif
            //DF-STOP

         }
         else
         {
            map->type = IMTree;
            val = map->values.value;
            map->values.tree = NULL;
            tmp.p_val = val;
            NumTreeStore(&(map->values.tree),map->max_key, tmp, tmp);
            handle = add_new_tree_node(map, key, NULL);
            res = &(handle->val1.p_val);
            map->entry_no = 2;

            //DF-START
#ifdef MEASURE_TREE
//            countTree++;
#endif
            //DF-STOP
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

            //DF-START
#ifdef MEASURE_TREE
            countTree++;
#endif
            //DF-STOP
         }
         else
         {
            res = &(PDRangeArrElementP(map->values.array, key));
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
   case IMTree:
         handle = NumTreeFind(&(map->values.tree), key);
         if(handle)
         {
            res = &(handle->val1.p_val);
         }
         else
         {
            if(switch_to_array(map->min_key, map->max_key, key, map->entry_no+1))
            {
               tree_to_array(map);
               res = IntMapGetRef(map, key);

               //DF-START
#ifdef MEASURE_ARRAY
               countArray++;
#endif
               //DF-STOP

            }
            else
            {
               handle = add_new_tree_node(map, key, NULL);
               map->max_key=MAX(map->max_key, key);
               map->min_key=MIN(map->min_key, key);
               res = &(handle->val1.p_val);

               //DF-START
#ifdef MEASURE_TREE
               countTree++;
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
   NumTree_p   handle;

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
         handle = NumTreeExtractEntry(&(map->values.tree), key);
         if(handle)
         {
            map->entry_no--;
            res = handle->val1.p_val;
            if(handle->key == map->max_key)
            {
               if(map->values.tree)
               {
                  map->max_key = NumTreeMaxKey(map->values.tree);
               }
               else
               {
                  map->max_key = map->min_key;
               }
               if(switch_to_array(map->min_key, map->max_key, map->max_key, map->entry_no))
               {
                  tree_to_array(map);
               }
            }
            NumTreeCellFree(handle);
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
      case IMTree:
            handle->admin_data.tree_iter =
               NumTreeLimitedTraverseInit(map->values.tree, lower_key);
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
            break;
      case IMTree:
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


