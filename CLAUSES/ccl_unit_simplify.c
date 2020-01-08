/*-----------------------------------------------------------------------

File  : ccl_unit_simplify.c

Author: Stephan Schulz

Contents

  Functions for doing unit cut-off and unit-subsumption with indexed
  mixed clause sets.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 23 02:00:52 CEST 2002
    New

-----------------------------------------------------------------------*/

#include "ccl_unit_simplify.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* UnitSimplifyNames[]=
{
   "NoSimplify",
   "TopSimplify",
   "FullSimplify",
   NULL
};

const SimplifyRes SIMPLIFY_FAILED = {.pos = NULL, .remaining_args = MATCH_FAILED};

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
// Function: FindTopSimplifyingUnit()
//
//   Find a unit s=t (or s!=t) in units such that sigma(s)=t1 and
//   sigma(t)=t2 for some sigma.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

SimplifyRes FindTopSimplifyingUnit(ClauseSet_p units, Term_p t1,
               Term_p t2)
{
   Subst_p     subst = SubstAlloc();
   int remains = MATCH_FAILED;
   ClausePos_p pos;
   SimplifyRes res = SIMPLIFY_FAILED;

   assert(TermStandardWeight(t1) == TermWeight(t1,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(TermStandardWeight(t2) == TermWeight(t2,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(units);
   assert(units->demod_index);

   PDTreeSearchInit(units->demod_index, t1, PDTREE_IGNORE_NF_DATE, false);

   MatchRes_p mi;
   while((mi = PDTreeFindNextDemodulator(units->demod_index, subst)))
   {
      pos = mi->pos;

      if((remains = SubstMatchPossiblyPartial(ClausePosGetOtherSide(pos), t2, subst)) 
            != MATCH_FAILED)
      {
        // if the problem is not HO, we match completely.
        assert(!(problemType == PROBLEM_FO) || remains == 0);
        assert(pos->clause->set == units);
        assert(remains == mi->remaining_args);
        res = (SimplifyRes){.pos = pos, .remaining_args = remains};
        MatchResFree(mi);
        break;
      }
   }
   PDTreeSearchExit(units->demod_index);
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FindSignedTopSimplifyingUnit()
//
//   Find a unit s=t (or s!=t) in units such that sigma(s)=t1 and
//   sigma(t)=t2 for some sigma. Return only clauses with sign sign.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
SimplifyRes FindSignedTopSimplifyingUnit(ClauseSet_p units, Term_p t1,
                Term_p t2, bool sign)
{
   Subst_p     subst = SubstAlloc();
   int remains = MATCH_FAILED;
   ClausePos_p pos;
   SimplifyRes res = SIMPLIFY_FAILED;

   assert(TermStandardWeight(t1) == TermWeight(t1,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(TermStandardWeight(t2) == TermWeight(t2,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(units);
   assert(units->demod_index);

   PDTreeSearchInit(units->demod_index, t1, PDTREE_IGNORE_NF_DATE, false);

   MatchRes_p mi;
   while((mi = PDTreeFindNextDemodulator(units->demod_index, subst)))
   {
      pos = mi->pos;
      if(EQUIV(EqnIsPositive(pos->literal), sign)
          && (remains = 
                SubstMatchPossiblyPartial(ClausePosGetOtherSide(pos), t2, subst)) != MATCH_FAILED)
      {
        // if the problem is not HO, we match completely.
        assert(!(problemType == PROBLEM_FO) || remains == 0);
        assert(pos->clause->set == units);
        assert(remains == mi->remaining_args);
        res = (SimplifyRes){.pos = pos, .remaining_args = remains};
        MatchResFree(mi);
        break;
      }
      MatchResFree(mi);
   }
   PDTreeSearchExit(units->demod_index);
   SubstDelete(subst);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: RemainingArgsSame()
//
//   Determines if the trailing arugments are the same in both of the
//   terms. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
__inline__ bool RemainingArgsSame(Term_p t1, Term_p t2, SimplifyRes *res)
{
   int remains = res->remaining_args;
   assert(problemType != PROBLEM_FO || !remains);
   while(remains)
   {
      if(t1->args[t1->arity - remains] != t2->args[t2->arity - remains])
      {
         return false;
      }
      remains --;
   }
   assert(!SimplifyFailed(*res));
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: FindSimplifyingUnit()
//
//   Return a unit clause with from set that can simplify or subsume
//   t1=t2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

SimplifyRes FindSimplifyingUnit(ClauseSet_p set, Term_p t1, Term_p t2,
            bool positive_only)
{
   Term_p   tmp1, tmp2 = NULL;
   int      i;
   SimplifyRes res = SIMPLIFY_FAILED;

   if(positive_only)
   {
      res = FindSignedTopSimplifyingUnit(set, t1, t2, true);
   }
   else
   {
      res = FindTopSimplifyingUnit(set, t1, t2);
   }
   
   if(!SimplifyFailed(res))
   {
      return RemainingArgsSame(t1, t2, &res) ? res : SIMPLIFY_FAILED;
   }

   while(SimplifyFailed(res))
   {
      if(t1->f_code != t2->f_code || !t1->arity)
      {
        break;
      }
      tmp1 = NULL; /* Used to determine if another position has
            already been found */

      assert(t1!=t2);
      for(i=0; i<t1->arity; i++)
      {
         if(t1->args[i] != t2->args[i])
         {
            if(tmp1)
            {
               tmp2 = NULL; /* Signal that more than one conflict
                     exists */
               break;
            }
            tmp1 = t1->args[i];
            tmp2 = t2->args[i];
         }
      }
      if(!tmp2)
      {
         break;
      }
      t1 = tmp1;
      t2 = tmp2;
      res = FindSignedTopSimplifyingUnit(set, t1, t2, true);
      if(problemType == PROBLEM_HO && !SimplifyFailed(res))
      {
         return RemainingArgsSame(t1, t2, &res) ? res : SIMPLIFY_FAILED;
      }
   }
   return res;
}





/*-----------------------------------------------------------------------
//
// Function: ClauseSimplifyWithUnitSet()
//
//   Simplify a clause with the (indexed) units from set. Performs
//   simplify-reflect and subsumption steps. simplify-reflect is
//   controlled by the value of how. If clause is subsumed by a unit,
//   return false, otherwise return true.
//
// Global Variables: -
//
// Side Effects    : Changes clause, may cause output.
//
/----------------------------------------------------------------------*/

bool ClauseSimplifyWithUnitSet(Clause_p clause, ClauseSet_p unit_set,
                               UnitSimplifyType how)
{
   Eqn_p *handle;
   SimplifyRes res;

   assert(clause);
   assert(unit_set);
   assert(how);

   handle = &(clause->literals);
   while(*handle)
   {
      if(how == TopLevelUnitSimplify)
      {
         res = FindTopSimplifyingUnit(unit_set,
                                       (*handle)->lterm,
                                       (*handle)->rterm);
      }
      else
      {
         res = FindSimplifyingUnit(unit_set,
                    (*handle)->lterm,
                    (*handle)->rterm, false);
      }
      if(!SimplifyFailed(res))
      {
         ClausePos_p pos = res.pos;
         assert(ClauseIsUnit(pos->clause));
         if(EQUIV(EqnIsPositive(*handle),
             EqnIsPositive(pos->literal)))
         {
            DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
                 "subsumed by unprocessed unit",
                 pos->clause);
            if(!ClauseIsUnit(clause)&&
               ClauseStandardWeight(clause)==ClauseStandardWeight(pos->clause))
            {
               ClauseSetProp(pos->clause, CPIsProtected);
            }
            ClauseSetProp(pos->clause, ClauseQueryProp(clause, CPIsSOS));
            return false;
         }
         ClauseDelProp(clause, CPLimitedRW);
         ClauseRemoveLiteralRef(clause, handle);
         DocClauseModification(GlobalOut, OutputLevel, clause,
                     inf_simplify_reflect, pos->clause,
                     NULL, "cut with unprocessed unit");
      }
      else
      {
         handle = &((*handle)->next);
      }
   }
   return true;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/







