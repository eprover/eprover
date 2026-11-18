/*-----------------------------------------------------------------------

File  : cte_termcellstore.c

Author: Stephan Schulz

Contents

  Implementation of term cell stores (except for all intersting parts,
  which are realized as macros...

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Oct  5 01:09:50 MEST 1998
    New
<2> Wed Nov 18 2026
    Growing hash array.

-----------------------------------------------------------------------*/

#include "cte_termcellstore.h"



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
// Function: tcs_bucket_alloc()
//
//   Allocate and return an array of size empty term cell trees.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Term_p* tcs_bucket_alloc(long size)
{
   Term_p *res = SizeMalloc(size*sizeof(Term_p));
   long   i;

   for(i=0; i<size; i++)
   {
      res[i] = NULL;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tcs_grow()
//
//   Double the number of buckets of the store and move all cells
//   into the new array. The old trees are taken apart, and each cell
//   is inserted into its new tree on its own.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes store
//
/----------------------------------------------------------------------*/

static void tcs_grow(TermCellStore_p store)
{
   Term_p   *old_store = store->store;
   long     old_size   = store->size;
   long     i;
   PStack_p stack = PStackAlloc();
   Term_p   cell, existing;

   store->size  = 2*old_size;
   store->mask  = store->size-1;
   store->store = tcs_bucket_alloc(store->size);

   for(i=0; i<old_size; i++)
   {
      PStackPushP(stack, old_store[i]);
      while(!PStackEmpty(stack))
      {
         cell = PStackPopP(stack);
         if(cell)
         {
            PStackPushP(stack, cell->lson);
            PStackPushP(stack, cell->rson);
            existing = TermTreeInsert(
               &(store->store[TermCellHash(cell, store->mask)]), cell);
            UNUSED(existing); assert(!existing);
         }
      }
   }
   PStackFree(stack);
   SizeFree(old_store, old_size*sizeof(Term_p));
}



/*-----------------------------------------------------------------------
//
// Function: collect_unmarked_termcells()
//
//   Push the addresses of all unmarked term cells in the tree onto
//   the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void collect_unmarked_termcells(PStack_p res_stack, Term_p tree,
                   TermProperties gc_state)
{
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, tree);

   while(!PStackEmpty(stack))
   {
      tree = PStackPopP(stack);
      if(tree)
      {
    if(GiveProps(tree,TPGarbageFlag)==gc_state)
    {
       PStackPushP(res_stack, tree);
    }
    PStackPushP(stack, tree->lson);
    PStackPushP(stack, tree->rson);
      }
   }
   PStackFree(stack);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermCellStoreInit()
//
//   Initialize a term cell storage.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCellStoreInit(TermCellStore_p store)
{
   store->entries   = 0;
   store->arg_count = 0;
   store->size      = TERM_STORE_INIT_SIZE;
   store->mask      = TERM_STORE_INIT_SIZE-1;
   store->store     = tcs_bucket_alloc(store->size);
}

/*-----------------------------------------------------------------------
//
// Function: TermCellStoreExit()
//
//   Free the trees in a term cell storage.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TermCellStoreExit(TermCellStore_p store)
{
   long i;

   for(i=0; i<store->size; i++)
   {
      TermTreeFree(store->store[i]);
   }
   SizeFree(store->store, store->size*sizeof(Term_p));
   store->store = NULL;
   store->size  = 0;
   store->mask  = 0;
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreFind()
//
//   Find a term cell in the store.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p  TermCellStoreFind(TermCellStore_p store, Term_p term)
{
   return TermTreeFind(&(store->store[TermCellHash(term, store->mask)]), term);

}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreInsert()
//
//   Insert a term cell into the store.
//
// Global Variables: -
//
// Side Effects    : Changes store.
//
/----------------------------------------------------------------------*/

Term_p  TermCellStoreInsert(TermCellStore_p store, Term_p term)
{
   Term_p ret;

   ret = TermTreeInsert(&(store->store[TermCellHash(term, store->mask)]),
                        term);
   if(!ret)
   {
      store->entries++;
      store->arg_count+=term->arity;
      if(store->entries > TERM_STORE_MAX_LOAD*store->size)
      {
         tcs_grow(store);
      }
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: TermCellStoreExtract()
//
//   Extract a term cell from the store, return it.
//
// Global Variables: -
//
// Side Effects    : Changes store
//
/----------------------------------------------------------------------*/

Term_p  TermCellStoreExtract(TermCellStore_p store, Term_p term)
{
   Term_p ret;
   ret = TermTreeExtract(&(store->store[TermCellHash(term, store->mask)]), term);
   if(ret)
   {
      store->entries--;
      store->arg_count-=term->arity;
   }
   assert(store->entries>=0);
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreDelete()
//
//   Delete a node from the store.
//
// Global Variables: -
//
// Side Effects    : Changes store
//
/----------------------------------------------------------------------*/

bool TermCellStoreDelete(TermCellStore_p store, Term_p term)
{
   bool ret;
   int  arity = term->arity; /* term may be freed below */

   ret = TermTreeDelete(&(store->store[TermCellHash(term, store->mask)]), term);

   if(ret)
   {
      store->entries--;
      store->arg_count-=arity;
   }
   assert(store->entries>=0);
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreSetProp()
//
//   Set the given properties in all term cells in store.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCellStoreSetProp(TermCellStore_p store, TermProperties props)
{
   long i;

   for(i=0; i<store->size; i++)
   {
      TermTreeSetProp(store->store[i], props);
   }
}

/*-----------------------------------------------------------------------
//
// Function: TermCellStoreDelProp()
//
//   Delete the given properties in all term cells in store.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCellStoreDelProp(TermCellStore_p store, TermProperties props)
{
   long i;

   for(i=0; i<store->size; i++)
   {
      TermTreeDelProp(store->store[i], props);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreCountNodes()
//
//   Return the number of nodes in the term cell store.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermCellStoreCountNodes(TermCellStore_p store)
{
   long res = 0;
   long i;

   for(i=0; i<store->size; i++)
   {
      res+=TermTreeNodes(store->store[i]);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStoreGCSweep()
//
//   Sweep the term cell store and free unmarked cells. Return number
//   of cells recovered. Note that we separate the collection of
//   unmarked terms from the actual deletion, since walking the trees
//   while they may be reorganized is somewhere between messy and
//   impossible.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes term cell store.
//
/----------------------------------------------------------------------*/

long TermCellStoreGCSweep(TermCellStore_p store, TermProperties gc_state)
{
   long recovered = 0;
   long i;
   PStack_p del_stack = PStackAlloc();
   Term_p cell;

   for(i=0; i<store->size; i++)
   {
      collect_unmarked_termcells(del_stack, store->store[i], gc_state);
      while(!PStackEmpty(del_stack))
      {
    cell = PStackPopP(del_stack);
    TermCellStoreDelete(store, cell);
    recovered++;
      }
   }
   PStackFree(del_stack);
   return recovered;
}


/*-----------------------------------------------------------------------
//
// Function: TermCellStorePrintDistrib()
//
//   For each entry (hash value) in store, print the number of term
//   cells in the corresponding tree.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TermCellStorePrintDistrib(FILE* out, TermCellStore_p store)
{
   long i;

   for(i=0; i<store->size; i++)
   {
      CommentPrint(out, " Hash %4ld: %6ld\n", i, TermTreeNodes(store->store[i]));
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
