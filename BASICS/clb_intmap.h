/*-----------------------------------------------------------------------

File  : clb_intmap.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Definitions and functions for a data type that maps natural numbers
  (including 0) to void* pointers, supporting assignments, retrieval,
  deletion, and iteration.

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Dec 18 15:51:13 CET 2004
    New

-----------------------------------------------------------------------*/

#ifndef CLB_INTMAP

#define CLB_INTMAP

#include <clb_numtrees.h>
#include <clb_pdarrays.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   IMEmpty,
   IMSingle,
   IMArray,
   IMTree.
}IntMapType;

#define MAX_TREE_DENSITY 10
#define MIN_TREE_DENSITY 4


typedef struct intmap_cell
{
   IntMapType type;
   long entry_no;        /* How many key/value pairs? */
   long max_key;         /* Largest key (may be only key) */
   union 
   {
      void*     value;   /* For IMSingle */
      PDArray_p array;   /* For IMArray  */
      PTree_p   tree;    /* For IMTree   */
   }values;
}IntMapCell, *IntMap_p;


typedef long (*IntMapFreeFunc)(void *junk_node);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define IntMapCellAlloc() (IntMapCell*)SizeMalloc(sizeof(IntMapCell))
#define IntMapCellFree(junk) SizeFree(junk, sizeof(IntMapCell))

IntMap_p IntMapAlloc();
void     IntMapFree(IntMap_p map);
long     IntMapRecFree(IntMap_p map, IntMapFreeFunc free_func);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





