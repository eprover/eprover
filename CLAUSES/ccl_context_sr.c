/*-----------------------------------------------------------------------

File  : ccl_context_sr.c

Author: Stephan Schulz

Contents

  Implementation of contextual (top level) simplify-reflect.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul 14 23:20:54 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "ccl_context_sr.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseContextualSimplifyReflect()
//
//   Perform contextial-simplify-reflect with all clauses in set on
//   clause. Return number of literals deleted.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

int ClauseContextualSimplifyReflect(ClauseSet_p set, Clause_p clause)
{
   Eqn_p handle;
   int res = 0;
   Clause_p subsumer;
   PStack_p lit_stack = ClauseToStack(clause);

   clause->weight = ClauseStandardWeight(clause);

   while(!PStackEmpty(lit_stack))
   {
      handle = PStackPopP(lit_stack);

      ClauseFlipLiteralSign(clause, handle);
      ClauseSubsumeOrderSortLits(clause);

      subsumer = ClauseSetSubsumesClause(set, clause);

      if(subsumer)
      {
         if(ClauseQueryProp(subsumer, CPIsSOS))
         {
            ClauseSetProp(clause, CPIsSOS);
         }
         ClauseDelProp(clause, CPInitial|CPLimitedRW);
         ClauseRemoveLiteral(clause, handle);
         assert(clause->weight == ClauseStandardWeight(clause));
         DocClauseModificationDefault(clause,
                                      inf_context_simplify_reflect,
                                      subsumer);
         res++;
         ClausePushDerivation(clause, DCContextSR, subsumer, NULL);
      }
      else
      {
         ClauseFlipLiteralSign(clause, handle);
      }
   }
   PStackFree(lit_stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindContextSRClauses()
//
//   Find all clauses in set that can be contextually
//   simplify-reflected ;-) with clause and push them onto
//   res. ATTENTION! A clause that can be simplified in more than one
//   way will be pushed more than once onto the stack! Returns number
//   of clauses pushed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetFindContextSRClauses(ClauseSet_p set, Clause_p clause,
               PStack_p res)
{
   Eqn_p handle;
   long old_sp = PStackGetSP(res);
   PStack_p lit_stack = ClauseToStack(clause);

   assert(clause->weight == ClauseStandardWeight(clause));


   while(!PStackEmpty(lit_stack))
   {
      handle = PStackPopP(lit_stack);
      ClauseFlipLiteralSign(clause, handle);
      ClauseSubsumeOrderSortLits(clause);
      ClauseSetFindSubsumedClauses(set, clause, res);
      ClauseFlipLiteralSign(clause, handle);
   }
   PStackFree(lit_stack);
   return PStackGetSP(res) - old_sp;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
