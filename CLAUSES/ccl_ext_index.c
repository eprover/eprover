/*-----------------------------------------------------------------------

File  : ccl_ext_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Maintaining an index that map symbol a to pairs (C,p) where C
  is a clause and p is a position such that C|_p is a term s
  headed by the symbol a such that s has a partially applied subterm.

  Copyright 2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include "ccl_ext_index.h"
#include <ccl_clausecpos.h>
#include <ccl_clausepos_tree.h>

typedef void (*IdxOperator)(ExtIndex_p, Clause_p, PStack_p);



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
// Function: delete_idx()
//
//   Delete clause from the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void delete_idx(ExtIndex_p idx, Clause_p cl, PStack_p collected_pos)
{
   FunCode fc;
   while(!PStackEmpty(collected_pos))
   {
      UNUSED(PStackPopInt(collected_pos));
      fc = PStackPopInt(collected_pos);

      ClauseTPosTreeDeleteClause(((ClauseTPosTree_p *)IntMapGetRef(idx, fc)), cl);
   }
}


/*-----------------------------------------------------------------------
//
// Function: insert_idx()
//
//   Given a clause and a stack containg pairs symbol, compact position
//   insert them into idx.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void insert_idx(ExtIndex_p idx, Clause_p cl, PStack_p collected_pos)
{
   CompactPos pos;
   FunCode fc;
   while(!PStackEmpty(collected_pos))
   {
      pos = PStackPopInt(collected_pos);
      fc = PStackPopInt(collected_pos);

      ClauseTPosTreeInsertPos(((ClauseTPosTree_p *)IntMapGetRef(idx, fc)), cl, pos);
   }
}


/*-----------------------------------------------------------------------
//
// Function: collect_into_pos_term()
//
//   Fill the stack with pairs (function symbol, position) eligible for
//   ExtSup inferences. Returns true if t has a functional subterm.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void collect_into_pos_term(Term_p t, CompactPos pos, PStack_p stack)
{
   PStackPointer old_top = PStackGetSP(stack);
   bool has_func_subterm = false;
   
   Term_p normalized_pattern = MAYBE_NORMALIZE_APP_VAR(t);
   if(!TermIsLambda(t) && !normalized_pattern)
   {
      CompactPos new_pos = pos + DEFAULT_FWEIGHT*(TermIsPhonyApp(t) ? 0 : 1);
      for(int i=0; i < t->arity; i++)
      {
         Term_p arg = t->args[i];
         collect_into_pos_term(arg, new_pos, stack);
         has_func_subterm = has_func_subterm || 
                           (TYPE_EXT_ELIGIBLE(arg->type) && !TermIsTopLevelAnyVar(arg));
         new_pos += TermStandardWeight(arg);
      }
   }
   if(!TypeIsArrow(t->type) && !TermIsTopLevelAnyVar(t) && !normalized_pattern)
   {
      if(has_func_subterm || PStackGetSP(stack) != old_top)
      {
         PStackPushInt(stack, t->f_code);
         PStackPushInt(stack, pos);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: build_into_pos_stack()
//
//   Insert all positions that are into-targets of ExtSup inference to index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void build_into_pos_stack(Eqn_p lit, CompactPos pos, PStack_p collected_pos)
{
   collect_into_pos_term(lit->lterm, pos, collected_pos);
   collect_into_pos_term(lit->rterm, (pos + TermStandardWeight(lit->lterm)),
                         collected_pos);
}

/*-----------------------------------------------------------------------
//
// Function: handle_into_idx()
//
//    Perform a generic operation on into idx
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void handle_into_idx(ExtIndex_p into_index, Clause_p cl, IdxOperator op)
{
   PStack_p collected_pos = PStackAlloc();
   CollectExtSupIntoPos(cl, collected_pos);
   op(into_index, cl, collected_pos);
   PStackFree(collected_pos);
}


/*-----------------------------------------------------------------------
//
// Function: handle_from_idx()
//
//    Perform a generic operation on from idx
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void handle_from_idx(ExtIndex_p into_index, Clause_p cl, IdxOperator op)
{
   PStack_p collected_pos = PStackAlloc();
   CollectExtSupFromPos(cl, collected_pos);
   op(into_index, cl, collected_pos);
   PStackFree(collected_pos);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermHasExtEligSubterm()
//
//   Check if a term actually has an eligible subterm for ExtSup
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermHasExtEligSubterm(Term_p t)
{
   bool ans = false;
   for(int i=0; !ans && i < t->arity; i++)
   {
      ans = ans || (TYPE_EXT_ELIGIBLE(t->args[i]->type)
                    && !TermIsTopLevelAnyVar(t->args[i]))
                || TermHasExtEligSubterm(t->args[i]);
   }
   return ans;
}

/*-----------------------------------------------------------------------
//
// Function: ExtIndexInsertIntoClause()
//
//   Insert all positions that are into-targets of ExtSup inference to index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExtIndexInsertIntoClause(ExtIndex_p into_index, Clause_p cl, int max_depth)
{
   if(cl->proof_depth <= max_depth)
   {
      handle_into_idx(into_index, cl, insert_idx);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ExtIndexDeleteIntoClause()
//
//   Delete the clause from into index
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExtIndexDeleteIntoClause(ExtIndex_p into_index, Clause_p cl)
{
   handle_into_idx(into_index, cl, delete_idx);
}


/*-----------------------------------------------------------------------
//
// Function: ExtIndexInsertFromClause()
//
//   Insert all positions that are into-targets of ExtSup inference to index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExtIndexInsertFromClause(ExtIndex_p into_index, Clause_p cl, int max_depth)
{
   if(cl->proof_depth <= max_depth)
   {
      handle_from_idx(into_index, cl, insert_idx);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ExtIndexDeleteFromClause()
//
//   Delete the clause from into index
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExtIndexDeleteFromClause(ExtIndex_p into_index, Clause_p cl)
{
   handle_from_idx(into_index, cl, delete_idx);
}


/*-----------------------------------------------------------------------
//
// Function: ExtIndexFree()
//
//   Delete the clause from into index
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExtIndexFree(ExtIndex_p into_index)
{
   IntMapIter_p i = IntMapIterAlloc(into_index, 0, LONG_MAX);
   ClauseTPosTree_p value;
   long dummy;
   // static inline void* IntMapIterNext(IntMapIter_p iter, long *key);
   while((value = IntMapIterNext(i, &dummy)))
   {
      ClauseTPosTreeFree(value);
   }
   IntMapIterFree(i);
   IntMapFree(into_index);
}

/*-----------------------------------------------------------------------
//
// Function: CollectExtSupFromPos()
//
//   Put pairs (f_code, compact_pos) on pos_stack for all eligible from
//   positions
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void CollectExtSupFromPos(Clause_p cl, PStack_p pos_stack)
{
   CompactPos pos = 0;
   for(Eqn_p handle = cl->literals; handle; handle = handle->next)
   {
      if(!TypeIsArrow(handle->lterm) && EqnIsPositive(handle))
      {
         if(!TermIsTopLevelFreeVar(handle->lterm) && !MAYBE_NORMALIZE_APP_VAR(handle->lterm) &&
            TermHasExtEligSubterm(handle->lterm))
         {
            PStackPushInt(pos_stack, handle->lterm->f_code);
            PStackPushInt(pos_stack, pos);
         }
         pos += TermStandardWeight(handle->lterm);
         if(!TermIsTopLevelFreeVar(handle->rterm) && !MAYBE_NORMALIZE_APP_VAR(handle->lterm) &&
            TermHasExtEligSubterm(handle->rterm))
         {
            PStackPushInt(pos_stack, handle->rterm->f_code);
            PStackPushInt(pos_stack, pos);
         }
         pos += TermStandardWeight(handle->rterm);
      }
      else
      {
         pos += EqnStandardWeight(handle);
      }
   }

}

/*-----------------------------------------------------------------------
//
// Function: CollectExtSupIntoPos()
//
//   Put pairs (f_code, compact_pos) on pos_stack for all eligible into
//   positions
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void CollectExtSupIntoPos(Clause_p cl, PStack_p pos_stack)
{
   CompactPos pos = 0;
   for(Eqn_p handle = cl->literals; handle; handle = handle->next)
   {
      build_into_pos_stack(handle, pos, pos_stack);
      pos += EqnStandardWeight(handle);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

