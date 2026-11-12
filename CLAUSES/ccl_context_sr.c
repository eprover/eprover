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
//   Perform contextual simplify-reflect with all clauses in set on
//   clause. Return number of literals deleted. With plain, the
//   context clause has to subsume the clause with the literal negated
//   by plain (set) subsumption, otherwise by multiset subsumption. A
//   full proof object keeps the clause before each step in
//   step_archive.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

int ClauseContextualSimplifyReflect(ClauseSet_p set, Clause_p clause,
                                    bool plain, ClauseSet_p step_archive)
{
   Eqn_p handle;
   int res = 0;
   Clause_p subsumer;
   PStack_p lit_stack = ClauseToStack(clause);

   /* The clause changes, so it must not have features */
   assert(!clause->feature_vec && !clause->feature_set);
   clause->weight = ClauseStandardWeight(clause);

   while(!PStackEmpty(lit_stack))
   {
      handle = PStackPopP(lit_stack);

      ClauseFlipLiteralSign(clause, handle);
      ClauseSubsumeOrderSortLits(clause);

      subsumer = plain?
         ClauseSetPlainSubsumesClause(set, clause):
         ClauseSetMultiSetSubsumesClause(set, clause);

      if(subsumer)
      {
         if(ClauseQueryProp(subsumer, CPIsSOS))
         {
            ClauseSetProp(clause, CPIsSOS);
         }
         ClauseDelProp(clause, CPInitial|CPLimitedRW);
         /* The literal was negated above to look for a subsuming
            clause, so it gets its sign back while the clause is
            archived */
         ClauseFlipLiteralSign(clause, handle);
         ClauseArchiveStep(step_archive, clause);
         ClauseFlipLiteralSign(clause, handle);
         ClauseRemoveLiteral(clause, handle);
         assert(clause->weight == ClauseStandardWeight(clause));
         DocClauseModificationDefault(clause,
                                      inf_context_simplify_reflect,
                                      subsumer);
         res++;
         ClausePushDerivation(clause, DCContextSR, subsumer);
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
//   of clauses pushed. plain selects the subsumption test as for
//   ClauseContextualSimplifyReflect().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetFindContextSRClauses(ClauseSet_p set, Clause_p clause,
                                   bool plain, PStack_p res)
{
   Eqn_p handle;
   long old_sp = PStackGetSP(res);
   PStack_p lit_stack = ClauseToStack(clause);
   /* The clauses with a flipped literal have different features, so
      the clause's own ones (valid again afterwards) must not be used
      for them */
   FreqVector_p feature_vec = clause->feature_vec;
   FeatureSet_p feature_set = clause->feature_set;

   assert(clause->weight == ClauseStandardWeight(clause));

   clause->feature_vec = NULL;
   clause->feature_set = NULL;
   while(!PStackEmpty(lit_stack))
   {
      handle = PStackPopP(lit_stack);
      ClauseFlipLiteralSign(clause, handle);
      ClauseSubsumeOrderSortLits(clause);
      if(plain)
      {
         ClauseSetFindPlainSubsumedClauses(set, clause, res);
      }
      else
      {
         ClauseSetFindMultiSetSubsumedClauses(set, clause, res);
      }
      ClauseFlipLiteralSign(clause, handle);
   }
   clause->feature_vec = feature_vec;
   clause->feature_set = feature_set;
   PStackFree(lit_stack);
   return PStackGetSP(res) - old_sp;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
