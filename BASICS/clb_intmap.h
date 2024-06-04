/*-----------------------------------------------------------------------

File  : clb_intmap.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Definitions and functions for a data type that maps natural numbers
  (including 0) to void* pointers, supporting assignments, retrieval,
  deletion, and iteration.

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Dec 18 15:51:13 CET 2004
    New

-----------------------------------------------------------------------*/

#ifndef CLB_INTMAP

#define CLB_INTMAP

#include <limits.h>
#include <clb_skiplists.h>
#include <clb_ptrees.h>
#include <clb_pdrangearrays.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   IMEmpty,
   IMSingle,
   IMArray,
   IMList
}IntMapType;

#define MAX_LIST_DENSITY 8
#define MIN_LIST_DENSITY 4
#define IM_ARRAY_SIZE MAX_LIST_DENSITY

/* This is the main thing - a datatype that keeps key/value pairs and
 * allows inserting, updating, deleting, and ordered iteration. I
 * expect additons to be frequent and deletions to be rare. Element
 * access and iteration are the most frequent operations. We want this
 * time- and space efficient for many different key distributions. */
//
typedef struct intmap_cell
{
   IntMapType type;
   unsigned long entry_no; /* How many key/value pairs? May be slightly
                            * larger than the real value, as keys
                            * associated to NULL are indistinguishable
                            * from unassociated keys. */
   long min_key;           /* Smallest key (may be only key). Again, this
                            * may be an overestimate, as we do not
                            * always correct this if a key is deleted
                            * from an array. */
   long max_key;           /* Largest key (may be only key). Again, this
                            * may be an overestimate, as we do not
                            * always correct this if a key is deleted
                            * from an array. */
//DF-START
#ifdef MEASURE_INTMAP_STATS
/* print these variables during deletion with the size of the
 * corresponding datatype
 */
   long countGetVal;       /* Count the times IntMapGetVal() is executed*/
   long countGetRef;       /* Count the times IntMapGetRef() is executed*/
   long countAssign;       /* Count the times IntMapAssign() is executed*/
   long countDelKey;       /* Count the times IntMapDelKey() is executed*/
   long countArrayToList;  /* Count the times array_to_list() is executed*/
   long countListToArray;  /* Count the times list_to_array() is executed*/
#endif
//DF-STOP

   union
   {
      void*        value;   /* For IMSingle */
      PDRangeArr_p array;   /* For IMArray  */
      SkipList_p   list;    /* For IMList   */
   }values;
}IntMapCell, *IntMap_p;

/////////////////////////////////////////

typedef struct intmap_iter_cell
{
   IntMap_p map;
   long     lower_key;
   long     upper_key;
   union
   {
      bool      seen;      /* For IMSingle */
      long      current;   /* For IMArray  */
      node_p    list_iter; /* For IMList */
   }admin_data;
}IntMapIterCell, *IntMapIter_p;


typedef long (*IntMapFreeFunc)(void *junk_node);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


//DF-START
#ifdef MEASURE_EMPTY
extern long countEmpty;
#endif

#ifdef MEASURE_INT
extern long countInt;
#endif

#ifdef MEASURE_ARRAY
extern long countArray;
#endif

#ifdef MEASURE_LIST
extern long countList;
#endif

#ifdef MEASURE_DELETE
extern long countDelete;

extern long countDeleteEmpty;
extern long countDeleteInt;
extern long countDeleteArray;
extern long countDeleteList;
#endif
//DF-STOP


#define IntMapCellAlloc() (IntMapCell*)SizeMalloc(sizeof(IntMapCell))
#define IntMapCellFree(junk) SizeFree(junk, sizeof(IntMapCell))

IntMap_p IntMapAlloc(void);
void     IntMapFree(IntMap_p map);

void*    IntMapGetVal(IntMap_p map, long key);
void**   IntMapGetRef(IntMap_p map, long key);
void     IntMapAssign(IntMap_p map, long key, void* value);
void*    IntMapDelKey(IntMap_p map, long key);

#define IntMapIterCellAlloc() (IntMapIterCell*)SizeMalloc(sizeof(IntMapIterCell))
#define IntMapIterCellFree(junk) SizeFree(junk, sizeof(IntMapIterCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define INTMAPCELL_MEM 20
#else
#define INTMAPCELL_MEM MEMSIZE(IntMapCell)
#endif

#define IntMapDStorage(map) (((map)->type == IMArray)?\
                             PDArrayStorage((map)->values.array):\
                             (((map)->type == IMList)?\
                              ((map)->entry_no*SKIPLISTNODE_MEM):0))

#define IntMapStorage(map) (INTMAPCELL_MEM+IntMapDStorage(map))

IntMapIter_p IntMapIterAlloc(IntMap_p map, long lower_key, long upper_key);
void         IntMapIterFree(IntMapIter_p junk);
static inline void* IntMapIterNext(IntMapIter_p iter, long *key);

void     IntMapDebugPrint(FILE* out, IntMap_p map);


//DF-START
#ifdef MEASURE_INTMAP_STATS
extern PTree_p intmaps;
#endif
//DF-STOP

/*---------------------------------------------------------------------*/
/*                      Inline Functions                               */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: IntMapIterNext()
//
//   Return the next value/key pair in the map (or NULL/ndef) if the
//   iterator is exhausted.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void* IntMapIterNext(IntMapIter_p iter, long *key)
{
   void* res = NULL;
   long  i;
   node_p handle;

   assert(iter);
   assert(key);

   if(!iter->map)
   {
      return NULL;
   }

   //printf("IntMapIterNext()...\n");
   switch(iter->map->type)
   {
   case IMEmpty:
         break;
   case IMSingle:
         //printf("Case IMSingle\n");
         if(!iter->admin_data.seen)
         {
            iter->admin_data.seen = true;
            *key = iter->map->max_key;
            res = iter->map->values.value;
         }
         break;
   case IMArray:
         // printf("Case IMArray %ld\n", iter->admin_data.current);
         for(i=iter->admin_data.current; i<= iter->upper_key; i++)
         {
            res = PDRangeArrElementP(iter->map->values.array, i);
            if(res)
            {
               *key = i;
               break;
            }
         }
         iter->admin_data.current = i+1;
         break;
   case IMList:
         if(handle && handle->next[1] != iter->map->values.list->header)
         {
            if(handle->value.p_val)
            {
               *key = handle->key;
               res = handle->value.p_val;
               handle = handle->next[1];
            }
         }
         iter->admin_data.list_iter = handle;
         break;
   default:
         assert(false && "Unknown IntMap type.");
         break;
   }
   return res;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

