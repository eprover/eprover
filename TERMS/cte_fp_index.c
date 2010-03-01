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

#define VAR_INDEX(f_code) ((f_code == BELOW_VAR?0:-f_code))


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FPIndexAlloc()
//
//   Allocate an initialized FPIndexCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPIndex_p FPIndexAlloc()
{
   FPIndex_p handle = FPIndexCellAlloc();
   
   handle->f_alternatives = NULL;
   handle->v_alternatives = NULL;
   handle->payload        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFree()
//
//   Free an FPIndex tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexFree(FPIndex_p index, FPIndexFreeFun payload_free)
{
   IntMapIter_p iter;
   long         i;
   FPIndex_p    child;

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
         FPIndexFree(child, payload_free);
      }
      IntMapIterFree(iter);
   }
   if(index->v_alternatives)
   {
      for(i=0; i<=index->max_var; i++)
      {
         child = PDArrayElementP(index->v_alternatives, i);
         if(child)
         {
            FPIndexFree(child, payload_free);
         }
      }   
   }
   FPIndexCellFree(index);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFind()
//
//   Find the leaf node corresponding to key in the index at
//   root. Return NULL if no such node exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FPIndex_p FPIndexFind(FPIndex_p root, IndexFP_p key)
{
   long i, current;
   FPIndex_p res = root;
   
   for(i=1; (i<key[0]) && res; i++)
   {
      current = key[i];
      if(current < 0)
      {
         if(!res->v_alternatives)
         {
            res = NULL;
         }
         else
         {
            res = PDArrayElementP(res->v_alternatives, VAR_INDEX(current));
         }            
      }
      else
      {
         if(!res->f_alternatives)
         {
            res = NULL;
         }
         else
         {
            res = IntMapGetVal(res->f_alternatives, current);
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexInsert() 
//
//   Insert a node corrsponding to key into the index (if necessary)
//   and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes the index
//
/----------------------------------------------------------------------*/

FPIndex_p FPIndexInsert(FPIndex_p root, IndexFP_p key)
{
   long i, current;
   FPIndex_p res = root;
   
   for(i=1; (i<key[0]) && res; i++)
   {
      current = key[i];
      if(current < 0)
      {
         if(!res->v_alternatives)
         {
            res->v_alternatives = PDArrayAlloc(FPINDEX_VAR_INIT_ALT,
                                               FPINDEX_VAR_GROW_ALT);
         }
         root = PDArrayElementP(res->v_alternatives, VAR_INDEX(current));
         if(!root)
         {
            root = FPIndexAlloc();
            PDArrayAssignP(res->v_alternatives, VAR_INDEX(current), root);
            res->max_var = MAX(res->max_var,  VAR_INDEX(current));
         }
         res = root;
      }
      else
      {
         if(!res->f_alternatives)
         {
            res->f_alternatives = IntMapAlloc();
         }
         root = IntMapGetVal(res->f_alternatives, current);
         if(!root)
         {
            root = FPIndexAlloc();
            IntMapAssign(res->f_alternatives, current, root);            
         }
         res = root;
      }
   }
   return res;
}


FPIndex_p FPIndexDelete(FPIndex_p root, IndexFP_p key);

long      FPIndexFindUnifiable(FPIndex_p root, IndexFP_p key, PStack_p res);
long      FPIndexFindMatchable(FPIndex_p root, IndexFP_p key, PStack_p res);


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


