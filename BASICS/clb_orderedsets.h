/*-----------------------------------------------------------------------

  File  : clb_orderedsets.h

  Author: Stephan Schulz

  Contents

  Ordered pointer sets: Sets of pointers that combine a splay tree
  (for fast membership tests, insertion and deletion, as in
  clb_ptrees) with a cyclic doubly linked list through the same cells
  that keeps the elements in order of insertion. Iteration and all
  other order-dependent operations follow the list, so their results
  do not depend on the addresses of the stored objects (and hence not
  on address space layout randomization).

  A set is represented by a pointer to an anchor cell (of the same
  type as the element cells), which holds the root of the tree (in
  lson) and is the anchor of the list (succ is the first, pred the
  last element). The anchor only exists for non-empty sets. The empty
  set is represented by NULL, so sets are used just like PTrees: Pass
  &set to all functions that may change the set.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Oct 29 2026

  -----------------------------------------------------------------------*/

#ifndef CLB_ORDEREDSETS

#define CLB_ORDEREDSETS

#include <stdint.h>
#include <clb_pstacks.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Used for both the element cells and the anchor. In element cells,
   lson/rson are the tree children and pred/succ the neighbours in
   the list. In the anchor, lson is the tree root, succ/pred are the
   first/last element of the list, and key and rson are unused. */

typedef struct ordsetcell
{
   struct ordsetcell *lson;
   struct ordsetcell *rson;
   void              *key;
   struct ordsetcell *pred;
   struct ordsetcell *succ;
}OrdSetCell, *OrdSet_p;

/* Iterator state for OrdSetTraverseInit()/OrdSetTraverseNext() */

typedef struct ordsetitercell
{
   OrdSet_p anchor;
   OrdSet_p next;   /* Next element to return, or anchor at the end */
}OrdSetIterCell, *OrdSetIter_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define OrdSetCellAlloc()    (OrdSetCell*)SizeMalloc(sizeof(OrdSetCell))
#define OrdSetCellFree(junk) SizeFree(junk, sizeof(OrdSetCell))
#define OrdSetIterCellAlloc()    (OrdSetIterCell*)SizeMalloc(sizeof(OrdSetIterCell))
#define OrdSetIterCellFree(junk) SizeFree(junk, sizeof(OrdSetIterCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define ORDSET_CELL_MEM 24
#else
#define ORDSET_CELL_MEM MEMSIZE(OrdSetCell)
#endif

#define OrdSetEmpty(set) ((set)==NULL)
#define OrdSetFirst(set) ((set)?(set)->succ:NULL)
#define OrdSetLast(set)  ((set)?(set)->pred:NULL)
#define OrdSetNext(set, cell) (((cell)->succ==(set))?NULL:(cell)->succ)
#define OrdSetPrev(set, cell) (((cell)->pred==(set))?NULL:(cell)->pred)

void     OrdSetFree(OrdSet_p junk);
bool     OrdSetStore(OrdSet_p *set, void* key);
OrdSet_p OrdSetFind(OrdSet_p *set, void* key);
OrdSet_p OrdSetFindBinary(OrdSet_p set, void* key);
void*    OrdSetExtractKey(OrdSet_p *set, void* key);
void*    OrdSetExtractFirst(OrdSet_p *set);
void*    OrdSetExtractLast(OrdSet_p *set);
bool     OrdSetDeleteEntry(OrdSet_p *set, void* key);
bool     OrdSetMerge(OrdSet_p *set, OrdSet_p add);
void     OrdSetInsertSet(OrdSet_p *set, OrdSet_p add);
long     OrdSetCardinality(OrdSet_p set);
long     PStackToOrdSet(OrdSet_p *set, PStack_p stack);
long     OrdSetToPStack(PStack_p target_stack, OrdSet_p set);
void*    OrdSetSharedElement(OrdSet_p *set1, OrdSet_p set2);
OrdSet_p OrdSetIntersection(OrdSet_p set1, OrdSet_p set2);
long     OrdSetDestrIntersection(OrdSet_p *set1, OrdSet_p set2);
OrdSet_p OrdSetCopy(OrdSet_p set);
bool     OrdSetEquiv(OrdSet_p set1, OrdSet_p set2);
bool     OrdSetIsSubset(OrdSet_p sub, OrdSet_p *super);
void     OrdSetVisit(OrdSet_p set, void (*visitor)(void*, void*), void* arg);
bool     OrdSetCheck(OrdSet_p set);

OrdSetIter_p OrdSetTraverseInit(OrdSet_p set);
OrdSet_p     OrdSetTraverseNext(OrdSetIter_p iter);
#define      OrdSetTraverseExit(iter) OrdSetIterCellFree(iter)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
