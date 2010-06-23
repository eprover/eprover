/*-----------------------------------------------------------------------

File  : cte_fp_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Code implementing fingerprint indexing for terms.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Feb 28 22:49:34 CET 2010
    New

-----------------------------------------------------------------------*/

#include "cte_fp_index.h"



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
// Function: fpindex_alternative()
//
//   Return the child indexed by key f_code in index if it exists,
//   NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p fpindex_alternative(FPTree_p index, FunCode f_code)
{
   assert(index);
   
   if(f_code == BELOW_VAR)
   {
      return index->below_var;
   }            
   else if(f_code == ANY_VAR)
   {
      return index->any_var;      
   }
   else
   {
      if(!index->f_alternatives)
      {
         return NULL;
      }
      else
      {
         return IntMapGetVal(index->f_alternatives, f_code);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: fpindex_alternative_ref()
//
//   Return the address of the child pointer indexed by key f_code in
//   index (create it if its not already there).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p* fpindex_alternative_ref(FPTree_p index, FunCode f_code)
{
   void *res;

   assert(index);
   

   if(f_code == BELOW_VAR)
   {
      return &(index->below_var);
   }            
   else if(f_code == ANY_VAR)
   {
      return &(index->any_var);      
   }
   else
   {
      if(!index->f_alternatives)
      {
         index->f_alternatives = IntMapAlloc();
      }
      res = IntMapGetRef(index->f_alternatives, f_code);   
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fpindex_extract_alt()
//
//   Return the the child pointer indexed by key f_code in
//   index and remove it from the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p fpindex_extract_alt(FPTree_p index, FunCode f_code)
{
   FPTree_p res = NULL;

   assert(index);
   
   if(f_code == BELOW_VAR)
   {
      res = index->below_var;
      index->below_var = NULL;
   }            
   else if(f_code == ANY_VAR)
   {
      res = index->any_var;      
      index->any_var = NULL;
   }
   else if(index->f_alternatives)
   {
      res = IntMapDelKey(index->f_alternatives, f_code);
   }
   if(res)
   {
      index->count--;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_delete()
//
//   Delete the branches leading (only) to the leaf identified to the
//   given key, _if_ that node has no payload.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/

static bool fpindex_rek_delete(FPTree_p index, IndexFP_p key, int current)
{
   bool delete;

   if(!index)
   {
      return false;
   }
   if(current == key[0])
   {
      return index->payload!=NULL;
   }
   delete = fpindex_rek_delete(fpindex_alternative(index, key[current]), 
                               key, 
                               current+1);
   if(delete)
   {
      FPTree_p junk = fpindex_extract_alt(index, key[current]); 
      FPTreeCellFree(junk);
   }   
   return index->count==0;
}


/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_find_unif()
//
//   Find (and push) all payloads from terms unification-compatible
//   with key.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/

static long fp_index_rek_find_unif(FPTree_p index, IndexFP_p key, 
                                   int current, PStack_p collect)
{
   FunCode i;
   long    res = 0;
   IntMapIter_p iter;
   long iter_start;
   FPTree_p child;

   if(!index)
   {
      return 0;
   }
   if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1;
   }
   if(key[current] > 0)
   {
      /* t|p is a function symbol, compatible with:
         - the same symbol or
         - any variable
         - or a below_var position */
      res += fp_index_rek_find_unif(fpindex_alternative(index, key[current]), 
                                    key, 
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(index->any_var, 
                                    key, 
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(index->below_var, 
                                    key, 
                                    current+1,
                                    collect);
   }
   else if(key[current] == NOT_IN_TERM)
   {
      /* Position does not exist in t or any instance -> it cannot
       * unify with an existing position. */
      res += fp_index_rek_find_unif(fpindex_alternative(index, NOT_IN_TERM), 
                                    key, 
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(fpindex_alternative(index, BELOW_VAR), 
                                    key, 
                                    current+1,
                                    collect);
   }
   else if(key[current] == BELOW_VAR || key[current] == ANY_VAR)
   {
      /* t|p is a variable or below a variable -> all but NOT_IN_TERM
         can certainly be unified, NOT_IN_TERM can be unified if
         BELOW_VAR, but not if ANY_VAR (if t|p = X in the query term,
         the position must exist in the search term. But if t|p =
         BELOW_VAR in the query term, the instantiation of X may not
         have the p. */
      
      res += fp_index_rek_find_unif(index->any_var, 
                                    key, 
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(index->below_var, 
                                    key, 
                                    current+1,
                                    collect);      

      iter_start = key[current] == BELOW_VAR? 0:1;               
      iter = IntMapIterAlloc(index->f_alternatives, iter_start, LONG_MAX); 
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);
         res += fp_index_rek_find_unif(child,
                                       key, 
                                       current+1,
                                       collect);
      }
      IntMapIterFree(iter);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_find_matchable()
//
//   Find (and push) all payloads from terms match-compatible
//   with key.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/

static long fp_index_rek_find_matchable(FPTree_p index, IndexFP_p key, 
                                        int current, PStack_p collect)
{
   FunCode i;
   long    res = 0;
   IntMapIter_p iter;
   long iter_start;
   FPTree_p child;

   if(!index)
   {
      return 0;
   }
   if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1; 
   }
   if(key[current] > 0)
   {
      /* t|p is a function symbol, compatible with:
         - the same symbol  */
      res += fp_index_rek_find_matchable(fpindex_alternative(index, key[current]), 
                                         key, 
                                         current+1,
                                         collect);
   }
   else if(key[current] == NOT_IN_TERM)
   {
      /* Position does not exist in t or any instance:
         - it cannot match an existing position
         - It can match below-var, though, as instantiation can
         introduce new excluded positions */
      res += fp_index_rek_find_matchable(fpindex_alternative(index, NOT_IN_TERM), 
                                         key, 
                                         current+1,
                                         collect);
      res += fp_index_rek_find_matchable(fpindex_alternative(index, BELOW_VAR), 
                                         key, 
                                         current+1,
                                         collect);
   }
   else if(key[current] == BELOW_VAR || key[current] == ANY_VAR)
   {
      /* t|p is a variable or below a variable -> all but NOT_IN_TERM
         can certainly be matched, NOT_IN_TERM can be matched if
         BELOW_VAR, but not if ANY_VAR (if t|p = X in the query term,
         the position must exist in the search term. But if t|p =
         BELOW_VAR in the query term, the instantiation of X may not
         have the p. */
      
      res += fp_index_rek_find_matchable(index->any_var, 
                                         key, 
                                         current+1,
                                         collect);
      res += fp_index_rek_find_matchable(index->below_var, 
                                         key, 
                                         current+1,
                                         collect);      

      iter_start = key[current] == BELOW_VAR? 0:1;               
      iter = IntMapIterAlloc(index->f_alternatives, iter_start, LONG_MAX); 
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);
         res += fp_index_rek_find_matchable(child,
                                            key, 
                                            current+1,
                                            collect);
      }
      IntMapIterFree(iter);
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FPTreeAlloc()
//
//   Allocate an initialized FPTreeCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeAlloc()
{
   FPTree_p handle = FPTreeCellAlloc();
   
   handle->f_alternatives = NULL;
   handle->below_var      = NULL;
   handle->any_var        = NULL;
   handle->count          = 0;
   handle->payload        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFree()
//
//   Free an FPTree tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPTreeFree(FPTree_p index, FPTreeFreeFun payload_free)
{
   IntMapIter_p iter;
   long         i;
   FPTree_p    child;

   if(index->payload)
   {
      payload_free(index->payload);
   }
   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, 0, LONG_MAX); 
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);
         FPTreeFree(child, payload_free);
      }
      IntMapIterFree(iter);
      IntMapFree(index->f_alternatives);
   }
   if(index->below_var)
   {
      FPTreeFree(index->below_var, payload_free);
   }
   if(index->any_var)
   {
      FPTreeFree(index->any_var, payload_free);
   }

   FPTreeCellFree(index);
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFind()
//
//   Find the leaf node corresponding to key in the index at
//   root. Return NULL if no such node exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeFind(FPTree_p root, IndexFP_p key)
{
   long i;
   FPTree_p res = root;
   
   for(i=1; (i<key[0]) && res; i++)
   {
      res = fpindex_alternative(res, key[i]);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeInsert() 
//
//   Insert a node corrsponding to key into the index (if necessary)
//   and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes the index
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeInsert(FPTree_p root, IndexFP_p key)
{
   long i, current;
   FPTree_p res = root, *pos;
   
   for(i=1; (i<key[0]) && res; i++)
   {
      current = key[i];

      pos = fpindex_alternative_ref(root, current);
      root = *pos;
      if(!root)
      {
         root = FPTreeAlloc();
         *pos = root;
         res->count++;
      }
      res = root;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeDelete()
//
//   Delete a node corresponding to a key if it does not carry any
//   payload.
//
// Global Variables: -
//
// Side Effects    : Changes index, memory operations
//
/----------------------------------------------------------------------*/

void FPTreeDelete(FPTree_p root, IndexFP_p key)
{
   fpindex_rek_delete(root, key, 1);
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFindUnifiable()
//
//   Push all the payloads of nodes unification-compatible with the
//   given key onto the stack. Return number of payloads pushed. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FPTreeFindUnifiable(FPTree_p root, IndexFP_p key, PStack_p collect)
{
   long count = 0;

   count = fp_index_rek_find_unif(root, key, 1, collect);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: FPTreeFindMatchable()
//
//   Push all the payloads of nodes match-compatible with the
//   given key onto the stack. Return number of payloads pushed. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FPTreeFindMatchable(FPTree_p root, IndexFP_p key, PStack_p collect)
{
   long count = 0;

   count = fp_index_rek_find_matchable(root, key, 1, collect);

   return count;
}



/*-----------------------------------------------------------------------
//
// Function: FPIndexAlloc()
//
//   Alloc an FPIndex.
//
// Global Variables: 
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPIndex_p FPIndexAlloc(FPIndexFunction fp_fun, FPTreeFreeFun payload_free)
{
   FPIndex_p handle = FPIndexCellAlloc();

   handle->fp_fun       = fp_fun;
   handle->payload_free = payload_free;
   handle->index        = FPTreeAlloc();

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexFree()
//
//   Free an FPIndex.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexFree(FPIndex_p index)
{
   FPTreeFree(index->index, index->payload_free);
   FPIndexCellFree(index);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFind()
//
//   Find the index tree node representing term (if any) and return it
//   (or NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p FPIndexFind(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);
   FPTree_p res = FPTreeFind(index->index, key);

   IndexFPFree(key);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexInsert()
//
//   Return a node representing term, creating it if necessary.
//
// Global Variables: -
//
// Side Effects    :  Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p  FPIndexInsert(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);
   FPTree_p res = FPTreeInsert(index->index, key);
   
   IndexFPFree(key);
   return res; 
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexDelete()
//
//   Delete the node representing term, unless it's still in use (per
//   the payload field).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexDelete(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);
   
   FPTreeDelete(index->index, key);
   IndexFPFree(key);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFindUnifiable()
//
//   Return (via collect) all payloads of nodes representing
//   potentially unifiable terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FPIndexFindUnifiable(FPIndex_p index, Term_p term, PStack_p collect)
{
   IndexFP_p key = index->fp_fun(term);
   long res = FPTreeFindUnifiable(index->index, key, collect);
   
   IndexFPFree(key);
   return res; 
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFindMatchable()
//
//   Return (via collect) all payloads of nodes representing
//   potentially matchable terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FPIndexFindMatchable(FPIndex_p index, Term_p term, PStack_p collect)
{
   IndexFP_p key = index->fp_fun(term);
   long res = FPTreeFindMatchable(index->index, key, collect);

   /* IndexFPPrint(stdout, key);
      printf("\n"); */
   IndexFPFree(key);
   
   return res; 
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


