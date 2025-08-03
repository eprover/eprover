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
   int i;

   store->entries = 0;
   store->arg_count = 0;
   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {
      store->store[i] = NULL;
   }
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
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {
      TermTreeFree(store->store[i]);
      store->store[i] = NULL;
   }
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
   return TermTreeFind(&(store->store[TermCellHash(term)]), term);

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

   ret = TermTreeInsert(&(store->store[TermCellHash(term)]), term);
   if(!ret)
   {
      store->entries++;
      store->arg_count+=term->arity;
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
   ret = TermTreeExtract(&(store->store[TermCellHash(term)]), term);
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

   ret = TermTreeDelete(&(store->store[TermCellHash(term)]), term);

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
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
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
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
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
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
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
   int i;
   PStack_p del_stack = PStackAlloc();
   Term_p cell;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
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
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {
      fprintf(out, COMCHAR" Hash %4d: %6ld\n", i, TermTreeNodes(store->store[i]));
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
