/*-----------------------------------------------------------------------

File  : clb_intmap.h

Authors: Stephan Schulz (schulz@eprover.org), Albert Eisfeld

Contents

  Definitions and functions for a data type that maps natural numbers
  (including 0) to void* pointers, supporting assignments, retrieval,
  deletion, and iteration.

  Copyright 2004, 2026 by the author.
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
#include <clb_numarrtrees.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   IMEmpty,
   IMSingle,
   IMArrTree
}IntMapType;

/* This is the main thing - a datatype that keeps key/value pairs and
 * allows inserting, updating, deleting, and ordered iteration. I
 * expect additons to be frequent and deletions to be rare. Element
 * access and iteration are the most frequent operations. We want this
 * time- and space efficient for many different key distributions. */

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
   union
   {
      void*         value; /* For IMSingle */
      NumArrTree_p   tree;
   }values;
}IntMapCell, *IntMap_p;



typedef struct intmap_iter_cell
{
   IntMap_p map;
   long     lower_key;
   long     upper_key;
   union
   {
      bool      seen;           /* For IMSingle */
      struct
      {
         PStack_p     tree_iter; /* Nodes still to visit */
         NumArrTree_p node;      /* Node being scanned, or NULL */
         long         slot;      /* Next slot to look at in node */
      }arr;                      /* For IMArrTree */
   }admin_data;
}IntMapIterCell, *IntMapIter_p;


typedef long (*IntMapFreeFunc)(void *junk_node);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

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

#define IntMapDStorage(map) ((((map)->type == IMArrTree)?\
                              ((map)->entry_no*NUMARRTREECELL_MEM):0))

#define IntMapStorage(map) (INTMAPCELL_MEM+IntMapDStorage(map))

IntMapIter_p IntMapIterAlloc(IntMap_p map, long lower_key, long upper_key);
void         IntMapIterFree(IntMapIter_p junk);
static inline void* IntMapIterNext(IntMapIter_p iter, long *key);

void     IntMapDebugPrint(FILE* out, IntMap_p map);




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
   NumArrTree_p handle;

   assert(iter);
   assert(key);

   if(!iter->map)
   {
      return NULL;
   }

   switch(iter->map->type)
   {
   case IMEmpty:
         break;
   case IMSingle:
         if(!iter->admin_data.seen)
         {
            iter->admin_data.seen = true;
            *key = iter->map->max_key;
            res = iter->map->values.value;
         }
         break;
   case IMArrTree:
         while(true)
         {
            handle = iter->admin_data.arr.node;
            if(!handle)
            {
               handle = NumArrTreeTraverseNext(iter->admin_data.arr.tree_iter);
               if(!handle)
               {
                  break;
               }
               iter->admin_data.arr.node = handle;
               iter->admin_data.arr.slot = 0;
               if(handle->key < iter->lower_key)
               {
                  /* Skip the part of the node below the range */
                  iter->admin_data.arr.slot = iter->lower_key - handle->key;
               }
            }
            for(long i = iter->admin_data.arr.slot; i < handle->size; i++)
            {
               if(handle->key + i > iter->upper_key)
               {
                  /* Overrun limit, nothing more to find */
                  PStackReset(iter->admin_data.arr.tree_iter);
                  iter->admin_data.arr.node = NULL;
                  return NULL;
               }
               if(handle->vals[i].p_val)
               {
                  iter->admin_data.arr.slot = i+1;
                  *key = handle->key + i;
                  return handle->vals[i].p_val;
               }
            }
            iter->admin_data.arr.node = NULL;
         }
         break;
   default:
         assert(false && "Unknown IntMap type.");
         break;
   }
   //printf("...IntMapIterNext()\n");
   return res;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





